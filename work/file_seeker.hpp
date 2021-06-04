#ifndef FILE_SEEKER_HPP
#define FILE_SEEKER_HPP

#include <map>
#include <string>
#include <vector>

class file_seeker {
public:
	// todo: 统计函数大量冗余

	/**
	 * @brief 统计文件所有行中的某一列的内容
	 * @param filename 文件的名字
	 * @param column 内容所在列
	 * @param delimiter 分隔符
	 * @return 所有统计的列
	 */
	static std::vector<std::string> get_column_in_file(const std::string& filename, size_t column, char delimiter = '\t');

	/**
	 * @brief 统计文件所有行中某几列组合的内容出现的次数
	 * @param filename 文件的名字
	 * @param columns 所有列
	 * @param out 输出所有组合及其出现的次数
	 * @param delimiter 分隔符
	 * @param combination 组合方式，默认为在两列之间插入 `##`
	 */
	static void						count_columns_combination_in_file(const std::string& filename, std::vector<size_t> columns, std::map<std::string, size_t>& out, char delimiter = '\t', const std::string& combination = "##");

	/**
	 * @brief 统计所有文件所有行中某几列组合的内容出现的次数
	 * @param files 所有文件
	 * @param columns 所有列
	 * @param out 输出所有组合及其出现的次数
	 * @param delimiter 分隔符
	 * @param combination 组合方式，默认为在两列之间插入 `##`
	 */
	static void						count_columns_combination_in_files(const std::vector<std::string>& files, const std::vector<size_t>& columns, std::map<std::string, size_t>& out, char delimiter = '\t', const std::string& combination = "##");
};

#endif//FILE_SEEKER_HPP
