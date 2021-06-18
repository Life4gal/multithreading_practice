#include <array>
#include <iostream>
#include <thread>

#include "thread_safe_queue.hpp"
#include "thread_safe_stack.hpp"

void test_stack();
void test_queue();

int main() {
	//	test_stack();
	test_queue();
}

void test_stack() {
	thread_safe_stack<int> stack;

	std::array<std::thread, 10> threads;

	for (auto i = 0; i < threads.size() / 2; ++i) {
		threads[i] = std::thread{
				[i](thread_safe_stack<int>& stack) {
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
				[i](thread_safe_stack<int>& stack) {
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
				[i](thread_safe_stack<int>& stack) {
					std::string str{std::string{"Thread: "} + std::to_string(i) + " -> pop: "};
					std::cout << str;
					int v;
					try {
						stack.pop(&v);
						std::cout << v << std::endl;
					} catch (thread_safe_stack<int>::empty_stack_exception& e) {
						std::cout << e.what() << std::endl;
					}
				},
				std::ref(stack)};
	}

	for (auto& thread: threads) {
		thread.join();
	}
}

void test_queue() {
	thread_safe_queue<int> queue;

	std::array<std::thread, 10> threads;

	for (auto i = 0; i < threads.size() / 2; ++i) {
		threads[i] = std::thread{
				[i](thread_safe_queue<int>& queue) {
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
				[i](thread_safe_queue<int>& queue) {
					std::string str{std::string{"Thread: "} + std::to_string(i) + " -> wait_and_pop: "};
					std::cout << str;

					auto ret = queue.wait_and_pop();
					// 不需要判断
					std::cout << ret.operator*() << " at " << ret << std::endl;
				},
				std::ref(queue)};
	}

	for (auto i = threads.size() / 2 + threads.size() / 4; i < threads.size(); ++i) {
		threads[i] = std::thread{
				[i](thread_safe_queue<int>& queue) {
					std::string str{std::string{"Thread: "} + std::to_string(i) + " -> try_pop: "};

					auto ret = queue.try_pop();
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
