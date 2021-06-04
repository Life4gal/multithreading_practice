#include "file_seeker.hpp"

#include <algorithm>
#include <fstream>

#include "error_logger.hpp"

namespace {
	bool do_file_validate(const std::string& filename, std::ifstream& out) {
		if (filename.empty()) {
			log_to_file("Error", __PRETTY_FUNCTION__, "Empty filename");
			return false;
		}

		out.open(filename);

		if (!out.is_open()) {
			log_to_file("Error", __PRETTY_FUNCTION__, "Cannot open file: " + filename);
			return false;
		}

		return true;
	}

	bool do_line_length_validate(const std::string& line, std::string::size_type element_num) {
		// 长度应该至少有 （index + 1) * 2 - 1 这么多个 delimiter(char) 那么长
		if (line.length() < ((element_num + 1) * 2 - 1) * sizeof(char)) {
			log_to_file("Error", __PRETTY_FUNCTION__, "Insufficient length: " + line);
			return false;
		}
		return true;
	}
}// namespace

std::vector<std::string> file_seeker::get_column_in_file(const std::string& filename, size_t column, char delimiter) {
	std::ifstream file;
	if (!do_file_validate(filename, file)) {
		return {};
	}

	std::vector<std::string> ret{};

	std::string				 entire_line;
	while (std::getline(file, entire_line)) {
		if (!do_line_length_validate(entire_line, column)) {
			continue;
		}

		std::string::size_type it = 0;
		for (auto i = 0; i <= column; ++i) {
			// 加上 sizeof(delimiter)的偏移以跳过这个delimiter
			it = entire_line.find(delimiter, it) + sizeof(delimiter);
		}

		decltype(it) next = entire_line.find(delimiter, it);

		ret.push_back(entire_line.substr(it, next - it));
	}

	return ret;
}

void file_seeker::count_columns_combination_in_file(const std::string& filename, std::vector<size_t> columns, std::map<std::string, size_t>& out, char delimiter, const std::string& combination) {
	std::ifstream file;
	if (!do_file_validate(filename, file)) {
		return;
	}

	// 排序一下，保证行数从小到大，提高效率
	std::sort(columns.begin(), columns.end());

	std::string entire_line;
	while (std::getline(file, entire_line)) {
		if (!do_line_length_validate(entire_line, columns.back())) {
			continue;
		}

		std::string					  curr_key{};
		std::string::size_type		  it		  = 0;
		decltype(columns)::value_type last_column = 0;
		for (auto curr_column = columns.cbegin(); curr_column != columns.cend(); ++curr_column) {
			for (auto column_seeker = last_column; column_seeker != *curr_column; ++column_seeker) {
				// 加上 sizeof(delimiter)的偏移以跳过这个delimiter
				it = entire_line.find(delimiter, it) + sizeof(delimiter);
			}

			decltype(it) next = entire_line.find(delimiter, it);

			curr_key.append(entire_line.substr(it, next - it));

			// 不是最后一个
			if (curr_column != columns.cend() - 1) {
				curr_key.append(combination);
			}

			it			= next + sizeof(delimiter);
			last_column = *curr_column;
		}

		out[curr_key] += 1;
	}
}

void file_seeker::count_columns_combination_in_files(const std::vector<std::string>& files, const std::vector<size_t>& columns, std::map<std::string, size_t>& out, char delimiter, const std::string& combination) {
	std::map<std::string, size_t> ret{};

	for (const auto& file: files) {
		count_columns_combination_in_file(file, columns, ret, delimiter, combination);
	}
}
