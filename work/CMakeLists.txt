cmake_minimum_required(VERSION 2.8.4)
project(work)

add_definitions(-std=c++11)

set(
		SOURCE
		error_logger.cpp
		data_form.cpp
		file_manager.cpp
		net_manager.cpp
		dir_watchdog.cpp
		thread_manager.cpp
		application.cpp
)

add_executable(
		${PROJECT_NAME}
		${SOURCE}
		main.cpp
)

include_directories(usr/local/include/boost)

set(Boost_USE_STATIC_LIBS ON)

find_package(
		Boost COMPONENTS
		system
		filesystem
		thread
		regex
		REQUIRED
)

target_link_libraries(
		${PROJECT_NAME}
		curl
		pthread
		${Boost_SYSTEM_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY}
		${Boost_THREAD_LIBRARY}
		${Boost_REGEX_LIBRARY}
)
