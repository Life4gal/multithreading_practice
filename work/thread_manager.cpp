#include "thread_manager.hpp"

namespace work {
	ThreadManager::~ThreadManager() {
		thread_group_.join_all();
	}
}// namespace work
