cmake_minimum_required(VERSION 2.8.4)
project(practice)

#set(CMAKE_CXX_STANDARD 11)

add_definitions(-std=c++11)

set(
		SOURCE
		thread_safe_stack.cpp
)

add_executable(
		${PROJECT_NAME}
		main.cpp
		${SOURCE}
)

target_link_libraries(
		${PROJECT_NAME}
		pthread
)
