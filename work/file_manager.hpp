#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <functional>
#include <string>
#include <vector>

#include "data_form_fwd.hpp"
#include "json_fwd.hpp"

namespace work {
	class file_manager {
	public:
		/**
		 * @brief 读取配置文件构建配置管理器
		 * @param config_path 配置文件路径
		 * @return 构建的配置管理器
		 */
		static data::data_config_manager load_config(const std::string& config_path);

		/**
		 * @brief 载入并解析一个文件
		 * @param detail 文件内容解释详情
		 * @param name 文件的类型，支持的类型见`FILE_TYPE get_file_type(const std::string& type)`
		 * @param filename 文件的名字
		 * @param delimiter 文件内容的分割符(每一行)
		 * @return 解析的文件数据
		 */
		static data::file_data_type			 load_file(
						 const data::data_source_field_detail& detail,
						 data::FILE_TYPE											 name,
						 const std::string&										 filename,
						 char																	 delimiter = '\t');

		/**
		 * @brief 获得所给路径中所有的文件
		 * @param path 路径
		 * @param recursive 是否递归搜索
		 * @param pred 用于用户进一步匹配所需要的文件名
		 * 注意，传递的参数是`完整的文件名`(如果有后缀则包含后缀，即使是递归的也不包含相对目录),
		 * @return 所有搜寻到的文件名
		 */
		static std::vector<std::string> get_files_in_path(
				const std::string&														 path,
				bool																					 recursive = false,
				const std::function<bool(const std::string&)>& pred			 = [](const std::string&) { return true; });

		/**
		 * @brief 获取目标文件路径的文件名
		 * @param path 目标的路径
		 * @return 文件名
		 */
		static std::string get_filename_in_path(const std::string& path);

		/**
		 * @brief 获取目标文件的绝对路径
		 * @param filename 文件名，可能直接是一个绝对路径
		 * @param current_path 当前路径，留空则为程序运行的路径
		 * @return 绝对路径
		 */
		static std::string get_absolute_path(const std::string& filename, const std::string& current_path = "");
	};
}// namespace work

#endif//FILE_MANAGER_HPP
