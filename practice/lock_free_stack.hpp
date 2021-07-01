#ifndef LOCK_FREE_STACK_HPP
#define LOCK_FREE_STACK_HPP

#include <atomic>
#include <memory>

namespace lock_free {

	/**
 * 最简单的stack实现方式是包含头节点指针的链表。
 * push的过程很简单，创建一个新节点，
 * 然后让新节点的next指针指向当前head，最后head设为新节点
 */

	template<typename T>
	class rough_stack {
		struct node;

	public:
		using value_type = T;

		/**
	 * @brief 这里的race condition在于，如果两个线程同时push，
	 * 让各自的新节点的next指针指向当前head，
	 * 这样必然导致head最终设为二者之一的新节点，而另一个被丢弃
	 *
	 * 解决方法是，在最后设置head时先进行判断，
	 * 只有当前head与新节点的next相等，才将head设为新节点，
	 * 如果不等则让next指向当前head并重新判断。
	 * 而这个操作必须是原子的，因此就需要使用compare_exchange_weak，
	 * 不需要使用compare_exchange_strong，
	 * 因为compare_exchange_weak在相等时可能替换失败，
	 * 但替换失败也会返回false，放在循环里带来的效果是一样的，
	 * 而compare_exchange_weak在一些机器架构上
	 * 可以产生比compare_exchange_strong更优化的代码
	 *
	 * @param value push的值
	 */
		void push(const value_type& value) {
			const auto new_node = new node(value);
			new_node->next		= head.load();
			while (!head.compare_exchange_weak(new_node->next, new_node))
				;
		}

		/**
	 * @brief pop的过程也很简单，先用一个指针存储当前head，
	 * 再将head设为head->next，最后返回存储的值并删除指针
	 *
	 * 这里的race condition在于，如果两个线程同时pop，
	 * 一个已经删除了原来的head，另一个线程读取head->next时，
	 * head已经是空悬指针。
	 * 因此，这里绕开删除指针这一步，先考虑前几步的实现
	 * @param value
	 */
		void pop(value_type& value) {
			// 还需要考虑head为空指针的情况
			auto old_head = head.load();
			while (!head.compare_exchange_weak(old_head, old_head->next))
				;
			value = old_head->value;
		}

	private:
		struct node {
			value_type value;
			node*	   next;
		};

		std::atomic<node*> head;
	};

	/**
	 * 这里还有两个问题要考虑，一是链表为空时head为空指针，
	 * 二是异常安全问题。如果直接返回值，弹出元素一定在返回之前，
	 * 如果拷贝返回值时抛出异常就会导致元素丢失（移除成功但拷贝失败），
	 * 因此这里传入一个引用来保存结果。但实际上传引用也不行，
	 * 如果其他线程移除了节点，就无法解引用被删除的节点，
	 * 当前线程就不能安全地拷贝数据
	 *
	 * 如果想安全地返回值，应该返回一个指向数据值的智能指针，
	 * 如果没有返回值则可以通过返回nullptr来表示
	 */

	template<typename T>
	class maybe_safe_stack {
		struct node;

	public:
		using value_type = T;
		using ptr_type	 = std::shared_ptr<value_type>;

		void push(const value_type& value) {
			const auto new_node = new node(value);
			new_node->next		= head.load();
			while (!head.compare_exchange_weak(new_node->next, new_node))
				;
		}

		ptr_type pop() {
			auto old_head = head.load();
			while (old_head && !head.compare_exchange_weak(old_head, old_head->next))
				;
			return old_head ? old_head->value : nullptr;
		}

	private:
		struct node {
			ptr_type value;
			node*	 next;

			explicit node(const value_type& v) : value(std::make_shared<value_type>(v)) {}
		};

		std::atomic<node*> head;
	};

	template<typename T>
	class maybe_non_leak_stack {
		struct node;

	public:
		using value_type = T;
		using ptr_type	 = std::shared_ptr<value_type>;

		void push(const value_type& value) {
			const auto new_node = new node(value);
			new_node->next		= head.load();
			while (!head.compare_exchange_weak(new_node->next, new_node))
				;
		}

		ptr_type pop() {
			// 调用pop的线程数加一，表示old_head正被持有，保证可以被解引用
			++pop_count;
			auto old_head = head.load();
			while (old_head && !head.compare_exchange_weak(old_head, old_head->next))
				;
			ptr_type result;
			if (old_head) {
				// old_head一定可以解引用，old_head->val设为nullptr
				result.swap(old_head->value);
			}
			// 如果计数器为1则释放oldHead，否则添加到待删除列表中
			try_reclaim(old_head);
			// res保存了old_head->val
			return result;
		}

	private:
		struct node {
			ptr_type value;
			node*	 next;

			explicit node(const value_type& v) : value(std::make_shared<value_type>(v)) {}
		};

		std::atomic<node*>	head;
		std::atomic<size_t> pop_count;
		std::atomic<node*>	dying_node;

		/**
		 * 但如果要释放所有节点，必须有一个时刻计数器的值为0。
		 * o在高负载的情况下，往往不会存在这样的时刻，从而导致待删除节点的列表无限增长
		 */
		static void			destroy_dying_node(node* head) {
			while (head) {
				auto tmp = head->next;
				delete head;
				head = tmp;
			}
		}

		void add_dying_node(node* first, node* last) {
			// 链接到已有的待删除列表之前
			last->next = dying_node;
			// 确保最后last->next为dying_node，再将dying_node设为first，first即新的头节点
			while (!dying_node.compare_exchange_weak(last->next, first))
				;
		}

		void add_dying_node(node* curr) {
			auto last = curr;
			while (const auto tmp = last->next) {
				// last指向尾部
				last = tmp;
			}
			add_dying_node(curr, last);
		}

		void try_reclaim(node* curr) {
			// 调用pop的线程数为1则可以进行释放
			if (pop_count == 1) {
				// exchange返回dying_node值，即待删除列表的头节点，再将dying_node设为nullptr
				auto n = dying_node.exchange(nullptr);
				--pop_count;
				if (pop_count == 0) {
					// 没有其他线程，则释放待删除列表中所有节点
					destroy_dying_node(n);
				} else if (n) {
					// 如果多于一个线程则继续保存到待删除列表
					add_dying_node(n);
				}
				// 删除传入的节点
				delete curr;
			} else {
				// 调用pop的线程数超过1，添加当前节点到待删除列表
				add_dying_node(curr, curr);
				--pop_count;
			}
		}
	};

	template<typename T>
	requires std::atomic<std::shared_ptr<T>>::is_always_lock_free class maybe_cannot_use_stack {
		struct node;

	public:
		using value_type		 = T;
		using ptr_type			 = std::shared_ptr<value_type>;
		using shared_node		 = std::shared_ptr<node>;
		using atomic_shared_node = std::atomic<shared_node>;

		void push(const value_type& value) {
			const auto new_node = std::make_shared<node>(value);
			new_node->next		= head.load();
			while (!head.compare_exchange_weak(new_node->next, new_node))
				;
		}

		ptr_type pop() {
			auto old_head = head.load();
			while (old_head && !head.atomic_compare_exchange_weak(old_head, old_head->next.load()))
				;
			if (old_head) {
				old_head->next = nullptr;
				return old_head->value;
			}
			return nullptr;
		}

	private:
		struct node {
			ptr_type		   value;
			atomic_shared_node next;

			explicit node(const value_type& v) : value(std::make_shared<value_type>(v)) {}
		};
		atomic_shared_node head;
	};

	// 如果没有libatomic则会出现[undefined reference to `__atomic_xxx_16']
	template<typename T>
	class customize_stack {
		struct node;
		struct count_ptr;

	public:
		using value_type  = T;
		using ptr_type	  = std::shared_ptr<value_type>;
		using atomic_node = std::atomic<count_ptr>;
		using count_type  = long;

		~customize_stack() {
			while (pop())
				;
		}

		void push(const value_type& value) {
			count_ptr new_node{value};
			// 下面比较中release保证之前的语句都先执行，因此load可以使用relaxed
			new_node.p->next = head.load(std::memory_order_relaxed);
			// 比较失败不改变当前值，并可以继续循环，因此可以选择relaxed
			while (!head.compare_exchange_weak(new_node.p->next, new_node, std::memory_order_release, std::memory_order_relaxed))
				;
		}

		ptr_type pop() {
			auto old_head = head.load(std::memory_order_relaxed);
			while (true) {
				// 外部计数递增表示该节点正被使用
				increase_head_count(old_head);
				// 因此可以安全地访问
				auto* const p = old_head.p;
				if (!p) {
					return nullptr;
				}

				if (head.compare_exchange_strong(old_head, p->next, std::memory_order_relaxed)) {
					ptr_type result;
					result.swap(p->value);
					// 再将外部计数减2加到内部计数，减2是因为，
					// 节点被删除减1，该线程无法再次访问此节点再减1
					const auto increase_count = old_head.count - 2;
					// swap要先于delete，因此使用release
					if (p->count.fetch_add(increase_count, std::memory_order_release) == -increase_count) {
						// 如果内部计数加上increase_count为0（相加前为-increase_count）
						delete p;
					}
					return result;
				} else if (p->count.fetch_sub(1, std::memory_order_relaxed) == 1) {
					// 只是用acquire来同步
					p->count.load(std::memory_order_acquire);
					// acquire保证delete在之后执行
					delete p;
				}
			}
		}

	private:
		struct node {
			ptr_type				value;
			// 内部计数
			std::atomic<count_type> count;
			count_ptr				next;

			explicit node(const value_type& v)
				: value(std::make_shared<value_type>(v)),
				  count(0) {}
		};

		struct count_ptr {
			// 外部计数
			count_type count;
			node*	   p;

			explicit count_ptr(const value_type& v)
				: count(1),
				  p(new node(v)) {}

			count_ptr()
				: count(0),
				  p(nullptr) {}
		};

		atomic_node head;

		void		increase_head_count(count_ptr& old_count) {
			   count_ptr new_count;
			   do {
				   new_count = old_count;
				   // 访问head时递增外部计数，表示该节点正被使用
				   ++new_count.count;
			   } while (!head.compare_exchange_strong(old_count, new_count, std::memory_order_acquire, std::memory_order_relaxed));
			   old_count.count = new_count.count;
		}
	};

}// namespace lock_free

#endif//LOCK_FREE_STACK_HPP
