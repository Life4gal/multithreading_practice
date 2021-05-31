#include "logger.hpp"

#include <curl/curl.h>

#include <boost/filesystem.hpp>
#include <mutex>

namespace {
	std::mutex					   g_log_mutex;
	std::once_flag				   g_log_once_flag;
	const char *				   g_log_filename = "logger_output.txt";
	std::shared_ptr<std::ofstream> g_log_output_ptr;
}// namespace

template<typename T>
void log_to_file(const T &message) {
	std::lock_guard<std::mutex> lock(g_log_mutex);

	std::call_once(
			g_log_once_flag,
			[](std::shared_ptr<std::ofstream> &p) -> void {
				p = std::make_shared<std::ofstream>(g_log_filename, std::ios::app | std::ios::out);
			},
			std::ref(g_log_output_ptr));

	g_log_output_ptr.operator*() << message << std::endl;
	g_log_output_ptr->flush();
}

std::vector<std::string> LogLoader::operator()(const std::string &path, const std::string &suffix) {
	std::vector<std::string> ret{};

	if (!path.empty()) {
		namespace fs = boost::filesystem;

		fs::path						 dir_path{path};
		fs::recursive_directory_iterator end{};
		bool							 need_extension = !suffix.empty();

		for (fs::recursive_directory_iterator i{dir_path}; i != end; ++i) {
			auto p = i->path();

			if (need_extension) {
				if (p.has_extension() && p.extension().string() == suffix) {
					ret.emplace_back(p.string());
				}
			} else {
				ret.emplace_back(p.string());
			}
			log_to_file(std::string{"Info: "} + __PRETTY_FUNCTION__ + "::FindFile -> " + p.string());
		}
	} else {
		log_to_file(std::string{"Error: "} + __PRETTY_FUNCTION__ + " -> Empty path");
	}

	return ret;
}

std::function<std::vector<std::string>(int, char)> LogLoader::operator()(const std::string &filename) {
	if (filename.empty()) {
		log_to_file(std::string{"Error: "} + __PRETTY_FUNCTION__ + " -> Empty filename");
		return [](int, char) -> std::vector<std::string> {
			return {};
		};
	}

	std::shared_ptr<std::ifstream> file = std::make_shared<std::ifstream>(filename);
	if (!file->is_open()) {
		log_to_file(std::string{"Error: "} + __PRETTY_FUNCTION__ + "::Cannot open file -> " + filename);
		file.reset();
		return [](int, char) -> std::vector<std::string> {
			return {};
		};
	}

	// todo: require C++14
	auto func = __PRETTY_FUNCTION__;
	return [file, func](int index, char delimiter = '\t') -> std::vector<std::string> {
		std::vector<std::string> ret{};

		std::string				 entire_line;
		while (std::getline(file.operator*(), entire_line)) {
			// 长度应该至少有 （index + 1) * 2 - 1 这么多个 delimiter 那么长
			if (entire_line.length() < ((index + 1) * 2 - 1) * sizeof(delimiter)) {
				log_to_file(std::string{"Error: "} + func + "::<lambda>`RealFind`::Insufficient length -> " + entire_line);
				continue;
			}

			std::string::size_type it = 0;
			for (auto i = 0; i <= index; ++i) {
				// 加上 sizeof(delimiter)的偏移以跳过这个delimiter
				it = entire_line.find(delimiter, it) + sizeof(delimiter);
			}

			decltype(it) next = entire_line.find(delimiter, it);

			ret.push_back(entire_line.substr(it, next - it));
		}

		return ret;
	};
}

std::function<int(const std::string &, bool)> LogSender::operator()(const std::string &url) {
	if (url.empty()) {
		log_to_file(std::string{"Error: "} + __PRETTY_FUNCTION__ + " -> Empty Url");
		return [](const std::string &, bool) -> int {
			return -1;
		};
	}

	auto *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

	// todo: require C++14
	auto func = __PRETTY_FUNCTION__;
	return [&url, curl, func](const std::string &domain, bool decode) -> int {
		auto real_url = domain + url;

		if (decode) {
			char *encoded_url = curl_easy_unescape(curl, real_url.c_str(), static_cast<int>(real_url.size()), nullptr);
			curl_easy_setopt(curl, CURLOPT_URL, encoded_url);
			log_to_file(std::string{"Error: "} + func + "::<lambda>`Sender` -> " + encoded_url);
			delete encoded_url;
		} else {
			curl_easy_setopt(curl, CURLOPT_URL, real_url.c_str());
			log_to_file(std::string{"Error: "} + func + "::<lambda>`Sender` -> " + real_url);
		}

		return curl_easy_perform(curl);
	};
}

LogManager::~LogManager() {
	thread_group.join_all();
}
