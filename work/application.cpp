#include "application.hpp"

#include <boost/regex.hpp>
#include <regex>

#include "error_logger.hpp"
#include "net_manager.hpp"
#include "thread_manager.hpp"

namespace work {
	application::application(std::string config_path)
		: path(std::move(config_path)) {
	}

	bool application::init() {
		config = file_manager::load_config(path);
		if (!init_watchdog()) {
			return false;
		}
		wake_up_watchdog();
		return true;
	}

	void application::run() {
		thread_manager thread;
		thread.push_function(&application::process_all_exist_file, this);

		for (const auto& name_source: config.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				thread.push_function(&dir_watchdog::run, &watchdog, dir_path_detail.first);
			}
		}
	}

	bool application::init_watchdog() {
		for (const auto& name_source: config.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				if (!watchdog.add_path(dir_path_detail.first)) {
					return false;
				}
			}
		}
		return true;
	}

	bool application::process_all_exist_file() {
		for (const auto& name_source: config.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				auto files = file_manager::get_files_in_path(
						dir_path_detail.first,
						dir_path_detail.second.recursive,
						name_source.second.suffix);
				if (files.empty()) {
					continue;
				}
				for (const auto& file: files) {
					do_resolve_and_post(
							file,
							dir_path_detail.first,
							dir_path_detail.second.filename_pattern,
							name_source.second.detail,
							dir_path_detail.second.type);
				}
			}
		}
	}

	void application::wake_up_watchdog() {
		for (const auto& name_source: config.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				// hook 文件关闭应该能够满足需求
				if (!watchdog.set_watch(dir_path_detail.first, dir_watchdog::IN_MOVED_TO)) {
					LOG2FILE(LOG_LEVEL::ERROR, "Cannot set watch to " + dir_path_detail.first);
				}
				if (!watchdog.set_callback(
							dir_path_detail.first,
							[&](uint32_t event_code, const std::string& filename) {
								do_resolve_and_post(
										filename,
										dir_path_detail.first,
										dir_path_detail.second.filename_pattern,
										name_source.second.detail,
										dir_path_detail.second.type);
							})) {
					LOG2FILE(LOG_LEVEL::ERROR, "Cannot set callback to " + dir_path_detail.first);
				}
			}
		}
	}

	void application::do_resolve_and_post(
			const std::string&					  filename,
			const std::string&					  dir_name,
			const std::string&					  filename_pattern,
			const data::data_source_field_detail& detail,
			const std::string&					  type) {
		boost::regex  pattern(filename_pattern);
		boost::smatch result;
		if (!boost::regex_match(filename, result, pattern)) {
			LOG2FILE(LOG_LEVEL::ERROR, "Invalid filename: " + filename);
			return;
		}
		auto time_str	 = result[1];

		auto target_time = get_target_full_time(time_str, dir_name);

		if (!target_time.first) {
			LOG2FILE(LOG_LEVEL::WARNING, "Already processed file: " + filename);
			return;
		}

		auto message = file_manager::load_file(
				detail,
				data::get_file_type(type),
				filename);

		nlohmann::json json;
		json[target_time.second] = message;
		nlohmann::json json_sum;
		json_sum[target_time.second] = data::get_sum_of_file_data_type(message);

		for (const auto& name_url: config.target) {
			if (name_url.second.sum) {
				work::net_manager::post_data_to_url(name_url.second.url, json_sum.dump());
			} else {
				work::net_manager::post_data_to_url(name_url.second.url, json.dump());
			}
			LOG2FILE(LOG_LEVEL::INFO, "Send data to " + name_url.second.url + " for file " + filename);
		}
	}

	std::pair<bool, std::string> application::get_target_full_time(const std::string& time_str, const std::string& from) const {
		auto time = stoull(time_str, nullptr);
		if (time_str.length() == 4) {
			// time = hour + min
			boost::regex  pattern(R"(\b(\d{8})\b)");
			boost::smatch result;
			// todo: regex_match cannot match
			if (!boost::regex_search(from, result, pattern)) {
				LOG2FILE(LOG_LEVEL::ERROR, "Invalid directory: " + from);
				return std::make_pair(false, "");
			}
			auto year_mon_time = stoull(result[1], nullptr);

			if (config.start_time.compare_time_year_mon(year_mon_time, time)) {
				return std::make_pair(true, result[1] + time_str);
			}
		} else if (time_str.length() == 8) {
			// time = mon + day + hour + min
			boost::regex  pattern(R"(\b(\d{4})\b)");
			boost::smatch result;
			// todo: regex_match cannot match
			if (!boost::regex_search(from, result, pattern)) {
				LOG2FILE(LOG_LEVEL::ERROR, "Invalid directory: " + from);
				return std::make_pair(false, "");
			}
			auto year_time = stoull(result[1], nullptr);

			if (config.start_time.compare_time_year(year_time, time)) {
				return std::make_pair(true, result[1] + time_str);
			}
		} else if (time_str.length() == 12) {
			// time = year + mon + day + hour + min
			if (config.start_time.compare_time(time)) {
				return std::make_pair(true, time_str);
			}
		}

		return std::make_pair(false, "");
	}
}// namespace work
