#include "thread_manager.hpp"

thread_manager::~thread_manager() {
	thread_group.join_all();
}
