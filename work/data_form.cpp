#include "data_form.hpp"

#include "error_logger.hpp"

namespace work {
	namespace data {
		bool data_source_code_detail::accept(value_type value) const {
			if (exclude) {
				return std::none_of(values.cbegin(), values.cend(), [&](value_type v) { return v == value; });
			} else {
				return std::any_of(values.cbegin(), values.cend(), [&](value_type v) { return v == value; });
			}
		}

		void basic_data::increase(size_type layer, FILE_TYPE name, value_type price, value_type count) {
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

		basic_data::operator basic_data_sum() const {
			return {
					std::accumulate(wins.cbegin(), wins.cend(), static_cast<value_type>(0)),
					std::accumulate(imps.cbegin(), imps.cend(), static_cast<value_type>(0)),
					std::accumulate(clks.cbegin(), clks.cend(), static_cast<value_type>(0)),
					std::accumulate(cost.cbegin(), cost.cend(), static_cast<value_type>(0))};
		}

		data_with_type::operator data_sum_with_type() const {
			data_sum_with_type sum{type};
			for (const auto& kv: data) {
				sum.data.emplace(kv.first, kv.second);
			}
			return sum;
		}
	}// namespace data
}// namespace work
