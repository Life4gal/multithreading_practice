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
		static data::data_config_manager load_config(const std::string& config_path);
		static data::file_data_type		 load_file(const data::data_source_field_detail& detail, data::FILE_TYPE name, const std::string& filename, char delimiter = '\t');

		/**
		 * @brief 获得所给路径中所有的文件
		 * @param path 路径
		 * @param recursive 是否递归搜索
		 * @param suffix 后缀,留空表示无后缀
		 * @param pred 用于用户进一步匹配所需要的文件名，
		 * 注意:
		 * 传递的参数是`完整的文件名`(如果有后缀则包含后缀，即使是递归的也不包含相对目录),
		 * 先进行 pred 再进行后缀判断
		 * @return 所有搜寻到的文件名
		 */
		static std::vector<std::string>	 get_files_in_path(
				 const std::string&								path,
				 bool											recursive = false,
				 const std::string&								suffix	  = {},
				 const std::function<bool(const std::string&)>& pred	  = [](const std::string&) { return true; });
	};
}// namespace work

#endif//FILE_MANAGER_HPP
