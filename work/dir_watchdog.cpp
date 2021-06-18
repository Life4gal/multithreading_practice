#include "dir_watchdog.hpp"

#include <sys/inotify.h>
#include <unistd.h>

#include "error_logger.hpp"

namespace work {
	bool dir_watchdog::add_path(const std::string& path) {
		auto fd = inotify_init();
		if (fd < 0) {
			LOG2FILE(LOG_LEVEL::ERROR, "inotify_init failed: " + path);
			return false;
		}

		return path_fd.emplace(path, fd).second;
	}

	int dir_watchdog::get_path_fd(const std::string& path) {
		auto it = path_fd.find(path);
		if (it == path_fd.end()) {
			LOG2FILE(LOG_LEVEL::ERROR, "Path not added yet: " + path);
			return path_not_added_code;
		}

		return it->second;
	}

	bool dir_watchdog::set_watch(const std::string& path, EVENT_TYPE event, bool overwrite) {
		auto fd = get_path_fd(path);
		if (fd == path_not_added_code) {
			return false;
		}

		event_underlying_type curr_mask;
		if (overwrite) {
			curr_mask = event;
		} else {
			curr_mask = fd_mask[fd] | event;
		}

		auto wd = inotify_add_watch(fd, path.c_str(), curr_mask);
		if (wd < 0) {
			LOG2FILE(LOG_LEVEL::ERROR, "inotify_add_watch failed: " + path);
			return false;
		}

		fd_wd[fd]	= wd;
		fd_mask[fd] = curr_mask;
		return true;
	}

	bool dir_watchdog::set_callback(const std::string& path, const callback_type& callback, bool overwrite) {
		auto fd = get_path_fd(path);
		if (fd == path_not_added_code) {
			return false;
		}

		if (fd_callback.find(fd) == fd_callback.end() || overwrite) {
			return fd_callback.emplace(fd, callback).second;
		}
		return false;
	}

	void dir_watchdog::run(const std::string& path) {
		auto fd = get_path_fd(path);
		if (fd == path_not_added_code) {
			return;
		}

		auto&	callback = fd_callback[fd];

		char	buffer[BUFSIZ]{};
		ssize_t length;
		while ((length = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
			ssize_t curr = 0;
			while (length > 0) {
				auto* event = reinterpret_cast<inotify_event*>(buffer + curr);

				if (callback) {
					callback(event->mask, event->name);
					LOG2FILE(LOG_LEVEL::INFO, "Mask: " + std::to_string(event->mask) + " Name: " + event->name);
				} else {
					LOG2FILE(LOG_LEVEL::WARNING, "Callback not valid of fd: " + std::to_string(fd));
				}

				curr += static_cast<ssize_t>(sizeof(inotify_event)) + event->len;
				length -= static_cast<ssize_t>(sizeof(inotify_event)) + event->len;
			}
		}
	}
}// namespace work
