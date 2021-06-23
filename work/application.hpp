#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "data_form.hpp"
#include "dir_watchdog.hpp"
#include "file_manager.hpp"

namespace work {
	class Application {
	public:
		/**
		 * @brief 构造app
		 * @param config_path 用于初始化的配置文件路径
		 */
		explicit Application(std::string config_path);

		/**
		 * @brief 初始化程序
		 * @return 初始化是否成功
		 */
		bool Init();

		/**
		 * @brief 运行程序
		 */
		void Run();

	private:
		// support function below

		/**
		 * @brief 初始化watchdog
		 * @return watchdog是否初始化成功
		 */
		bool																				 InitWatchdog();
		/**
		 * @brief 处理已经存在的文件
		 */
		void																				 ProcessAllExistFile();
		/**
		 * @brief 唤醒watchdog，进入监视状态
		 */
		void																				 WakeUpWatchdog();

		/**
		 * @brief 解析文件
		 * @param filename 目标文件
		 * @param dir_name 目标所在目录
		 * @param filename_pattern 用于获取时间的正则表达式
		 * @param detail 目标的详细字段详情
		 * @param type 解析的类型
		 * @return 数据时间戳与数据组成的pair
		 */
		std::pair<std::string, data::FileDataType> DoResolveData(
				const std::string&										filename,
				const std::string&										dir_name,
				const std::string&										filename_pattern,
				const data::DataSourceFieldDetail& detail,
				const std::string&										type);

		/**
		 * @brief post给予的数据
		 * @param time 数据的时间戳，时间戳为空不进行post
		 * @param data 数据，数据为空不进行post
		 */
		void DoPostData(const std::string& time, const data::FileDataType& data) const;

		/**
		 * @brief 解析文件并且post
		 * @param filename 目标文件
		 * @param dir_name 目标所在目录
		 * @param filename_pattern 用于获取时间的正则表达式
		 * @param detail 目标的详细字段详情
		 * @param type 解析的类型
		 */
		void DoResolveAndPostData(
				const std::string&										filename,
				const std::string&										dir_name,
				const std::string&										filename_pattern,
				const data::DataSourceFieldDetail& detail,
				const std::string&										type);

		/**
		 * @brief 获取目标的完整时间
		 * @param time_str 目标文件的时间
		 * @param folder_str 目标所在的文件夹
		 * @return 一个键值对，first表示是否获取成功，second表示如果获取成功，完整的时间是什么
		 */
		std::pair<bool, std::string> GetTargetFullTime(const std::string& time_str, const std::string& folder_str) const;

		// member data below

		// 配置文件路径
		std::string									 config_path_;
		// 配置管理器，包含所需的所有配置
		data::DataConfigManager			 config_manager_;
		// 用于监控文件的watchdog
		DirWatchdog									 watchdog_;
	};
}// namespace work


#endif//APPLICATION_HPP
