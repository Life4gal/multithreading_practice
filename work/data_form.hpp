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
		struct basic_data {
			using value_type = int64_t;
			using data_layer = std::array<value_type, 8>;

			data_layer	   wins;
			data_layer	   imps;
			data_layer	   clks;
			data_layer	   cost;

			nlohmann::json pad_json;

			inline void	   increase(std::array<value_type, 8>::size_type index, value_type price, value_type count = 1) {
				   wins[index] += count;
				   imps[index] += count;
				   clks[index] += count;
				   cost[index] += price;
			}

			inline value_type get_wins_sum() const {
				return std::accumulate(wins.cbegin(), wins.cend(), static_cast<value_type>(0));
			}

			inline value_type get_imps_sum() const {
				return std::accumulate(imps.cbegin(), imps.cend(), static_cast<value_type>(0));
			}

			inline value_type get_clks_sum() const {
				return std::accumulate(clks.cbegin(), clks.cend(), static_cast<value_type>(0));
			}

			inline value_type get_cost_sum() const {
				return std::accumulate(cost.cbegin(), cost.cend(), static_cast<value_type>(0));
			}

			basic_data_sum get_sum() const;
		};

		struct basic_data_sum {
			using value_type = basic_data::value_type;

			value_type	   wins;
			value_type	   imps;
			value_type	   clks;
			value_type	   cost;

			nlohmann::json pad_json;
		};

		inline basic_data_sum basic_data::get_sum() const {
			return {
					get_wins_sum(),
					get_imps_sum(),
					get_clks_sum(),
					get_cost_sum(),
					pad_json};
		}

		// id <-> data
		using basic_data_with_id	 = std::unordered_map<std::string, basic_data>;
		// id <-> data
		using basic_data_sum_with_id = std::unordered_map<std::string, basic_data_sum>;

		struct data_with_type {
			std::string		   type;

			basic_data_with_id data;
		};

		struct data_sum_with_type {
			std::string			   type;

			basic_data_sum_with_id data;
		};

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

			if (!data.pad_json.empty()) {
				j.merge_patch(data.pad_json);
			}
		}

		inline void to_json(nlohmann::json& j, const data_with_type& data) {
			j[data.type] = data.data;
		}

		inline void to_json(nlohmann::json& j, const data_sum_with_type& data) {
			j[data.type] = data.data;
		}

		struct field_helper {
			enum class data_column_pos {
				CODE	   = 0,
				AD		   = 1,
				AD_GROUP   = 2,
				CAMPAIGN   = 3,
				ADVERTISER = 4,
				SPOT	   = 5,
				LAYER	   = 6,
				PRICE	   = 7
			};

			enum class need_data_pos {
				AD		   = 0,
				AD_GROUP   = 1,
				CAMPAIGN   = 2,
				ADVERTISER = 3,
				SPOT	   = 4
			};

			using size_type																= size_t;
			constexpr static size_type							 total_data_column_size = 8;
			constexpr static size_type							 total_need_data_size	= 5;

			size_type											 code;

			size_type											 ad;
			size_type											 ad_group;
			size_type											 campaign;
			size_type											 advertiser;
			size_type											 spot;

			size_type											 layer;
			size_type											 price;

			std::string											 pad_data;
			std::vector<std::string>							 need_pad_name;

			inline std::array<size_type, total_data_column_size> get_data_column_array() const {
				return {code, ad, ad_group, campaign, advertiser, spot, layer, price};
			}

			static inline std::array<data_with_type, total_need_data_size> get_needed_data_array() {
				std::array<data_with_type, total_need_data_size> data;
				data[get_index_in_necessary_array_by_pos(need_data_pos::AD)].type		  = ad_name;
				data[get_index_in_necessary_array_by_pos(need_data_pos::AD_GROUP)].type	  = ad_group_name;
				data[get_index_in_necessary_array_by_pos(need_data_pos::ADVERTISER)].type = advertiser_name;
				data[get_index_in_necessary_array_by_pos(need_data_pos::CAMPAIGN)].type	  = campaign_name;
				data[get_index_in_necessary_array_by_pos(need_data_pos::SPOT)].type		  = spot_name;

				return data;
			}

			static inline const char* get_name_by_pos(data_column_pos pos) {
				switch (pos) {
					case data_column_pos::CODE:
					case data_column_pos::LAYER:
					case data_column_pos::PRICE:
						return "";
					case data_column_pos::AD:
						return ad_name;
					case data_column_pos::AD_GROUP:
						return ad_group_name;
					case data_column_pos::CAMPAIGN:
						return campaign_name;
					case data_column_pos::ADVERTISER:
						return advertiser_name;
					case data_column_pos::SPOT:
						return spot_name;
				}
			}

			static inline std::array<size_type, total_data_column_size>::size_type get_index_in_data_column_array_by_pos(data_column_pos pos) {
				using ret_type = std::array<size_type, total_data_column_size>::size_type;
				switch (pos) {
					case data_column_pos::CODE:
						return static_cast<ret_type>(data_column_pos::CODE);
					case data_column_pos::AD:
						return static_cast<ret_type>(data_column_pos::AD);
					case data_column_pos::AD_GROUP:
						return static_cast<ret_type>(data_column_pos::AD_GROUP);
					case data_column_pos::CAMPAIGN:
						return static_cast<ret_type>(data_column_pos::CAMPAIGN);
					case data_column_pos::ADVERTISER:
						return static_cast<ret_type>(data_column_pos::ADVERTISER);
					case data_column_pos::SPOT:
						return static_cast<ret_type>(data_column_pos::SPOT);
					case data_column_pos::LAYER:
						return static_cast<ret_type>(data_column_pos::LAYER);
					case data_column_pos::PRICE:
						return static_cast<ret_type>(data_column_pos::PRICE);
				}
			}

			static inline std::array<size_type, total_need_data_size>::size_type get_index_in_necessary_array_by_pos(need_data_pos pos) {
				using ret_type = std::array<size_type, total_need_data_size>::size_type;
				switch (pos) {
					case need_data_pos::AD:
						return static_cast<ret_type>(need_data_pos::AD);
					case need_data_pos::AD_GROUP:
						return static_cast<ret_type>(need_data_pos::AD_GROUP);
					case need_data_pos::ADVERTISER:
						return static_cast<ret_type>(need_data_pos::ADVERTISER);
					case need_data_pos::CAMPAIGN:
						return static_cast<ret_type>(need_data_pos::CAMPAIGN);
					case need_data_pos::SPOT:
						return static_cast<ret_type>(need_data_pos::SPOT);
				}
			}
		};

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(field_helper, code, ad, ad_group, campaign, advertiser, spot, layer, price, pad_data, need_pad_name)
	}// namespace data
}// namespace work

namespace nlohmann {
	template<size_t N>
	struct adl_serializer<std::array<work::data::data_with_type, N>> {
		// 用于将 field_helper::get_needed_data_array 正确转为 json
		static void to_json(json& j, const std::array<work::data::data_with_type, N>& data) {
			for (const auto& d: data) {
				j[d.type] = d.data;
			}
		}
	};
}// namespace nlohmann

#endif//DATA_FORM_HPP
