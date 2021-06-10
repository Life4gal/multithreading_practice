#include "dir_watchdog.hpp"

#include <sys/inotify.h>
#include <unistd.h>

#include <cstdio>
#include <utility>

#include "error_logger.hpp"

namespace work {
	constexpr uint32_t dir_watchdog::all_event_mask[];
	constexpr size_t   dir_watchdog::all_event_mask_length;

	bool			   dir_watchdog::add_path(const std::string& path) {
		  auto fd = inotify_init();
		  if (fd < 0) {
			  LOG2FILE(log_level::ERROR, "inotify_init failed: " + path);
			  return false;
		  }

		  callbacks[fd].resize(all_event_mask_length);
		  return path_fd.emplace(path, fd).second;
	}

	int dir_watchdog::get_path_fd(const std::string& path) {
		auto it = path_fd.find(path);
		if (it == path_fd.end()) {
			LOG2FILE(log_level::ERROR, "Path not added yet: " + path);
			return path_not_added_code;
		}

		return it->second;
	}

	bool dir_watchdog::register_callback(const std::string& path, EVENT_TYPE event, callback_type callback, bool overwrite_if_exist) {
		auto fd = get_path_fd(path);
		if (fd == path_not_added_code) {
			return false;
		}

		auto& cb = callbacks[fd];

		for (size_t i = 0; i < all_event_mask_length; ++i) {
			if ((event >> i) & 1) {
				if (cb[i] && !overwrite_if_exist) {
					continue;
				}
				cb[i].swap(callback);
			}
		}
		return true;
	}

	std::vector<std::string> dir_watchdog::prepared(bool remove_if_not_ready) {
		std::vector<std::string> not_ready_path{};
		for (const auto& kv: path_fd) {
			const auto& cb		  = callbacks[kv.second];

			uint32_t	curr_mask = 0;
			for (size_t i = 0; i < all_event_mask_length; ++i) {
				if (cb[i]) {
					curr_mask |= all_event_mask[i];
				}
			}
			auto wd = inotify_add_watch(kv.second, kv.first.c_str(), curr_mask);

			if (wd < 0) {
				LOG2FILE(log_level::ERROR, "inotify_add_watch failed: " + kv.first);
				not_ready_path.push_back(kv.first);
				if (remove_if_not_ready) {
					path_fd.erase(kv.first);
				}
				continue;
			}
			fd_wd[kv.second] = wd;
		}

		return not_ready_path;
	}

	void dir_watchdog::run(const std::string& path) {
		auto fd = get_path_fd(path);
		if (fd == path_not_added_code) {
			return;
		}

		auto&	cb = callbacks[fd];

		char	buffer[BUFSIZ]{};
		ssize_t length;
		while ((length = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
			ssize_t curr = 0;
			while (length > 0) {
				auto* event = reinterpret_cast<inotify_event*>(buffer + curr);

				for (size_t i = 0; i < all_event_mask_length; ++i) {
					if ((event->mask >> i) & 1 && cb[i]) {
						cb[i](all_event_mask[i], event->name);
					}
				}

				curr += static_cast<ssize_t>(sizeof(inotify_event)) + event->len;
				length -= static_cast<ssize_t>(sizeof(inotify_event)) + event->len;
			}
		}
	}
}// namespace work
