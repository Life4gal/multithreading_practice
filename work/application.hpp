#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "data_form.hpp"
#include "dir_watchdog.hpp"
#include "file_manager.hpp"

namespace work {
	class application {
	public:
		/**
		 * @brief 构造app
		 * @param config_path 用于初始化的配置文件路径
		 */
		explicit application(std::string config_path);

		/**
		 * @brief 初始化程序
		 * @return 初始化是否成功
		 */
		bool init();

		/**
		 * @brief 运行程序
		 */
		void run();

	private:
		/**
		 * @brief 初始化watchdog
		 * @return watchdog是否初始化成功
		 */
		bool init_watchdog();
		/**
		 * @brief 处理已经存在的文件
		 */
		void process_all_exist_file();
		/**
		 * @brief 唤醒watchdog，进入监视状态
		 */
		void wake_up_watchdog();

		/**
		 * @brief 解析文件并且将其post
		 * @param filename 目标文件
		 * @param dir_name 目标所在目录
		 * @param filename_pattern 用于获取时间的正则表达式
		 * @param detail 目标的详细字段详情
		 * @param type 解析的类型
		 */
		void do_resolve_and_post(
				const std::string&										filename,
				const std::string&										dir_name,
				const std::string&										filename_pattern,
				const data::data_source_field_detail& detail,
				const std::string&										type);

		/**
		 * @brief 获取目标的完整时间
		 * @param time_str 目标文件的时间
		 * @param from 目标所在的文件夹
		 * @return 一个键值对，first表示是否获取成功，second表示如果获取成功，完整的时间是什么
		 */
		std::pair<bool, std::string> get_target_full_time(const std::string& time_str, const std::string& from) const;

		// 配置文件路径
		std::string									 path;
		// 配置管理器，包含所需的所有配置
		data::data_config_manager		 config;
		// 用于监控文件的watchdog
		dir_watchdog								 watchdog;
	};
}// namespace work


#endif//APPLICATION_HPP
