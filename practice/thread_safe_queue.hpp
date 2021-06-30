#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

/*
 * 使用条件变量实现线程安全
 * 合并front和pop
 */
template<typename T>
class normal_thread_safe_queue {
public:
	using container_type	   = std::queue<T>;
	using value_type		   = typename container_type::value_type;
	using size_type			   = typename container_type::size_type;
	using ptr_type			   = std::shared_ptr<value_type>;

	normal_thread_safe_queue() = default;

	normal_thread_safe_queue(const normal_thread_safe_queue& other) {
		std::lock_guard<std::mutex> lock(other.mutex);

		holding_queue = other.holding_queue;
	}

	void push(value_type value) {
		std::lock_guard<std::mutex> lock(mutex);

		holding_queue.push(std::move(value));
		condition_variable.notify_one();
	}

	ptr_type wait_and_pop() {
		std::unique_lock<std::mutex> lock(mutex);

		condition_variable.wait(lock, [this] { return !this->holding_queue.empty(); });

		auto ret = std::make_shared<value_type>(std::move(holding_queue.front()));
		holding_queue.pop();
		return ret;
	}

	void wait_and_pop(value_type* value) {
		std::unique_lock<std::mutex> lock(mutex);

		condition_variable.wait(lock, [this] { return !this->holding_queue.empty(); });

		if (value) {
			*value = std::move(holding_queue.front());
		}

		holding_queue.pop();
	}

	ptr_type try_pop() {
		std::lock_guard<std::mutex> lock(mutex);

		if (holding_queue.empty()) {
			return nullptr;
		}

		auto ret = std::make_shared<value_type>(std::move(holding_queue.front()));
		holding_queue.pop();
		return ret;
	}

	bool try_pop(value_type* value) {
		std::lock_guard<std::mutex> lock(mutex);

		if (holding_queue.empty()) {
			return false;
		}

		if (value) {
			*value = std::move(holding_queue.front());
		}
		holding_queue.pop();
		return true;
	}

	bool empty() const noexcept {
		std::lock_guard<std::mutex> lock(mutex);

		return holding_queue.empty();
	}

	size_type size() const noexcept {
		std::lock_guard<std::mutex> lock(mutex);

		return holding_queue.size();
	}

private:
	container_type			holding_queue;
	mutable std::mutex		mutex;
	std::condition_variable condition_variable;
};

/*
 * 上面的设计引入了一个异常安全问题，
 * 多个线程处于等待时，notify_one只会唤醒一个线程，
 * 如果这个线程在wait_and_pop中抛出异常(比如构造std::shared_ptr对象时)，
 * 其余线程将永远不会唤醒
 *
 * 如果不希望出现这种情况，应该把notify_one改为notify_all，
 * 这样就会唤醒所有线程，代价是大部分线程发现queue仍为empty时，又会继续休眠
 *
 * 第二种方案是抛出异常时让wait_and_pop调用notify_one，从而唤醒另一个线程
 *
 * 第三种方案是将std::shared_ptr的初始化移到push中，并且内部的std::queue不直接存储值，而是存储管理值的std::shared_ptr
 */
template<typename T>
class normal_exception_and_thread_safe_queue {
public:
	using container_type					 = std::queue<T>;
	using real_container_type				 = std::queue<std::shared_ptr<T>>;
	using value_type						 = typename container_type::value_type;
	using size_type							 = typename container_type::size_type;
	using ptr_type							 = std::shared_ptr<value_type>;

	normal_exception_and_thread_safe_queue() = default;

	normal_exception_and_thread_safe_queue(const normal_exception_and_thread_safe_queue& other) {
		std::lock_guard<std::mutex> lock(other.mutex);

		holding_queue = other.holding_queue;
	}

	void push(value_type value) {
		auto						data = std::make_shared<value_type>(std::move(value));

		// 上面的构造在锁外完成，之前只能在pop中且持有锁时完成
		// 内存分配操作开销很大，这种做法减少了mutex的持有时间，提升了性能
		std::lock_guard<std::mutex> lock(mutex);
		holding_queue.push(data);
		condition_variable.notify_one();
	}

	ptr_type wait_and_pop() {
		std::unique_lock<std::mutex> lock(mutex);

		condition_variable.wait(lock, [this] { return !this->holding_queue.empty(); });

		// 之前为 std::make_shared<value_type>(std::move(holding_queue.front()));
		// 现在构造转移到了push中
		auto ret = holding_queue.front();
		holding_queue.pop();
		return ret;
	}

	void wait_and_pop(value_type* value) {
		std::unique_lock<std::mutex> lock(mutex);

		condition_variable.wait(lock, [this] { return !this->holding_queue.empty(); });

		if (value) {
			*value = std::move(holding_queue.front().operator*());
		}
		holding_queue.pop();
	}

	ptr_type try_pop() {
		std::lock_guard<std::mutex> lock(mutex);

		if (holding_queue.empty()) {
			return nullptr;
		}

		// 之前为 std::make_shared<value_type>(std::move(holding_queue.front()));
		auto ret = holding_queue.front();
		holding_queue.pop();
		return ret;
	}

	bool try_pop(value_type* value) {
		std::lock_guard<std::mutex> lock(mutex);

		if (holding_queue.empty()) {
			return false;
		}

		if (value) {
			*value = std::move(holding_queue.front().operator*());
		}
		holding_queue.pop();
		return true;
	}

	bool empty() const noexcept {
		std::lock_guard<std::mutex> lock(mutex);

		return holding_queue.empty();
	}

	size_type size() const noexcept {
		std::lock_guard<std::mutex> lock(mutex);

		return holding_queue.size();
	}

private:
	real_container_type		holding_queue;
	mutable std::mutex		mutex;
	std::condition_variable condition_variable;
};

/*
 * 使用细粒度，锁和条件变量实现线程安全
 *
 * 和thread-safe stack一样，使用mutex保护了整个数据结构，
 * 但限制了对并发的支持。多线程在各种成员函数中被阻塞，
 * 而只有一个线程能在同一时间做任何事。
 * 不过这种限制主要是因为内部实现使用的是std::queue，
 * 为了支持更高级别的并发就需要提供更细粒度的锁，
 * 为了提供更细粒度的锁就要换一种实现方式
 *
 * 最简单的queue实现方式是包含头尾指针的单链表
 */
template<typename T>
class fine_grained_queue {
public:
	using value_type							  = T;
	using ptr_type								  = std::shared_ptr<value_type>;

	fine_grained_queue()						  = default;

	fine_grained_queue(const fine_grained_queue&) = delete;

	fine_grained_queue& operator=(const fine_grained_queue&) = delete;

	void				push(value_type value) {
		   unique_node p   = std::make_shared<node>(std::move(value));

		   node* const raw = p.get();

		   if (tail) {
			   // 如果尾节点不为空则next设为新节点
			   // @note 2
			   tail->next = std::move(p);
		   } else {
			   // 如果尾节点为空(说明无元素)则头节点设为新节点
			   head = std::move(p);
		   }

		   // tail设为新节点的原始指针
		   tail = raw;
	}

	ptr_type try_pop() {
		if (!head) {
			return nullptr;
		}

		ptr_type	ret		 = std::make_shared<value_type>(std::move(head->value));

		unique_node old_head = std::move(head);
		// @note 1
		head				 = std::move(old_head->next);
		return ret;
	}

private:
	struct node;
	using unique_node = std::unique_ptr<node>;
	struct node {
		value_type	value;
		unique_node next;
	};

	unique_node head;
	node*		tail;
};

/*
 * 上面的设计单线程下没问题，但在多线程下就有明显问题，
 * 即使用两个mutex分别保护头尾指针。push可以同时修改头尾指针，
 * 会对两个mutex上锁，更严重的是push和try_pop都能访问next节点(见 @note 1 & 2)，
 * 仅有一个元素时头尾指针相等，两处的next也是同一对象，
 * 于是try_pop读next与push写next就产生了竞争，锁的也是同一个mutex
 *
 * 这个问题可以通过在queue的最后预设一个dummy node解决
 */
template<typename T>
class fine_grained_better_queue {
public:
	using value_type = T;
	using ptr_type	 = std::shared_ptr<value_type>;

	// 预设头尾节点指向一处(未存储任何值)
	fine_grained_better_queue()
		:// C++11 没有 std::make_unique
		  head(new node{}),
		  tail(head.get()) {}

	fine_grained_better_queue(const fine_grained_better_queue&) = delete;

	fine_grained_better_queue& operator=(const fine_grained_better_queue&) = delete;

	void					   push(value_type value) {
		  // 现在push只访问tail而不访问head
		  // 意味着不再会与try_pop操作同一节点而争夺锁
		  ptr_type	  p = std::make_shared<value_type>(std::move(value));

		  // 新建一个不存储值的新节点(用作新的尾节点)
		  unique_node dummy{new node{}};
		  node* const raw = dummy.get();

		  tail->value	  = p;
		  tail->next	  = std::move(dummy);
		  tail			  = raw;
	  }

	ptr_type try_pop() {
		// 同时访问head和tail只在最初的比较上，锁是短暂的
		// 之前的判断条件为if(!head)，现在为头节点与尾节点指向一处
		if (head.get() == tail) {
			return nullptr;
		}

		ptr_type	ret{head->value};

		unique_node old_head = std::move(head);
		head				 = std::move(old_head->next);
		return ret;
	}

private:
	struct node;
	using unique_node = std::unique_ptr<node>;
	struct node {
		// 之前为 value_type value;
		ptr_type	value;
		unique_node next;
	};

	unique_node head;
	node*		tail;
};

/*
 * 接着加上锁，锁的范围应该尽可能小。
 * 对于push来说很简单，对tail的访问上锁即可。
 * 对于try_pop来说，在弹出head(即将head设为next)之前都应该对head加锁，
 * 在最后返回时则不需要锁，因此可以把加锁的部分提取到一个新函数中。
 * 在比较head与tail时，对tail也存在短暂的访问，因此对tail的访问也需要加锁
 *
 * 由于使用了细粒度锁，push中创建新值和新节点都没上锁，
 * 多线程并发创建新值和新节点就不是问题。同一时间内，只有一个线程能添加新节点，
 * 但这只需要一个指针赋值操作，持有锁的时间很短
 *
 * try_pop中对tail mutex的持有时间也很短，只是用来做一次比较，
 * 因此try_pop和push几乎可以同时调用。
 * try_pop中持有head mutex所做的也只是指针赋值操作，
 * 开销较大的析构操作在锁外进行(智能指针的析构特性)。
 * 这意味着同一时间内，虽然只有一个线程能调用pop_head，
 * 但允许多个线程删除节点并安全返回数据，这就提升了try_pop的并发调用数量
 */
template<typename T>
class thread_safe_queue {
	struct node;

public:
	using value_type  = T;
	using ptr_type	  = std::shared_ptr<value_type>;
	using unique_node = std::unique_ptr<node>;

	// 预设头尾节点指向一处(未存储任何值)
	thread_safe_queue()
		:// C++11 没有 std::make_unique
		  head(new node{}),
		  tail(head.get()) {}

	thread_safe_queue(const thread_safe_queue&) = delete;

	thread_safe_queue& operator=(const thread_safe_queue&) = delete;

	/**
	 * @brief 压入一个新值
	 * @param value 要压入的值
	 */
	auto			   push(value_type value) -> void {
		  // 现在push只访问tail而不访问head
		  // 意味着不再会与try_pop操作同一节点而争夺锁
		  ptr_type	  p = std::make_shared<value_type>(std::move(value));

		  // 新建一个不存储值的新节点(用作新的尾节点)
		  unique_node dummy{new node{}};
		  node* const raw = dummy.get();

		  {
			  std::lock_guard<std::mutex> lock(tail_mutex);

			  tail->value = p;
			  tail->next  = std::move(dummy);
			  tail		  = raw;
		  }
		  condition_variable.notify_one();
	  }

	/**
	 * @brief 取出头节点的值,保证取的到值(没有值会等待直到取到值)
	 * @return 头节点的值
	 */
	auto wait_and_pop() -> ptr_type {
		auto old_head = wait_pop_head();
		return old_head->value;
	}

	/**
	 * @brief 取出头节点的值,保证取的到值(没有值会等待直到取到值)
	 * @param value 用于获取值的传入值
	 * @return 头节点
	 */
	auto wait_and_pop(value_type& value) -> unique_node {
		return wait_pop_head(value);
	}

	/**
	 * @brief 取出头节点的值,不保证取的到值(没有值返回空指针)
	 * @return 头节点的值
	 */
	auto try_pop() -> ptr_type {
		auto old_head = try_pop_head();
		return old_head ? old_head->value : nullptr;
	}

	/**
	 * @brief 取出头节点的值,不保证取的到值(没有值返回空指针)
	 * @param value 用于获取值的传入值,如果取不到值则保证不会改变传入的值
	 * @return 头节点
	 */
	auto try_pop(value_type& value) -> unique_node {
		return try_pop_head(value);
	}

	/**
	 * @brief 队列是否为空
	 * @return 是否为空
	 */
	auto empty() -> bool {
		std::lock_guard<std::mutex> lock(head_mutex);
		return head.get() == get_tail();
	}

private:
	struct node {
		// 之前为 value_type value;
		ptr_type			  value;
		std::unique_ptr<node> next;
	};


	unique_node				head;
	node*					tail;

	std::mutex				head_mutex;
	std::mutex				tail_mutex;
	std::condition_variable condition_variable;

	// support function
	auto					get_tail() -> decltype(tail) {
		   std::lock_guard<std::mutex> lock(tail_mutex);
		   return tail;
	}

	auto pop_head() -> decltype(head) {
		auto old_head = std::move(head);
		head		  = std::move(old_head->next);
		return std::move(old_head);
	}

	auto wait_for_data() -> std::unique_lock<std::mutex> {
		std::unique_lock<std::mutex> lock(head_mutex);
		condition_variable.wait(lock, [&] { return head.get() != get_tail(); });
		return std::move(lock);
	}

	auto wait_pop_head() -> decltype(std::declval<decltype(*this)>().pop_head()) {
		std::unique_lock<std::mutex> lock(wait_for_data());
		return pop_head();
	}

	auto wait_pop_head(value_type& value) -> decltype(std::declval<decltype(*this)>().pop_head()) {
		std::unique_lock<std::mutex> lock(wait_for_data());
		value = std::move(head->value.operator*());
		return pop_head();
	}

	auto try_pop_head() -> decltype(std::declval<decltype(*this)>().pop_head()) {
		std::lock_guard<std::mutex> lock(head_mutex);
		if (head.get() == get_tail()) {
			return nullptr;
		}
		return pop_head();
	}

	auto try_pop_head(value_type& value) -> decltype(std::declval<decltype(*this)>().pop_head()) {
		std::lock_guard<std::mutex> lock(head_mutex);
		if (head.get() == get_tail()) {
			return nullptr;
		}
		value = std::move(head->value.operator*());
		return pop_head();
	}
};

#endif//THREAD_SAFE_QUEUE_HPP
