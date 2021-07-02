#include <array>
#include <iostream>
#include <thread>

//#include "lock_free_stack.hpp"
//#include "lock_free_queue.hpp"
#include "misc.hpp"
#include "thread_safe_map.hpp"
#include "thread_safe_queue.hpp"
#include "thread_safe_stack.hpp"

void test_thread_safe_stack();
void test_thread_safe_queue();
void test_thread_safe_map();

//void test_lock_free_stack();
//void test_lock_free_queue();

int	 main() {
	 std::cout.tie(nullptr);

	 std::cout << __cplusplus << std::endl;
	 test_thread_safe_stack();
	 test_thread_safe_queue();
	 test_thread_safe_map();
	 //test_lock_free_stack();
	 //test_lock_free_queue();

	 misc::test_spin_lock();
	 misc::test_parallel_quick_sort();
	 misc::test_parallel_for_each();
	 misc::test_parallel_find();
}

void test_thread_safe_stack() {
	std::cout << "-------------------------------\n"
			  << "Start " << __FUNCTION__
			  << "\n-------------------------------"
			  << std::endl;

	thread_safe::thread_safe_stack<int> stack;

	std::array<std::thread, 10>			threads;

	for (auto i = 0; i < threads.size() / 2; ++i) {
		threads[i] = std::thread{
				[i](thread_safe::thread_safe_stack<int>& stack) {
					for (auto j = 0; j < i + 1; ++j) {
						std::string str{std::string{"Thread: "} + std::to_string(i) + " -> push: " + std::to_string(j)};
						std::cout << str << std::endl;
						stack.push(j);
					}
				},
				std::ref(stack)};
	}

	for (auto i = threads.size() / 2; i < threads.size() / 2 + threads.size() / 4; ++i) {
		threads[i] = std::thread{
				[i](thread_safe::thread_safe_stack<int>& stack) {
					std::string str{std::string{"Thread: "} + std::to_string(i) + " -> pop: "};
					std::cout << str;

					auto ret = stack.pop();
					if (ret) {
						std::cout << ret.operator*() << std::endl;
					} else {
						std::cout << "ret is nullptr" << std::endl;
					}
				},
				std::ref(stack)};
	}

	for (auto i = threads.size() / 2 + threads.size() / 4; i < threads.size(); ++i) {
		threads[i] = std::thread{
				[i](thread_safe::thread_safe_stack<int>& stack) {
					std::string str{std::string{"Thread: "} + std::to_string(i) + " -> pop: "};
					std::cout << str;
					int v;
					try {
						stack.pop(&v);
						std::cout << v << std::endl;
					} catch (thread_safe::thread_safe_stack<int>::empty_stack_exception& e) {
						std::cout << e.what() << std::endl;
					}
				},
				std::ref(stack)};
	}

	for (auto& thread: threads) {
		thread.join();
	}
}

void test_thread_safe_queue() {
	std::cout << "-------------------------------\n"
			  << "Start " << __FUNCTION__
			  << "\n-------------------------------"
			  << std::endl;

	thread_safe::thread_safe_queue<int> queue;

	std::array<std::thread, 10>			threads;

	for (auto i = 0; i < threads.size() / 2; ++i) {
		threads[i] = std::thread{
				[i](thread_safe::thread_safe_queue<int>& queue) {
					for (auto j = 0; j < i + 1; ++j) {
						std::string str{std::string{"Thread: "} + std::to_string(i) + " -> push: " + std::to_string(j)};
						std::cout << str << std::endl;
						queue.push(j);
					}
				},
				std::ref(queue)};
	}

	for (auto i = threads.size() / 2; i < threads.size() / 2 + threads.size() / 4; ++i) {
		threads[i] = std::thread{
				[i](thread_safe::thread_safe_queue<int>& queue) {
					std::string str{std::string{"Thread: "} + std::to_string(i) + " -> wait_and_pop: "};
					std::cout << str;

					auto			 ret = queue.wait_and_pop();
					// 不需要判断
					std::cout << ret.operator*() << " at " << ret << std::endl;
				},
				std::ref(queue)};
	}

	for (auto i = threads.size() / 2 + threads.size() / 4; i < threads.size(); ++i) {
		threads[i] = std::thread{
				[i](thread_safe::thread_safe_queue<int>& queue) {
					std::string str{std::string{"Thread: "} + std::to_string(i) + " -> try_pop: "};

					auto		ret = queue.try_pop();
					// 需要判断
					if (ret) {
						std::cout << ret.operator*() << " at " << ret << std::endl;
					} else {
						std::cout << "nullptr" << std::endl;
					}
				},
				std::ref(queue)};
	}

	for (auto& thread: threads) {
		thread.join();
	}
}

void test_thread_safe_map() {
	std::cout << "-------------------------------\n"
			  << "Start " << __FUNCTION__
			  << "\n-------------------------------"
			  << std::endl;

	thread_safe::thread_safe_map<int, int> map;

	std::array<std::thread, 10>			   threads;

	for (auto i = 0; i < threads.size() / 2; ++i) {
		threads[i] = std::thread{
				[i](thread_safe::thread_safe_map<int, int>& map) {
					for (auto j = 1; j <= i + 1; ++j) {
						std::string str{std::string{"Thread: "} + std::to_string(i) + " -> add_or_update: " + std::to_string(j * 100) + "->" + std::to_string(i)};
						std::cout << str << std::endl;
						map.add_or_update(j * 100, i);
					}
				},
				std::ref(map)};
	}

	for (auto i = threads.size() / 2; i < threads.size(); ++i) {
		threads[i] = std::thread{
				[i](thread_safe::thread_safe_map<int, int>& map) {
					for (auto j = 1; j <= i + 1; ++j) {
						std::string str{std::string{"Thread: "} + std::to_string(i) + " -> get_value of key " + std::to_string(j * 100) + ": "};
						std::cout << str;
						auto value = map.get_value(j * 100, -1);
						if (value == -1) {
							std::cout << "not added or already erased" << std::endl;
						} else {
							std::cout << value << std::endl;
						}
					}
				},
				std::ref(map)};
	}

	for (auto& thread: threads) {
		thread.join();
	}
}

/*
void test_lock_free_stack() {
	std::cout << "-------------------------------\n"
			  << "Start " << __FUNCTION__
			  << "\n-------------------------------"
			  << std::endl;

	lock_free::customize_stack<int> stack;

	std::array<std::thread, 10>		threads;

	for (auto i = 0; i < threads.size() / 2; ++i) {
		threads[i] = std::thread{
				[i](lock_free::customize_stack<int>& stack) {
					for (auto j = 0; j < i + 1; ++j) {
						std::string str{std::string{"Thread: "} + std::to_string(i) + " -> push: " + std::to_string(j)};
						std::cout << str << std::endl;
						stack.push(j);
					}
				},
				std::ref(stack)};
	}

	for (auto i = threads.size() / 2; i < threads.size(); ++i) {
		threads[i] = std::thread{
				[i, size = threads.size()](lock_free::customize_stack<int>& stack) {
					for (auto j = 0; j < size - i; ++j) {
						std::string str{std::string{"Thread: "} + std::to_string(i) + " -> pop: "};
						std::cout << str;

						auto ret = stack.pop();
						if (ret) {
							std::cout << ret.operator*() << std::endl;
						} else {
							std::cout << "ret is nullptr" << std::endl;
						}
					}
				},
				std::ref(stack)};
	}

	for (auto& thread: threads) {
		thread.join();
	}
}
*/

/*
void test_lock_free_queue() {
	std::cout << "-------------------------------\n"
			  << "Start " << __FUNCTION__
			  << "\n-------------------------------"
			  << std::endl;

	lock_free::lock_free_queue<int> queue;

	std::array<std::thread, 10>		threads;

	for (auto i = 0; i < threads.size() / 2; ++i) {
		threads[i] = std::thread{
				[i](lock_free::lock_free_queue<int>& queue) {
					for (auto j = 0; j < i + 1; ++j) {
						std::string str{std::string{"Thread: "} + std::to_string(i) + " -> push: " + std::to_string(j)};
						std::cout << str << std::endl;
						queue.push(j);
					}
				},
				std::ref(queue)};
	}

	for (auto i = threads.size() / 2; i < threads.size(); ++i) {
		threads[i] = std::thread{
				[i, size = threads.size()](lock_free::lock_free_queue<int>& queue) {
					for (auto j = 0; j < size - i; ++j) {
						std::string str{std::string{"Thread: "} + std::to_string(i) + " -> pop: "};
						std::cout << str;

						auto ret = queue.pop();
						// 需要判断
						if (ret) {
							std::cout << ret.operator*() << " at " << ret << std::endl;
						} else {
							std::cout << "nullptr" << std::endl;
						}
					}
				},
				std::ref(queue)};
	}

	for (auto& thread: threads) {
		thread.join();
	}
}
*/
