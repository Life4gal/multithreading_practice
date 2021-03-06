#ifndef THREAD_SAFE_STACK_HPP
#define THREAD_SAFE_STACK_HPP

#include <memory>
#include <mutex>
#include <stack>

namespace thread_safe {

	/*
 * 使用锁保证线程安全
 * pop直接返回弹出值来避免pop和top的潜在竞争
 */
	template<typename T>
	class thread_safe_stack {
	public:
		using container_type = std::stack<T>;
		using value_type	 = typename container_type::value_type;
		using size_type		 = typename container_type::size_type;
		using ptr_type		 = std::shared_ptr<value_type>;

		struct empty_stack_exception : std::exception {
			const char* what() const noexcept override {
				return "stack is empty!";
			}
		};

		thread_safe_stack() = default;

		thread_safe_stack(const thread_safe_stack& other) {
			std::lock_guard<std::mutex> lock(other.mutex);

			holding_stack = other.holding_stack;
		}

		thread_safe_stack& operator=(const thread_safe_stack&) = delete;

		void			   push(value_type value) {
			  std::lock_guard<std::mutex> lock(mutex);

			  holding_stack.push(std::move(value));
		}

		ptr_type pop() {
			std::lock_guard<std::mutex> lock(mutex);

			if (holding_stack.empty()) {
				return nullptr;
			}

			const auto ret = std::make_shared<value_type>(std::move(holding_stack.top()));
			holding_stack.pop();
			return ret;
		}

		void pop(value_type* value) {
			std::lock_guard<std::mutex> lock(mutex);

			if (holding_stack.empty()) {
				throw empty_stack_exception{};
			}

			if (value) {
				*value = std::move(holding_stack.top());
			}

			holding_stack.pop();
		}

		bool empty() const noexcept {
			std::lock_guard<std::mutex> lock(mutex);

			return holding_stack.empty();
		}

		size_type size() const noexcept {
			std::lock_guard<std::mutex> lock(mutex);

			return holding_stack.size();
		}

	private:
		container_type	   holding_stack;
		mutable std::mutex mutex;
	};

}// namespace thread_safe

#endif//THREAD_SAFE_STACK_HPP
