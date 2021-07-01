cmake_minimum_required(VERSION 2.8.4)
project(practice)

add_definitions(-std=c++2a)

set(
		SOURCE
		thread_safe_stack.cpp
		thread_safe_queue.cpp
		thread_safe_map.cpp
		# need libatomic
		#lock_free_stack.cpp
)

add_executable(
		${PROJECT_NAME}
		main.cpp
		${SOURCE}
)

set(Boost_USE_STATIC_LIBS ON)

find_package(
		Boost COMPONENTS
		system
		thread
		REQUIRED
)

target_link_libraries(
		${PROJECT_NAME}
		pthread
#		atomic
		${Boost_SYSTEM_LIBRARY}
		${Boost_THREAD_LIBRARY}
)
