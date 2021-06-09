#ifndef DATA_FORM_FWD_HPP
#define DATA_FORM_FWD_HPP

#include "json_fwd.hpp"

namespace work {
	namespace data {
		constexpr static const char* ad_name		 = "ad";
		constexpr static const char* ad_group_name	 = "ad_group";
		constexpr static const char* campaign_name	 = "campaign";
		constexpr static const char* advertiser_name = "advertiser";
		constexpr static const char* spot_name		 = "spot";

		constexpr static const char* wins_name		 = "wins";
		constexpr static const char* imps_name		 = "imps";
		constexpr static const char* clks_name		 = "clks";
		constexpr static const char* cost_name		 = "cost";

		struct basic_data;
		struct basic_data_sum;
		struct data_with_type;
		struct data_sum_with_type;

		struct field_helper;

		void to_json(nlohmann::json& j, const basic_data& data);
		void to_json(nlohmann::json& j, const basic_data_sum& data);
		void to_json(nlohmann::json& j, const data_with_type& data);
		void to_json(nlohmann::json& j, const data_sum_with_type& data);

		void to_json(nlohmann::json& j, const field_helper& data);
		void from_json(const nlohmann::json& j, field_helper& data);
	}// namespace data
}// namespace work

#endif//DATA_FORM_FWD_HPP
