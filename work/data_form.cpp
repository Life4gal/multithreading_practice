#include "data_form.hpp"

#include <boost/regex.hpp>

#include "error_logger.hpp"

namespace work {
	namespace data {
		bool StartTimeDetail::IsTimeValid() const {
			return year >= min_year_time && year <= max_year_time &&
						 month_day >= min_month_day_time && month_day <= max_month_day_time &&
						 hour_minute >= min_hour_minute_time && hour_minute <= max_hour_minute_time;
		}

		StartTimeDetail::time_type StartTimeDetail::GetFullTime() const {
			return static_cast<StartTimeDetail::time_type>(year * 100000000) + static_cast<time_type>(month_day * 10000) + hour_minute;
		}

		bool StartTimeDetail::CompareTime(time_type your_year_mon_day_time) const {
			return your_year_mon_day_time > GetFullTime();
		}

		bool StartTimeDetail::CompareTimeYearMon(time_type your_year_mon_time, time_type your_day_time) const {
			time_type my_year_mon_time = year * 10000 + month_day;
			if (my_year_mon_time < your_year_mon_time) {
				return true;
			} else if (my_year_mon_time == your_year_mon_time) {
				return hour_minute <= your_day_time;
			}
			return false;
		}

		bool StartTimeDetail::CompareTimeYear(time_type your_year_time, time_type your_mon_day_time) const {
			if (year < your_year_time) {
				return true;
			} else if (year == your_year_time) {
				return month_day * 10000 + hour_minute <= your_mon_day_time;
			}
			return false;
		}

		std::pair<bool, std::string> StartTimeDetail::GetTargetFullTime(const std::string& time_str, const std::string& folder_str) const {
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

				if (CompareTimeYearMon(year_mon_time, time)) {
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

				if (CompareTimeYear(year_time, time)) {
					return std::make_pair(true, result[1] + time_str);
				}
			} else if (time_str.length() == 12) {
				// time = year + mon + day + hour + min
				if (CompareTime(time)) {
					return std::make_pair(true, time_str);
				}
			}

			return std::make_pair(false, "");
		}

		bool DataSourceCodeDetail::Accept(value_type value) const {
			if (exclude) {
				return std::none_of(values.cbegin(), values.cend(), [&](value_type v) { return v == value; });
			} else {
				return std::any_of(values.cbegin(), values.cend(), [&](value_type v) { return v == value; });
			}
		}

		bool DataSourcePathDetail::IsFileValid(const std::string& filename) const {
			return boost::regex_search(filename, boost::regex{filename_pattern});
		}

		std::string DataSourcePathDetail::GetFileTimeStr(const std::string& filename) const {
			boost::smatch result;
			// 在获取文件时一定会先 IsFileValid 判断，所以必定有结果
			boost::regex_search(filename, result, boost::regex{filename_pattern});
			return result[1];
		}

		void BasicData::Increase(size_type layer, FILE_TYPE name, value_type price, value_type count) {
			if (layer > bound - 1) {
				LOG2FILE(LOG_LEVEL::ERROR, "Layer out of bound! current: " + std::to_string(layer));
				return;
			}

			switch (name) {
				case FILE_TYPE::WIN:
					wins[layer] += count;
					cost[layer] += price;
					break;
				case FILE_TYPE::IMP:
					imps[layer] += count;
					break;
				case FILE_TYPE::CLK:
					clks[layer] += count;
					break;
				case FILE_TYPE::UNKNOWN:
					break;
			}
		}

		BasicData::operator BasicDataSum() const {
			return {
					std::accumulate(wins.cbegin(), wins.cend(), static_cast<value_type>(0)),
					std::accumulate(imps.cbegin(), imps.cend(), static_cast<value_type>(0)),
					std::accumulate(clks.cbegin(), clks.cend(), static_cast<value_type>(0)),
					std::accumulate(cost.cbegin(), cost.cend(), static_cast<value_type>(0))};
		}

		DataWithType::operator DataSumWithType() const {
			DataSumWithType sum{type};
			for (const auto& kv: data) {
				sum.data.emplace(kv.first, kv.second);
			}
			return sum;
		}
	}// namespace data
}// namespace work
