#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <boost/thread/thread.hpp>

namespace work {
	class thread_manager {
	public:
		/**
		 * @brief 创建一个线程执行所给函数
		 * @tparam Func 函数类型
		 * @tparam Args 参数类型
		 * @param func 函数
		 * @param args 所有参数
		 */
		template<typename Func, typename... Args>
		void push_function(Func&& func, Args&&... args) {
			thread_group.template create_thread(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
			//		thread_group.template create_thread(boost::bind(std::forward<Func>(func), std::forward<Args>(args)...));
		}

		~thread_manager();

	private:
		boost::thread_group thread_group;
	};
}// namespace work

#endif//THREAD_MANAGER_HPP
