#include "file_seeker.hpp"

#include <algorithm>
#include <fstream>

#include "error_logger.hpp"

std::vector<std::string> file_seeker::get_column_in_file(const std::string& filename, size_t column, char delimiter) {
	if (filename.empty()) {
		log_to_file("Error", __PRETTY_FUNCTION__, "Empty filename");
		return {};
	}

	std::ifstream file{filename};

	if (!file.is_open()) {
		log_to_file("Error", __PRETTY_FUNCTION__, "Cannot open file: " + filename);
		return {};
	}

	std::vector<std::string> ret{};

	std::string				 entire_line;
	while (std::getline(file, entire_line)) {
		// 长度应该至少有 （index + 1) * 2 - 1 这么多个 delimiter 那么长
		if (entire_line.length() < ((column + 1) * 2 - 1) * sizeof(delimiter)) {
			log_to_file("Error", __PRETTY_FUNCTION__, "Insufficient length: " + entire_line);
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
	if (filename.empty()) {
		log_to_file("Error", __PRETTY_FUNCTION__, "Empty filename");
		return;
	}

	std::ifstream file{filename};

	if (!file.is_open()) {
		log_to_file("Error", __PRETTY_FUNCTION__, "Cannot open file: " + filename);
		return;
	}

	// 排序一下，保证行数从小到大，提高效率
	std::sort(columns.begin(), columns.end());

	std::string entire_line;
	while (std::getline(file, entire_line)) {
		// 长度应该至少有 （columns.back() + 1) * 2 - 1 这么多个 delimiter 那么长
		if (entire_line.length() < ((columns.back() + 1) * 2 - 1) * sizeof(delimiter)) {
			log_to_file("Error", __PRETTY_FUNCTION__, "Insufficient length: " + entire_line);
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
