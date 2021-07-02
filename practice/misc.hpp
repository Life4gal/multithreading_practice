#ifndef MISC_HPP
#define MISC_HPP

#include <array>
#include <atomic>
#include <functional>
#include <future>
#include <list>
#include <thread>

namespace misc {
	namespace detail {
		template<typename T, typename Pred = std::less<T>>
		std::list<T> parallel_quick_sort(std::list<T> list);

#if __has_include(<jthread>)
		using jthread = std::jthread;
#else
		struct thread_guard {
			std::thread thread;
			~thread_guard() {
				if (thread.joinable()) {
					thread.join();
				}
			}
		};
		using jthread = thread_guard;
#endif

		struct find_element {
			template<typename Iterator, typename T>
			void operator()(Iterator begin, Iterator end, T value, std::promise<Iterator>* ret, std::atomic<bool>* done_flag) {
				try {
					for (; begin != end && !done_flag->load(); ++begin) {
						if (*begin == value) {
							ret->set_value(begin);
							done_flag->store(true);
							return;
						}
					}
				} catch (...) {
					try {
						ret->set_exception(std::current_exception());
						done_flag->store(true);
					} catch (...) {}
				}
			}
		};

		template<typename Iterator, typename Func>
		void parallel_for_each(Iterator begin, Iterator end, Func func);

		template<typename Iterator, typename T>
		Iterator parallel_find(Iterator begin, Iterator end, const T& value);
	}// namespace detail

	void test_spin_lock() {
		struct spin_lock {
			std::atomic_flag flag = ATOMIC_FLAG_INIT;

			void			 lock() {
				while (flag.test_and_set(std::memory_order_acquire))
					;
			}

			void unlock() {
				flag.clear(std::memory_order_release);
			}
		};

		struct lock_manager {
			spin_lock& lock;
			explicit lock_manager(spin_lock& lock) : lock(lock) {
				lock.lock();
			}
			~lock_manager() {
				lock.unlock();
			}
		};

		spin_lock lock;
		auto	  a_func = [&lock](int n) -> void {
			 lock_manager manager{lock};
			 std::cout << "Thread: " << std::this_thread::get_id() << " -> output: " << n << std::endl;
		};

		std::array<std::thread, 10> threads;
		for (auto i = 0; i < threads.size(); ++i) {
			threads[i] = std::thread{
					a_func,
					threads.size() - i};
		}

		for (auto& t: threads) {
			t.join();
		}
	}

	void test_parallel_quick_sort() {
		std::list<int> list;
		list.push_back(1);
		list.push_back(10);
		list.push_back(2);
		list.push_back(20);
		list.push_back(3);
		list.push_back(30);
		list.push_back(4);
		list.push_back(40);
		list.push_back(5);
		list.push_back(50);

		auto ret = detail::parallel_quick_sort(list);

		std::cout << std::dec << "before: ";
		for (auto i: list) {
			std::cout << i << '\t';
		}

		std::cout << "\nafter: ";
		for (auto i: ret) {
			std::cout << i << '\t';
		}

		std::cout << std::endl;
	}

	void test_parallel_for_each() {
		std::vector<size_t> vec(100);
		for (size_t i = 0; i < vec.size(); ++i) {
			vec[i] = (i + i) * i;
		}

		std::cout << std::dec << "before: ";
		for (auto i: vec) {
			std::cout << i << '\t';
		}

		detail::parallel_for_each(vec.begin(), vec.end(), [](size_t& v) {
			v *= 2;
		});

		std::cout << "\nafter: ";
		for (auto i: vec) {
			std::cout << i << '\t';
		}

		std::cout << std::endl;
	}

	void test_parallel_find() {
		std::vector<size_t> vec(100);
		for (size_t i = 0; i < vec.size(); ++i) {
			vec[i] = (i + i) * i;
		}

		const auto value = size_t((75 + 75) * 75);
		auto	   it	 = detail::parallel_find(vec.begin(), vec.end(), value);
		if (it != vec.end()) {
			std::cout << "found value " << value << " at position " << std::distance(vec.begin(), it) << std::endl;
		} else {
			std::cout << "cannot found value " << value << std::endl;
		}
	}

	namespace detail {
		template<typename T, typename Pred>
		std::list<T> parallel_quick_sort(std::list<T> list) {
			if (list.empty()) {
				return list;
			}

			std::list<T> ret;
			ret.splice(ret.begin(), list, list.begin());

			auto front = list.front();
			auto it	   = std::partition(
					   list.begin(),
					   list.end(),
					   [&f = std::as_const(front), pred = Pred{}](const auto& curr) {
						   return pred(curr, f);
					   });

			std::list<T> low;
			low.splice(low.begin(), list, list.begin(), it);
			std::future<std::list<T>> low_ret = std::async(&parallel_quick_sort<T>, std::move(low));

			std::list<T>			  remain  = parallel_quick_sort<T>(std::move(list));

			ret.splice(ret.end(), remain);
			ret.splice(ret.begin(), low_ret.get());
			return ret;
		}

		template<typename Iterator, typename Func>
		void parallel_for_each(Iterator begin, Iterator end, Func func) {
			auto distance = std::distance(begin, end);
			if (distance == 0) {
				return;
			}

			constexpr size_t			   min_require_per_thread = 25;
			const size_t				   max_threads			  = (distance + min_require_per_thread - 1) / min_require_per_thread;
			const size_t				   hardware_threads		  = std::thread::hardware_concurrency();

			const size_t				   min_used_thread		  = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
			const size_t				   block_size			  = distance / min_used_thread;

			std::vector<std::future<void>> futures(min_used_thread - 1);
			std::vector<jthread>		   threads(min_used_thread - 1);

			auto						   it = begin;
			for (size_t i = 0; i < min_used_thread - 1; ++i) {
				auto to = it;
				std::advance(to, block_size);
				std::packaged_task<void(void)> task([&func, it, to]() -> void {
					std::for_each(it, to, func);
				});
				futures[i]		  = task.get_future();
				threads[i].thread = std::thread(std::move(task));
				it				  = to;
			}

			std::for_each(it, end, func);
			for (size_t i = 0; i < min_used_thread - 1; ++i) {
				// 只是为了传递异常
				futures[i].get();
			}
		}

		template<typename Iterator, typename T>
		Iterator parallel_find(Iterator begin, Iterator end, const T& value) {
			auto distance = std::distance(begin, end);
			if (distance == 0) {
				return end;
			}

			constexpr size_t	   min_require_per_thread = 25;
			const size_t		   max_threads			  = (distance + min_require_per_thread - 1) / min_require_per_thread;
			const size_t		   hardware_threads		  = std::thread::hardware_concurrency();

			const size_t		   min_used_thread		  = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
			const size_t		   block_size			  = distance / min_used_thread;

			std::promise<Iterator> ret;
			std::atomic<bool>	   done_flag(false);
			std::vector<jthread>   threads(min_used_thread - 1);
			find_element		   finder{};

			auto				   it = begin;
			for (size_t i = 0; i < min_used_thread - 1; ++i) {
				auto to = it;
				std::advance(to, block_size);
				threads[i].thread = std::thread(finder, it, to, value, &ret, &done_flag);
				it				  = to;
			}
			finder(it, end, value, &ret, &done_flag);

			if (!done_flag.load()) {
				return end;
			}
			return ret.get_future().get();
		}
	}// namespace detail
}// namespace misc

#endif//MISC_HPP
