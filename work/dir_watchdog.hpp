#ifndef DIR_WATCHDOG_HPP
#define DIR_WATCHDOG_HPP

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace work {
	class dir_watchdog {
	public:
		using event_underlying_type = uint32_t;
		enum EVENT_TYPE : event_underlying_type {
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

		constexpr static int  path_not_added_code = -1;

		using callback_type						  = std::function<void(event_underlying_type, const std::string&)>;

		// 添加一个路径
		bool add_path(const std::string& path);

		// 通过路径获取fd
		int	 get_path_fd(const std::string& path);

		// 设置目标路径监控的事件
		bool set_watch(const std::string& path, EVENT_TYPE event, bool overwrite = false);

		// 给目标路径注册回调函数
		bool set_callback(const std::string& path, const callback_type& callback, bool overwrite = false);

		// 如此设计是允许用户多线程运行 watchdog
		void run(const std::string& path);

	private:
		// path <-> file_descriptor
		std::map<std::string, int>			 path_fd;
		// file_descriptor <-> watch_descriptor
		std::map<int, int>					 fd_wd;
		// file_descriptor <-> event_mask
		std::map<int, event_underlying_type> fd_mask;
		// file_descriptor <-> callback
		std::map<int, callback_type>		 fd_callback;
	};
}// namespace work

#endif//DIR_WATCHDOG_HPP
