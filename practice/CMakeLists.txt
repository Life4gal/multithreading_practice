cmake_minimum_required(VERSION 2.8.4)
project(practice)

#set(CMAKE_CXX_STANDARD 11)

add_definitions(-std=c++11)

set(
		SOURCE
		thread_safe_stack.cpp
		thread_safe_queue.cpp
		thread_safe_map.cpp
)

add_executable(
		${PROJECT_NAME}
		main.cpp
		${SOURCE}
)

include_directories(usr/local/include/boost)

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
		${Boost_SYSTEM_LIBRARY}
		${Boost_THREAD_LIBRARY}
)
