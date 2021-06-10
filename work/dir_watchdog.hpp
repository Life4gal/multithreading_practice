#ifndef DIR_WATCHDOG_HPP
#define DIR_WATCHDOG_HPP

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace work {
	class dir_watchdog {
	public:
		enum EVENT_TYPE : uint32_t {
			IN_ACCESS		  = 0x00000001,							  /* File was accessed.  */
			IN_MODIFY		  = 0x00000002,							  /* File was modified.  */
			IN_ATTRIB		  = 0x00000004,							  /* Metadata changed.  */
			IN_CLOSE_WRITE	  = 0x00000008,							  /* Writable file was closed. */
			IN_CLOSE_NO_WRITE = 0x00000010,							  /* Un-writable file closed.   */
			IN_CLOSE		  = (IN_CLOSE_WRITE | IN_CLOSE_NO_WRITE), /* Close. */

			IN_OPEN			  = 0x00000020,					   /* File was opened.  */
			IN_MOVED_FROM	  = 0x00000040,					   /* File was moved from X.  */
			IN_MOVED_TO		  = 0x00000080,					   /* File was moved to Y.  */
			IN_MOVE			  = (IN_MOVED_FROM | IN_MOVED_TO), /* Moves.  */

			IN_CREATE		  = 0x00000100, /* Sub-file was created.  */
			IN_DELETE		  = 0x00000200, /* Sub-file was deleted.  */
			IN_DELETE_SELF	  = 0x00000400, /* Self was deleted.  */
			IN_MOVE_SELF	  = 0x00000800, /* Self was moved.  */
			IN_ALL_EVENTS =
					IN_ACCESS | IN_MODIFY | IN_ATTRIB |
					IN_CLOSE_WRITE | IN_CLOSE_NO_WRITE | IN_OPEN |
					IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE |
					IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF /* All events which a program can wait on.  */
		};

		constexpr static uint32_t all_event_mask[]{
				IN_ACCESS,
				IN_MODIFY,
				IN_ATTRIB,
				IN_CLOSE_WRITE,
				IN_CLOSE_NO_WRITE,
				IN_OPEN,
				IN_MOVED_FROM,
				IN_MOVED_TO,
				IN_CREATE,
				IN_DELETE,
				IN_DELETE_SELF,
				IN_MOVE_SELF,
		};
		constexpr static size_t all_event_mask_length = 3 * 4;// 3 hex digit
		constexpr static int path_not_added_code = -1;

		using callback_type = std::function<void(uint32_t, std::string)>;

		// 添加一个路径
		bool add_path(const std::string& path);

		// 通过路径获取fd
		int get_path_fd(const std::string& path);

		// 给目标路径注册回调函数
		bool register_callback(const std::string& path, EVENT_TYPE event, callback_type callback, bool overwrite_if_exist = false);

		// 准备 watchdog 并获取未准备的路径
		std::vector<std::string> prepared(bool remove_if_not_ready = true);

		// 如此设计是允许用户多线程运行 watchdog
		void run(const std::string& path);

	private:
		// path <-> file_descriptor
		std::map<std::string, int> path_fd;
		// file_descriptor <-> watch_descriptor
		std::map<int, int> fd_wd;
		// file_descriptor <-> callbacks
		std::map<int, std::vector<callback_type>> callbacks;
	};
}// namespace work

#endif//DIR_WATCHDOG_HPP
