#include "file_manager.hpp"

#include <boost/filesystem.hpp>

#include "data_form.hpp"
#include "error_logger.hpp"

namespace {
	/**
	 * @brief 验证一个文件是否可以打开
	 * @param filename 文件的名字
	 * @param out 打开的文件流
	 * @return 是否成功打开
	 */
	bool do_file_validate(const std::string& filename, std::ifstream& out) {
		if (filename.empty()) {
			LOG2FILE(LOG_LEVEL::ERROR, "Empty filename");
			return false;
		}

		out.open(filename);

		if (!out.is_open()) {
			LOG2FILE(LOG_LEVEL::ERROR, "Cannot open file: " + filename);
			return false;
		}

		return true;
	}

	/**
	 * @brief 判断一个字符串是否有期待的长度
	 * @param line 字符串
	 * @param element_num 期待的元素个数
	 * @param delimiter 分隔符
	 * @return 是否有期待长度
	 */
	bool do_line_length_validate(const std::string& line, std::string::size_type element_num, char delimiter) {
		// 长度应该至少有 （index + 1) * 2 - 1 这么多个 delimiter(char) 那么长
		if (line.length() < ((element_num + 1) * 2 - 1) * sizeof(char) || std::count(line.cbegin(), line.cend(), delimiter) < element_num) {
			LOG2FILE(LOG_LEVEL::ERROR, "Insufficient length: " + line);
			return false;
		}
		return true;
	}

	/**
	 * @brief 获取字符串某一列的子字符串
	 * @param line 字符串
	 * @param which_column 某一列
	 * @param delimiter 分隔符
	 * @return 子字符串，如果长度不够返回空串
	 */
	std::string do_get_substr(const std::string& line, std::string::size_type which_column, char delimiter) {
		if (!do_line_length_validate(line, which_column, delimiter)) {
			return {};
		}
		decltype(which_column) it = 0;
		for (decltype(which_column) this_column = 0; this_column < which_column; ++this_column) {
			// 加上 sizeof(delimiter)的偏移以跳过这个delimiter
			it = line.find(delimiter, it) + sizeof(delimiter);
		}
		decltype(it) next = line.find(delimiter, it);
		return line.substr(it, next - it);
	}

	/**
	 * @brief 判断字符串是否符合所有代码
	 * @param code 代码集合
	 * @param line 字符串
	 * @param delimiter 分隔符
	 * @return 是否符合
	 */
	bool do_valid_code(const std::unordered_map<std::string, work::data::data_source_code_detail>& code, const std::string& line, char delimiter) {
		return std::all_of(code.cbegin(), code.cend(), [delimiter, &line](const std::pair<std::string, work::data::data_source_code_detail>& name_code) {
			auto code_str = do_get_substr(line, name_code.second.column, delimiter);
			if (code_str.empty()) {
				LOG2FILE(LOG_LEVEL::ERROR, "Valid code error, cannot get str of " + name_code.first + "'s column " + std::to_string(name_code.second.column));
				// todo: 如果给定的`列`不合法应该直接结束判断还是跳过这个判断？
				return false;
			}

			return name_code.second.accept(stoull(code_str, nullptr));
		});
	}

	/**
	 * @brief 获取符合条件的所有文件名字(绝对路径)
	 * @tparam Iterator 迭代器的类型(拥用于支持递归)
	 * @param out 输出所有文件名字
	 * @param pred 条件判断
	 * @param begin 起始迭代器
	 * @param end 终止迭代器
	 */
	template<typename Iterator>
	void do_find(std::vector<std::string>& out, const std::function<bool(const std::string&)>& pred, Iterator begin, Iterator end = {}) {
		for (; begin != end; ++begin) {
			auto p	   = begin->path();
			auto abs_p = absolute(p).string();

			if (pred(p.filename().string())) {
				out.push_back(abs_p);
			} else {
				continue;
			}

			LOG2FILE(LOG_LEVEL::INFO, "Found file: " + abs_p);
		}
	}
}// namespace

namespace work {
	data::data_config_manager file_manager::load_config(const std::string& config_path) {
		std::ifstream config;
		if (!do_file_validate(config_path, config)) {
			return {};
		}

		nlohmann::json json;
		config >> json;
		return json.get<data::data_config_manager>();
	}

	data::file_data_type file_manager::load_file(const data::data_source_field_detail& detail, data::FILE_TYPE name, const std::string& filename, char delimiter) {
		std::ifstream file;
		if (!do_file_validate(filename, file)) {
			return {};
		}

		//		LOG2FILE(LOG_LEVEL::INFO, "Logging for " + nlohmann::json{detail}.dump());

		data::file_data_type ret{};

		ret.reserve(detail.field.size());
		for (const auto& kv: detail.field) {
			// 只设置类型
			ret.push_back({kv.first});
		}

		std::string entire_line;
		while (std::getline(file, entire_line)) {
			// validate code
			if (!do_valid_code(detail.code, entire_line, delimiter)) {
				continue;
			}

			data::data_source_field_detail::value_type price = 0;
			data::data_source_field_detail::size_type  layer;

			{
				auto it = detail.code.find("price");
				if (it != detail.code.end()) {
					auto price_str = do_get_substr(entire_line, it->second.column, delimiter);
					if (price_str.empty()) {
						LOG2FILE(LOG_LEVEL::ERROR, std::string{"Invalid str "}.append(price_str).append(" of ").append(it->first).append(" in ").append(entire_line));
					} else {
						price = stoull(price_str, nullptr);
					}
				}
			}

			auto layer_str = do_get_substr(entire_line, detail.layer, delimiter);
			if (layer_str.empty()) {
				LOG2FILE(LOG_LEVEL::ERROR, std::string{"Invalid layer "}.append(layer_str).append(" in ").append(entire_line));
				continue;
			} else {
				layer = static_cast<data::data_source_field_detail::size_type>(stoull(layer_str, nullptr));
			}

			for (const auto& kv: detail.field) {
				auto  id   = do_get_substr(entire_line, kv.second, delimiter);
				auto& type = kv.first;
				auto  it   = std::find_if(ret.begin(), ret.end(), [&type](const data::data_with_type& data) { return data.type == type; });
				it->data[id].increase(layer, name, price);
				if (std::find(detail.pad_field_name.cbegin(), detail.pad_field_name.cend(), kv.first) != detail.pad_field_name.cend()) {
					it->data[id].pad_json = nlohmann::json::parse(detail.pad_data);
				}
			}
		}

		return ret;
	}

	std::vector<std::string> file_manager::get_files_in_path(
			const std::string&							   path,
			bool										   recursive,
			const std::function<bool(const std::string&)>& pred) {
		if (!path.empty()) {
			namespace fs = boost::filesystem;

			fs::path dir_path{path};
			if (!is_directory(dir_path)) {
				LOG2FILE(LOG_LEVEL::ERROR, "Path is not a directory: " + path);
				return {};
			}

			std::vector<std::string> ret{};
			if (recursive) {
				do_find(ret, pred, fs::recursive_directory_iterator{dir_path});
			} else {
				do_find(ret, pred, fs::directory_iterator{dir_path});
			}

			return ret;
		} else {
			LOG2FILE(LOG_LEVEL::ERROR, "Empty path");
			return {};
		}
	}

	std::string file_manager::get_filename_in_path(const std::string& path) {
		namespace fs = boost::filesystem;
		return fs::path{path}.filename().string();
	}

	std::string file_manager::get_absolute_path(const std::string& filename, const std::string& current_path) {
		namespace fs = boost::filesystem;
		fs::path p{filename};
		if (p.is_absolute()) {
			return p.string();
		} else {
			return current_path.empty() ? absolute(p).string() : absolute(p, current_path).string();
		}
	}
}// namespace work
