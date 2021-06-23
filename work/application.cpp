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
		// 载入配置文件
		config_manager_ = FileManager::LoadConfig(config_path_);
		// 初始化watchdog
		if (!InitWatchdog()) {
			return false;
		}

		// 唤醒watchdog
		WakeUpWatchdog();
		return true;
	}

	void Application::Run() {
		ThreadManager thread;
		// 处理已有文件的线程
		thread.PushFunction(&Application::ProcessAllExistFile, this);

		// 遍历源
		for (const auto& name_source: config_manager_.source) {
			// 遍历源所在的路径
			for (const auto& dir_path_detail: name_source.second.path) {
				// todo: 当前处理方式为对于每个文件夹都提供一个线程监控，但其实监控占用的资源很少，可以让一个线程监控多个文件夹
				thread.PushFunction(&DirWatchdog::Run, &watchdog_, dir_path_detail.first);
			}
		}
	}

	bool Application::InitWatchdog() {
		// 是否有源和目标
		if (config_manager_.source.empty() || config_manager_.target.empty()) {
			return false;
		}

		for (const auto& name_source: config_manager_.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				// 将源中的文件夹全部添加到watchdog的监控路径
				if (!watchdog_.AddPath(dir_path_detail.first)) {
					return false;
				}
			}
		}
		return true;
	}

	void Application::ProcessAllExistFile() {
		// 处理已有的文件
		for (const auto& name_source: config_manager_.source) {
			for (const auto& dir_path_detail: name_source.second.path) {
				// 对于时间不合法的路径直接跳过
				if (!dir_path_detail.second.start_time.IsTimeValid()) {
					continue;
				}

				// 获取目标路径(文件夹)中的所有符合条件的文件名
				auto files = FileManager::GetFilesInPath(
						// 路径(文件夹)
						dir_path_detail.first,
						// 是否递归
						dir_path_detail.second.recursive,
						// 辅助判断条件
						[&dir_path_detail](const std::string& filename) -> bool {
							// 文件名应该合法
							return dir_path_detail.second.IsFileValid(filename);
						});

				if (files.empty()) {
					continue;
				}

				// 解析并发送所有读取到的文件
				for (const auto& file: files) {
					DoResolveAndPostData(
							dir_path_detail.second,
							file,
							dir_path_detail.first,
							name_source.second.detail);
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
									if (!dir_path_detail.second.IsFileValid(filename)) {
										return;
									}
									DoResolveAndPostData(
											dir_path_detail.second,
											filename,
											dir_path_detail.first,
											name_source.second.detail);
								})) {
					LOG2FILE(LOG_LEVEL::ERROR, "Cannot set callback to " + dir_path_detail.first);
				}
			}
		}
	}

	std::pair<std::string, data::FileDataType> Application::DoResolveData(
			const data::DataSourcePathDetail&	 path_detail,
			const std::string&								 filename,
			const std::string&								 dir_name,
			const data::DataSourceFieldDetail& field_detail) {
		// 获取目标文件的包含时间的字符子串，保证是合法的时间串
		auto time_str		 = path_detail.GetFileTimeStr(filename);

		// 将文件名的时间子串与路径中的时间子串组合起来
		auto target_time = path_detail.start_time.GetTargetFullTime(time_str, dir_name);
		if (!target_time.first) {
			LOG2FILE(LOG_LEVEL::WARNING, "Already processed file: " + filename);
			return {};
		}

		// 载入目标文件的数据
		auto message = FileManager::LoadFile(
				field_detail,
				data::GetFileType(path_detail.type),
				FileManager::GetAbsolutePath(filename, dir_name));

		if (message.empty()) {
			LOG2FILE(LOG_LEVEL::ERROR, "Cannot load anything from " + FileManager::GetAbsolutePath(filename, dir_name));
			return {};
		}

		return std::make_pair(target_time.second, message);
	}

	void Application::DoPostData(const std::string& time, const data::FileDataType& data, const data::TargetMapping& target) {
		// 时间或者数据为空都直接跳过
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

		// 遍历目标，发送数据
		for (const auto& name_url: target) {
			std::string str_copy;
			if (name_url.second.sum) {
				str_copy = json_sum_str;
			} else {
				str_copy = json_str;
			}

			// 替换要替换的字段名
			// todo: 支持直接删除某个字段或者替换某个字段(在dump之前)
			//    require: extract && insert (c++17)(字段名是key)
			for (const auto& from_to: name_url.second.field_replace) {
				auto pos = str_copy.find(from_to.first);
				if (pos != std::string::npos) {
					str_copy.replace(pos, from_to.first.length(), from_to.second);
				}
			}

			// 发送数据
			if (name_url.second.sum) {
				//				work::net_manager::post_data_to_url(name_url.second.url, json_sum.dump());
			} else {
				//				work::net_manager::post_data_to_url(name_url.second.url, json.dump());
			}
		}
	}

	void Application::DoResolveAndPostData(
			const data::DataSourcePathDetail&	 path_detail,
			const std::string&								 filename,
			const std::string&								 dir_name,
			const data::DataSourceFieldDetail& field_detail) const {
		auto time_data = DoResolveData(path_detail, filename, dir_name, field_detail);
		DoPostData(time_data.first, time_data.second, config_manager_.target);
	}
}// namespace work
