#ifndef DIR_WATCHDOG_HPP
#define DIR_WATCHDOG_HPP

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace work {
	class DirWatchdog {
	public:
		using event_underlying_type = uint32_t;
		enum EVENT_TYPE : event_underlying_type {
			// 文件被访问
			IN_ACCESS					= 0x00000001,
			// 文件被修改
			IN_MODIFY					= 0x00000002,
			// 文件的属性被改变
			IN_ATTRIB					= 0x00000004,
			// 以写模式打开(或者说文件被修改后)的文件被关闭
			IN_CLOSE_WRITE		= 0x00000008,
			// 以只读模式打开(或者说文件没有被修改)的文件被关闭
			IN_CLOSE_NO_WRITE = 0x00000010,
			// 文件被关闭
			IN_CLOSE					= (IN_CLOSE_WRITE | IN_CLOSE_NO_WRITE),

			// 文件被打开
			IN_OPEN						= 0x00000020,
			// 文件从X移动到(改名为)Y，事件获取到X的名字
			IN_MOVED_FROM			= 0x00000040,
			// 文件从X移动到(改名为)Y，事件获取到Y的名字
			IN_MOVED_TO				= 0x00000080,
			// 文件从X移动到(改名为)Y
			IN_MOVE						= (IN_MOVED_FROM | IN_MOVED_TO),

			// 子文件被创建
			IN_CREATE					= 0x00000100,
			// 子文件被删除
			IN_DELETE					= 0x00000200,
			// 自身被删除
			IN_DELETE_SELF		= 0x00000400,
			// 自身被移动(改名)
			IN_MOVE_SELF			= 0x00000800,
			// 所有事件
			IN_ALL_EVENTS =
					IN_ACCESS | IN_MODIFY | IN_ATTRIB |
					IN_CLOSE_WRITE | IN_CLOSE_NO_WRITE | IN_OPEN |
					IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE |
					IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF
		};

		/**
		 * @brief 文件没有被映射的情况下获取到的fd
		 */
		constexpr static int path_not_added_code = -1;

		/**
		 * @brief callback函数类型，参数为事件的代码以及文件的名字
		 */
		using callback_type											 = std::function<void(event_underlying_type, const std::string&)>;

		/**
		 * @brief 添加一个路径到映射表
		 * @param path 要添加的路径
		 * @return 添加是否成功
		 */
		bool AddPath(const std::string& path);

		/**
		 * @brief 通过路径获取fd
		 * @param path 目标路径
		 * @return 获取的fd
		 */
		int	 GetPathFd(const std::string& path);

		/**
		 * @brief 设置目标路径监控的事件
		 * @param path 目标路径
		 * @param event 监控的事件
		 * @param overwrite 是否覆写(也就是说如果之前监控的事件被没有监控的事件覆写，将不会在监控那个事件)
		 * @return 设置是否成功
		 */
		bool SetWatch(const std::string& path, EVENT_TYPE event, bool overwrite = false);

		/**
		 * @brief 给目标路径注册回调函数
		 * @param path 目标路径
		 * @param callback 回调函数
		 * @param overwrite 是否覆写
		 * 注意，无法对一个事件调用两个不同的回调函数，需要用户自己处理
		 * 也无法对不同的事件调用不同的回调函数
		 * 监控的所有事件中只要有一个事件被触发就会调用回调函数
		 * @return 设置是否成功
		 */
		bool SetCallback(const std::string& path, const callback_type& callback, bool overwrite = false);

		/**
		 * @brief 开始监控目标路径，如此设计是允许用户多线程运行watchdog
		 * @param path 目标路径
		 */
		void Run(const std::string& path);

	private:
		/**
		 * @brief 监控的路径 <-> 路径对应的file descriptor
		 */
		std::map<std::string, int>					 path_fd_;
		/**
		 * @brief file_descriptor <-> watch_descriptor
		 */
		std::map<int, int>									 fd_wd_;
		/**
		 * @brief file_descriptor <-> event_mask
		 */
		std::map<int, event_underlying_type> fd_mask_;
		/**
		 * @brief file_descriptor <-> callback
		 */
		std::map<int, callback_type>				 fd_callback_;
	};
}// namespace work

#endif//DIR_WATCHDOG_HPP
