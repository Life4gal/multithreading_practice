#include <array>
#include <iostream>
#include <thread>

#include "thread_safe_stack.hpp"

void test_stack();

int main() {
	test_stack();
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

	//	for (auto& thread: threads) {
	//		thread.join();
	//	}
	for (auto i = 0; i < threads.size(); ++i) {
		threads[i].join();
	}
}
