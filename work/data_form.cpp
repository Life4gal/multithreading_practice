#include "data_form.hpp"

#include "error_logger.hpp"

namespace work {
	namespace data {
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

		bool DataSourceCodeDetail::Accept(value_type value) const {
			if (exclude) {
				return std::none_of(values.cbegin(), values.cend(), [&](value_type v) { return v == value; });
			} else {
				return std::any_of(values.cbegin(), values.cend(), [&](value_type v) { return v == value; });
			}
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
