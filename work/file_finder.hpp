#ifndef FILE_FINDER_HPP
#define FILE_FINDER_HPP

#include <string>
#include <vector>
#include <chrono>

class file_finder
{
public:
	/**
	 * @brief 获得所给路径中所有的文件
	 * @param path 路径
	 * @param suffix 后缀，留空表示无后缀
	 * @param recursive 是否递归搜索
	 * @return 所有搜寻到的文件名
	 */
	static std::vector<std::string> get_files_in_path(const std::string& path, const std::string& suffix, bool recursive);
};

#endif//FILE_FINDER_HPP
