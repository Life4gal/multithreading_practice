cmake_minimum_required(VERSION 2.8.4)
project(work)

#set(CMAKE_CXX_STANDARD 11)

add_definitions(-std=c++11)

set(
		SOURCE
		error_logger.cpp
		url_sender.cpp
		thread_manager.cpp
		file_finder.cpp
		file_seeker.cpp
)

add_executable(
		${PROJECT_NAME}
		${SOURCE}
		main.cpp
)

find_package(
		Boost COMPONENTS
		system
		filesystem
		#asio
		thread
		REQUIRED
)

target_link_libraries(
		${PROJECT_NAME}
		curl
		${Boost_SYSTEM_LIBRARY}
		${Boost_FILESYSTEM_LIBRARY}
		#${Boost_ASIO_LIBRARY}
		${Boost_THREAD_LIBRARY}
)
