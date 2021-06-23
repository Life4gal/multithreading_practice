#include "application.hpp"

#include <boost/regex.hpp>
#include <regex>

#include "error_logger.hpp"
#include "net_manager.hpp"
#include "thread_manager.hpp"

namespace work {
	Application::Application(std::string config_path)
		: config_path_(std::move(config_path)) {
	}

	bool Application::Init() {
		config_manager_ = FileManager::LoadConfig(config_path_);
		if (!InitWatchdog()) {
			return false;
		}
		WakeUpWatchdog();
		return true;
	}

	void Application::Run() {
		ThreadManager thread;
		thread.PushFunction(&Application::ProcessAllExistFile, this);

		for (const auto& name_source: config_manager_.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				thread.PushFunction(&DirWatchdog::Run, &watchdog_, dir_path_detail.first);
			}
		}
	}

	bool Application::InitWatchdog() {
		if (config_manager_.source.empty() || config_manager_.target.empty()) {
			return false;
		}

		for (const auto& name_source: config_manager_.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				if (!watchdog_.AddPath(dir_path_detail.first)) {
					return false;
				}
			}
		}
		return true;
	}

	void Application::ProcessAllExistFile() {
		for (const auto& name_source: config_manager_.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				auto files = FileManager::GetFilesInPath(
						dir_path_detail.first,
						dir_path_detail.second.recursive);
				if (files.empty()) {
					continue;
				}
				for (const auto& file: files) {
					DoResolveAndPostData(
							file,
							dir_path_detail.first,
							dir_path_detail.second.filename_pattern,
							name_source.second.detail,
							dir_path_detail.second.type);
				}
			}
		}
	}

	void Application::WakeUpWatchdog() {
		for (const auto& name_source: config_manager_.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				// hook 文件关闭应该能够满足需求
				if (!watchdog_.SetWatch(dir_path_detail.first, DirWatchdog::IN_MOVED_TO)) {
					LOG2FILE(LOG_LEVEL::ERROR, "Cannot set watch to " + dir_path_detail.first);
				}
				if (!watchdog_.SetCallback(
								dir_path_detail.first,
								[&](uint32_t event_code, const std::string& filename) {
									DoResolveAndPostData(
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

	std::pair<std::string, data::FileDataType> Application::DoResolveData(
			const std::string&										filename,
			const std::string&										dir_name,
			const std::string&										filename_pattern,
			const data::DataSourceFieldDetail& detail,
			const std::string&										type) {
		auto					file = FileManager::GetFilenameInPath(filename);

		boost::regex	pattern(filename_pattern);
		boost::smatch result;
		if (!boost::regex_search(file, result, pattern)) {
			LOG2FILE(LOG_LEVEL::ERROR, "Invalid filename: " + filename);
			return {};
		}
		auto time_str		 = result[1];

		auto target_time = GetTargetFullTime(time_str, dir_name);

		if (!target_time.first) {
			LOG2FILE(LOG_LEVEL::WARNING, "Already processed file: " + filename);
			return {};
		}

		auto message = FileManager::LoadFile(
				detail,
				data::GetFileType(type),
				FileManager::GetAbsolutePath(filename, dir_name));

		if (message.empty()) {
			LOG2FILE(LOG_LEVEL::ERROR, "Cannot load anything from " + FileManager::GetAbsolutePath(filename, dir_name));
			return {};
		}

		return std::make_pair(target_time.second, message);
	}

	void Application::DoPostData(const std::string& time, const data::FileDataType& data) const {
		if (time.empty() || data.empty()) {
			LOG2FILE(LOG_LEVEL::ERROR, "Timestamp or data is empty, cannot post");
			return;
		}

		nlohmann::json json;
		json[time] = data;

		nlohmann::json json_sum;
		json_sum[time]					 = data::GetSumOfFileDataType(data);

		std::string json_str		 = json.dump();
		std::string json_sum_str = json_sum.dump();

		for (const auto& name_url: config_manager_.target) {
			std::string str_copy;
			if (name_url.second.sum) {
				str_copy = json_sum_str;
			} else {
				str_copy = json_str;
			}

			for (const auto& from_to: name_url.second.field_replace) {
				auto pos = str_copy.find(from_to.first);
				if (pos != std::string::npos) {
					str_copy.replace(pos, from_to.first.length(), from_to.second);
				}
			}

			if (name_url.second.sum) {
				//				work::net_manager::post_data_to_url(name_url.second.url, json_sum.dump());
			} else {
				//				work::net_manager::post_data_to_url(name_url.second.url, json.dump());
			}
		}
	}

	void Application::DoResolveAndPostData(const std::string& filename, const std::string& dir_name, const std::string& filename_pattern, const data::DataSourceFieldDetail& detail, const std::string& type) {
		auto time_data = DoResolveData(filename, dir_name, filename_pattern, detail, type);
		DoPostData(time_data.first, time_data.second);
	}

	std::pair<bool, std::string> Application::GetTargetFullTime(const std::string& time_str, const std::string& folder_str) const {
		auto time = stoull(time_str, nullptr);
		if (time_str.length() == 4) {
			// time = hour + min
			boost::regex	pattern(R"(\b(\d{8})\b)");
			boost::smatch result;
			if (!boost::regex_search(folder_str, result, pattern)) {
				LOG2FILE(LOG_LEVEL::ERROR, "Invalid directory: " + folder_str);
				return std::make_pair(false, "");
			}
			auto year_mon_time = stoull(result[1], nullptr);

			if (config_manager_.start_time.CompareTimeYearMon(year_mon_time, time)) {
				return std::make_pair(true, result[1] + time_str);
			}
		} else if (time_str.length() == 8) {
			// time = mon + day + hour + min
			boost::regex	pattern(R"(\b(\d{4})\b)");
			boost::smatch result;
			if (!boost::regex_search(folder_str, result, pattern)) {
				LOG2FILE(LOG_LEVEL::ERROR, "Invalid directory: " + folder_str);
				return std::make_pair(false, "");
			}
			auto year_time = stoull(result[1], nullptr);

			if (config_manager_.start_time.CompareTimeYear(year_time, time)) {
				return std::make_pair(true, result[1] + time_str);
			}
		} else if (time_str.length() == 12) {
			// time = year + mon + day + hour + min
			if (config_manager_.start_time.CompareTime(time)) {
				return std::make_pair(true, time_str);
			}
		}

		return std::make_pair(false, "");
	}
}// namespace work
