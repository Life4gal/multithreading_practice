#ifndef ERROR_LOGGER_HPP
#define ERROR_LOGGER_HPP

#include <string>

enum class LOG_LEVEL {
	INFO,
	WARNING,
	ERROR
};

/**
 * @brief 日志
 */
#define LOG2FILE(level, what) \
	LogToFile(level, std::string{"FILE:\t"} + __FILE__ + "\tLINE:\t" + std::to_string(__LINE__) + "\tFUNCTION:\t" + __PRETTY_FUNCTION__, what)

/**
 * @brief 输出日志信息
 * @param level 日志级别
 * @param where 日志来源位置
 * @param what 详细信息
 */
void LogToFile(LOG_LEVEL level, const std::string& where, const std::string& what);

#endif//ERROR_LOGGER_HPP
