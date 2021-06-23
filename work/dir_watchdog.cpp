#include "dir_watchdog.hpp"

#include <sys/inotify.h>
#include <unistd.h>

#include <boost/thread/v2/thread.hpp>

#include "error_logger.hpp"

namespace work {
	bool DirWatchdog::AddPath(const std::string& path) {
		auto fd = inotify_init();
		if (fd < 0) {
			LOG2FILE(LOG_LEVEL::ERROR, "inotify_init failed: " + path);
			return false;
		}

		return path_fd_.emplace(path, fd).second;
	}

	int DirWatchdog::GetPathFd(const std::string& path) {
		auto it = path_fd_.find(path);
		if (it == path_fd_.end()) {
			LOG2FILE(LOG_LEVEL::ERROR, "Path not added yet: " + path);
			return path_not_added_code;
		}

		return it->second;
	}

	bool DirWatchdog::SetWatch(const std::string& path, EVENT_TYPE event, bool overwrite) {
		auto fd = GetPathFd(path);
		if (fd == path_not_added_code) {
			return false;
		}

		event_underlying_type curr_mask;
		if (overwrite) {
			curr_mask = event;
		} else {
			curr_mask = fd_mask_[fd] | event;
		}

		auto wd = inotify_add_watch(fd, path.c_str(), curr_mask);
		if (wd < 0) {
			LOG2FILE(LOG_LEVEL::ERROR, "inotify_add_watch failed: " + path);
			return false;
		}

		fd_wd_[fd]	 = wd;
		fd_mask_[fd] = curr_mask;
		return true;
	}

	bool DirWatchdog::SetCallback(const std::string& path, const callback_type& callback, bool overwrite) {
		auto fd = GetPathFd(path);
		if (fd == path_not_added_code) {
			return false;
		}

		if (fd_callback_.find(fd) == fd_callback_.end() || overwrite) {
			return fd_callback_.emplace(fd, callback).second;
		}
		return false;
	}

	void DirWatchdog::Run(const std::string& path) {
		auto fd = GetPathFd(path);
		if (fd == path_not_added_code) {
			return;
		}

		auto&		callback = fd_callback_[fd];

		char		buffer[BUFSIZ]{};
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
