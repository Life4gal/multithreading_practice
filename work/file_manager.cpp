#include "file_manager.hpp"

#include <boost/filesystem.hpp>

#include "data_form.hpp"
#include "error_logger.hpp"

namespace {
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

	bool do_line_length_validate(const std::string& line, std::string::size_type element_num) {
		// 长度应该至少有 （index + 1) * 2 - 1 这么多个 delimiter(char) 那么长
		if (line.length() < ((element_num + 1) * 2 - 1) * sizeof(char)) {
			LOG2FILE(LOG_LEVEL::ERROR, "Insufficient length: " + line);
			return false;
		}
		return true;
	}

	std::string do_get_substr(const std::string& line, std::string::size_type which_column, char delimiter) {
		if (!do_line_length_validate(line, which_column)) {
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

	template<typename Iterator>
	void do_find(std::vector<std::string>& out, const std::string& suffix, const std::function<bool(const std::string&)>& pred, Iterator begin, Iterator end = {}) {
		bool need_extension = !suffix.empty();
		for (; begin != end; ++begin) {
			auto filename = begin->path().filename();
			const auto& str = filename.string();
			if (pred(str)) {
				if (need_extension && (!filename.has_extension() || filename.extension().string() != suffix)) {
					continue;
				}

				out.push_back(str);
			}
			else
			{
				continue;
			}

			LOG2FILE(LOG_LEVEL::INFO, "Found file: " + str);
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

		bool				 ignore_code = detail.code == data::data_source_field_detail::ignore_code;
		data::file_data_type ret{};

		ret.reserve(detail.field.size());
		for (const auto& kv: detail.field) {
			// 只设置类型
			ret.push_back({kv.first});
		}

		std::string entire_line;
		while (std::getline(file, entire_line)) {
			// validate code
			if (!ignore_code && do_get_substr(entire_line, detail.code, delimiter).front() != '0') {
				LOG2FILE(LOG_LEVEL::INFO, "Code not zero, skip this line: " + entire_line);
				continue;
			}

			auto layer = static_cast<data::data_source_field_detail::size_type>(stoull(do_get_substr(entire_line, detail.layer, delimiter), nullptr));
			auto price = static_cast<data::data_source_field_detail::value_type>(stoull(do_get_substr(entire_line, detail.price, delimiter), nullptr));

			for (const auto& kv: detail.field) {
				auto  this_field = do_get_substr(entire_line, kv.second, delimiter);
				auto& type		 = kv.first;
				auto  it		 = std::find_if(ret.begin(), ret.end(), [&type](const data::data_with_type& data) { return data.type == type; });
				it->data[this_field].increase(layer, name, price);
				if (std::find(detail.pad_field_name.cbegin(), detail.pad_field_name.cend(), kv.first) != detail.pad_field_name.cend()) {
					it->data[this_field].pad_json = nlohmann::json::parse(detail.pad_data);
				}
			}
		}

		return ret;
	}

	std::vector<std::string> file_manager::get_files_in_path(
			const std::string&							   path,
			bool										   recursive,
			const std::string&							   suffix,
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
				do_find(ret, suffix, pred, fs::recursive_directory_iterator{dir_path});
			} else {
				do_find(ret, suffix, pred, fs::directory_iterator{dir_path});
			}

			return ret;
		} else {
			LOG2FILE(LOG_LEVEL::ERROR, "Empty path");
			return {};
		}
	}
}// namespace work
