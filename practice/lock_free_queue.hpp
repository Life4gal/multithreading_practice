#ifndef LOCK_FREE_QUEUE_HPP
#define LOCK_FREE_QUEUE_HPP

#include <atomic>
#include <memory>

namespace lock_free {
	/**
	 * queue与stack不同的是，push和pop访问的是数据结构中不同的部分，
	 * 因此需要确保对其中一方的修改对另一方来说是可见的，
	 * 即允许修改被另一方访问。以lock-based queue为基础，
	 * 使用原子的头尾指针，
	 * 实现如下单生产者单消费者（single-producer single-consumer）
	 * lock-free queue
	 */

	template<typename T>
	class rough_queue {
		struct node;

	public:
		using value_type  = T;
		using ptr_type	  = std::shared_ptr<value_type>;
		using atomic_node = std::atomic<node*>;

		rough_queue() : head(new node{}),
						tail(head.load()) {}

		rough_queue(const rough_queue&) = delete;
		rough_queue& operator=(const rough_queue&) = delete;

		~rough_queue() {
			while (auto* const old_head = head.load()) {
				head.store(old_head->next);
				delete old_head;
			}
		}

		void push(value_type value) {
			ptr_type	new_value = std::make_shared<value_type>(value);
			auto		new_node  = new node{};
			auto* const old_tail  = tail.load();
			old_tail->value.swap(new_value);
			old_tail->next = new_node;
			tail.store(new_node);
		}

		ptr_type pop() {
			auto old_head = pop_head();
			if (!old_head) {
				return nullptr;
			}
			auto ret = old_head->value;
			delete old_head;
			return ret;
		}

	private:
		struct node {
			ptr_type value;
			node*	 next;
		};

		atomic_node head;
		atomic_node tail;

		node*		pop_head() {
			  auto* const old_head = head.load();
			  if (old_head == tail.load()) {
				  return nullptr;
			  }
			  head.store(old_head->next);
			  return old_head;
		}
	};

	/**
	 * 这个实现在只有一个线程push，只有一个线程pop，
	 * 且push先于pop时，可以完美工作。
	 * 但如果两个线程同时push，就会读取同一个尾节点，
	 * 导致只有一个push的结果被保留，这是明显的race condition。
	 * 同理，两个线程同时pop_head也只会弹出一个节点
	 *
	 * 除了确保同时只能有一个线程pop，
	 * 还要确保其他线程在访问head时能安全地解引用。
	 * 这个问题类似于lock-free stack的pop，
	 * 需要确保安全地释放，因此之前lock-free stack
	 * 的方案都可以套用在这里
	 *
	 * 假设pop的问题已经解决了，现在考虑push，
	 * push的问题就是多个线程获取的是同一个tail节点。
	 * 第一种解决方法是添加一个dummy节点，
	 * 这样当前的tail只需要更新next指针，
	 * 如果一个线程将next由nullptr设为新节点就说明添加成功，
	 * 否则重新读取tail并添加。这要求对pop也做一些微小的修改，
	 * 以便丢弃带空数据指针的节点并再次循环。
	 * 这种方法的缺点是，每次pop通常要移除两个节点，
	 * 并且内存分配是原来的两倍
	 *
	 * 第二种方法是使用std::atomic<std::shared_ptr>作为节点数据，
	 * 通过compare_exchange_strong对其设置。
	 * 如果std::shared_ptr为lock-free则一切都解决了，
	 * 如果不是就需要其他方案，比如让pop返回std::unique_ptr（对象的唯一引用），
	 * 并使用原始指针原子类型作为节点数据
	 *
	 *
	 */

	template<typename T>
	class lock_free_queue {
		struct node_count;
		struct node;
		struct count_ptr;

	public:
		using value_type		= T;
		using ptr_type			= std::unique_ptr<value_type>;
		using atomic_value_type = std::atomic<value_type*>;
		using atomic_node_count = std::atomic<node_count>;
		using atomic_node		= std::atomic<count_ptr>;
		using count_type		= long;

		lock_free_queue() {
			count_ptr p{new node{}};
			head.store(p);
			tail.store(p);
		}

		void push(value_type value) {
			auto	  new_data = std::make_unique<value_type>(value);

			count_ptr new_next{new node{}};

			auto	  old_tail = tail.load();
			while (true) {
				increase_extra_count(tail, old_tail);

				if (value_type* old_data = nullptr;
					old_tail.p->value.compare_exchange_strong(old_data, new_data.get())) {

					if (count_ptr old_next{};
						!old_tail.p->next.compare_exchange_strong(old_next, new_next)) {
						delete new_next.p;
						new_next = old_next;
					}
					set_new_tail(old_tail, new_next);
					new_data.release();
					break;
				} else {
					// 其他线程帮助完成的工作
					count_ptr old_next{};
					if (old_tail.p->next.compare_exchange_strong(old_next, new_next)) {
						old_next   = new_next;
						new_next.p = new node{};
					}
					set_new_tail(old_tail, old_next);
				}
			}
		}

		ptr_type pop() {
			auto old_head = head.load(std::memory_order_relaxed);

			while (true) {
				increase_extra_count(head, old_head);

				auto* const p = old_head.p;
				if (p == tail.load().p) {
					return nullptr;
				}

				if (auto next = p->next.load();
					head.compare_exchange_strong(old_head, next)) {
					auto* const ret = p->value.exchange(nullptr);
					free_extra_count(old_head);
					return std::unique_ptr<value_type>(ret);
				}
				p->release_ref();
			}
		}

		~lock_free_queue() {
			while (pop())
				;
			delete tail.load().p;
		}

	private:
		struct node_count {
			unsigned int inner : 30;
			unsigned int extra : 2;
		};
		struct node {
			atomic_value_type value;
			atomic_node_count count;
			atomic_node		  next;

			node() {
				node_count new_count{
						.inner = 0,
						.extra = 2};
				count.store(new_count);
				count_ptr new_ptr{};
				next.store(new_ptr);
			}

			void release_ref() {
				auto	   old_counter = count.load(std::memory_order_relaxed);
				node_count new_counter;
				do {
					new_counter = old_counter;
					--new_counter.inner;
				} while (!count.compare_exchange_strong(old_counter, new_counter, std::memory_order_acquire, std::memory_order_relaxed));

				if (new_counter.inner == 0 && new_counter.extra == 0) {
					delete this;
				}
			}
		};

		struct count_ptr {
			count_type count;
			node*	   p;

			count_ptr()
				: count(0),
				  p(nullptr) {}

			explicit count_ptr(node* p)
				: count(1),
				  p(p) {}
		};

		atomic_node head;
		atomic_node tail;

		static void increase_extra_count(atomic_node& counter, count_ptr& old_counter) {
			count_ptr new_counter;
			do {
				new_counter = old_counter;
				++new_counter.count;
			} while (!counter.compare_exchange_strong(old_counter, new_counter, std::memory_order_acquire, std::memory_order_relaxed));
			old_counter.count = new_counter.count;
		}

		static void free_extra_count(count_ptr& old_node) {
			auto* const p			   = old_node.p;
			auto		increase_count = old_node.count - 2;

			auto		old_counter	   = p->count.load(std::memory_order_relaxed);
			node_count	new_counter;
			do {
				new_counter = old_counter;
				--new_counter.extra;
				new_counter.inner += increase_count;
			} while (!p->count.compare_exchange_strong(old_counter, new_counter, std::memory_order_acquire, std::memory_order_relaxed));

			if (new_counter.inner == 0 && new_counter.extra == 0) {
				delete p;
			}
		}

		void set_new_tail(count_ptr& old_tail, const count_ptr& new_tail) {
			// 完成tail的更新
			auto* const curr = old_tail.p;
			while (!tail.compare_exchange_weak(old_tail, new_tail) && old_tail.p == curr)
				;
			if (old_tail.p == curr) {
				free_extra_count(old_tail);
			} else {
				curr->release_ref();
			}
		}
	};
}// namespace lock_free

#endif//LOCK_FREE_QUEUE_HPP
