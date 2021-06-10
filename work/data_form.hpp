#ifndef DATA_FORM_HPP
#define DATA_FORM_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include "data_form_fwd.hpp"
#include "json.hpp"

namespace work {
	namespace data {
		struct data_target {
			std::string url;
			bool		sum;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_target, url, sum)

		struct data_source_detail {
			using size_type										   = size_t;
			using value_type									   = uint64_t;
			constexpr static size_type				   ignore_code = -1;

			size_type								   code;
			// field_name <-> column
			std::unordered_map<std::string, size_type> field;
			size_type								   layer;
			size_type								   price;

			std::string								   pad_data;
			std::vector<std::string>				   pad_field_name;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_source_detail, code, field, layer, price, pad_data, pad_field_name)

		struct data_source {
			using data_source_path = std::vector<std::string>;

			data_source_path   path;
			std::string		   suffix;
			data_source_detail detail;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_source, path, suffix, detail)

		// source_name <-> data_source
		using source_mapping = std::unordered_map<std::string, data_source>;
		// target_name <-> data_target
		using target_mapping = std::unordered_map<std::string, data_target>;

		struct data_config_manager {
			uint64_t	   start_time;
			target_mapping target;
			source_mapping source;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_config_manager, start_time, target, source)

		struct basic_data {
			using value_type = data_source_detail::value_type;
			using size_type	 = data_source_detail::size_type;
			using data_layer = std::array<value_type, 8>;

			data_layer	   wins;
			data_layer	   imps;
			data_layer	   clks;
			data_layer	   cost;

			nlohmann::json pad_json;

			inline void	   increase(size_type layer, file_type name, value_type price = 0, value_type count = 1) {
				   switch (name) {
					   case file_type::WIN:
						   wins[layer] += count;
						   cost[layer] += price;
						   break;
					   case file_type::IMP:
						   imps[layer] += count;
						   break;
					   case file_type::CLK:
						   clks[layer] += count;
						   break;
				   }
			}

			inline operator basic_data_sum() const;//NOLINT 需要 implicit conversions
		};

		struct basic_data_sum {
			using value_type = basic_data::value_type;

			value_type wins;
			value_type imps;
			value_type clks;
			value_type cost;
		};

		basic_data::operator basic_data_sum() const {
			return {
					std::accumulate(wins.cbegin(), wins.cend(), static_cast<value_type>(0)),
					std::accumulate(imps.cbegin(), imps.cend(), static_cast<value_type>(0)),
					std::accumulate(clks.cbegin(), clks.cend(), static_cast<value_type>(0)),
					std::accumulate(cost.cbegin(), cost.cend(), static_cast<value_type>(0))};
		}

		// id <-> data
		using basic_data_with_id	   = std::unordered_map<std::string, basic_data>;
		// id <-> data
		using basic_data_sum_with_id   = std::unordered_map<std::string, basic_data_sum>;

		struct data_sum_with_type;
		struct data_with_type {
			std::string			 type;
			basic_data_with_id	 data;

			inline				 operator data_sum_with_type() const;//NOLINT 需要 implicit conversions
		};

		struct data_sum_with_type {
			std::string				 type;
			basic_data_sum_with_id	 data;
		};

		data_with_type::operator data_sum_with_type() const {
			data_sum_with_type sum{type};
			for (const auto& kv: data) {
				sum.data.emplace(kv.first, kv.second);
			}
			return sum;
		}

		inline void to_json(nlohmann::json& j, const basic_data& data) {
			j = {
					{wins_name, data.wins},
					{imps_name, data.imps},
					{clks_name, data.clks},
					{cost_name, data.cost}};

			if (!data.pad_json.empty()) {
				j.merge_patch(data.pad_json);
			}
		}

		inline void to_json(nlohmann::json& j, const basic_data_sum& data) {
			j = {
					{wins_name, data.wins},
					{imps_name, data.imps},
					{clks_name, data.clks},
					{cost_name, data.cost}};
		}

		inline void to_json(nlohmann::json& j, const data_with_type& data) {
			j[data.type] = data.data;
		}

		inline void to_json(nlohmann::json& j, const data_sum_with_type& data) {
			j[data.type] = data.data;
		}

		inline file_data_sum_type get_sum_of_file_data_type(const file_data_type& data) {
			file_data_sum_type ret;
			ret.reserve(data.size());
			for (const auto& d: data) {
				ret.emplace_back(d);
			}
			return ret;
		}
	}// namespace data
}// namespace work

namespace nlohmann {
	template<>
	struct adl_serializer<work::data::file_data_type> {
		// 保证 work::data::file_data_type 能被正确解析
		static void to_json(json& j, const work::data::file_data_type& data) {
			for (const auto& d: data) {
				j[d.type] = d.data;
			}
		}
	};

	template<>
	struct adl_serializer<work::data::file_data_sum_type> {
		// 保证 work::data::file_data_sum_type 能被正确解析
		static void to_json(json& j, const work::data::file_data_sum_type& data) {
			for (const auto& d: data) {
				j[d.type] = d.data;
			}
		}
	};
}// namespace nlohmann

#endif//DATA_FORM_HPP
