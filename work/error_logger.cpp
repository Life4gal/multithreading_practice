#include "error_logger.hpp"

#include <fstream>
#include <memory>
#include <mutex>

namespace {
	std::mutex										 g_log_mutex;
	std::once_flag								 g_log_once_flag;
	const char*										 g_log_filename = "logger_output.txt";
	std::shared_ptr<std::ofstream> g_log_output_ptr;
}// namespace

void LogToFile(LOG_LEVEL level, const std::string& where, const std::string& what) {
	std::lock_guard<std::mutex> lock(g_log_mutex);

	std::call_once(
			g_log_once_flag,
			[](std::shared_ptr<std::ofstream>& p) -> void {
				p = std::make_shared<std::ofstream>(g_log_filename, std::ios::app | std::ios::out);
			},
			std::ref(g_log_output_ptr));

	switch (level) {
		case LOG_LEVEL::INFO:
			g_log_output_ptr.operator*() << "INFO:\t";
			break;
		case LOG_LEVEL::WARNING:
			g_log_output_ptr.operator*() << "WARNING:\t";
			break;
		case LOG_LEVEL::ERROR:
			g_log_output_ptr.operator*() << "ERROR:\t";
			break;
		default:
			g_log_output_ptr.operator*() << "UNKNOWN:\t";
			break;
	}

	g_log_output_ptr.operator*() << where << "\t-->\t" << what << std::endl;
}
