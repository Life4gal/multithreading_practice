#include "thread_manager.hpp"

namespace work {
	thread_manager::~thread_manager() {
		thread_group.join_all();
	}
}// namespace work
