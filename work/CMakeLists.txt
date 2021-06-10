cmake_minimum_required(VERSION 2.8.4)
project(work)

add_definitions(-std=c++11)

set(
		SOURCE
		error_logger.cpp
		file_manager.cpp
		net_manager.cpp
		dir_watchdog.cpp
		thread_manager.cpp
		application.cpp
)

add_executable(
		${PROJECT_NAME}
		${SOURCE}
#		main.cpp
		test_main.cpp
)

find_package(
		Boost COMPONENTS
		system
		filesystem
		thread
		REQUIRED
)

target_link_libraries(
		${PROJECT_NAME}
		curl
		${Boost_SYSTEM_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY}
		${Boost_THREAD_LIBRARY}
)
