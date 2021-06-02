#ifndef ERROR_LOGGER_HPP
#define ERROR_LOGGER_HPP

#include <string>

/**
 * @brief 输出日志信息
 * @param level 日志级别
 * @param where 日志来源位置
 * @param what 详细信息
 */
void log_to_file(const std::string& level, const std::string& where, const std::string& what);

#endif//ERROR_LOGGER_HPP
