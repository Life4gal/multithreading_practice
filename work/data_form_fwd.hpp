#ifndef DATA_FORM_FWD_HPP
#define DATA_FORM_FWD_HPP

#include "json_fwd.hpp"

namespace work {
	namespace data {
		constexpr static const char* wins_name		 = "wins";
		constexpr static const char* imps_name		 = "imps";
		constexpr static const char* clks_name		 = "clks";
		constexpr static const char* cost_name		 = "cost";

		constexpr static const char* file_type_win	 = "win";
		constexpr static const char* file_type_imp	 = "imp";
		constexpr static const char* file_type_clk	 = "clk";

		enum class FILE_TYPE {
			WIN,
			IMP,
			CLK,
			UNKNOWN
		};

		/**
		 * @brief 根据文件的类型(字符串)获取对应的类型(枚举)
		 * @param type 文件的类型(字符串)
		 * @return 对应的类型(枚举)
		 */
		inline FILE_TYPE get_file_type(const std::string& type) {
			if (type == file_type_win) {
				return FILE_TYPE::WIN;
			} else if (type == file_type_imp) {
				return FILE_TYPE::IMP;
			} else if (type == file_type_clk) {
				return FILE_TYPE::CLK;
			}
			return FILE_TYPE::UNKNOWN;
		}

		struct start_time_detail;
		struct data_target;
		struct data_source_field_detail;
		struct data_source_path_detail;
		struct data_source;
		struct data_config_manager;

		struct basic_data;
		struct basic_data_sum;
		struct data_with_type;
		struct data_sum_with_type;

		void from_json(const nlohmann::json& j, start_time_detail& data);
		void to_json(nlohmann::json& j, const start_time_detail& data);
		void from_json(const nlohmann::json& j, data_target& data);
		void to_json(nlohmann::json& j, const data_target& data);
		void from_json(const nlohmann::json& j, data_source_field_detail& data);
		void to_json(nlohmann::json& j, const data_source_field_detail& data);
		void from_json(const nlohmann::json& j, data_source_path_detail& data);
		void to_json(nlohmann::json& j, const data_source_path_detail& data);
		void from_json(const nlohmann::json& j, data_source& data);
		void to_json(nlohmann::json& j, const data_source& data);
		void from_json(const nlohmann::json& j, data_config_manager& data);
		void to_json(nlohmann::json& j, const data_config_manager& data);

		void to_json(nlohmann::json& j, const basic_data& data);
		void to_json(nlohmann::json& j, const basic_data_sum& data);
		void to_json(nlohmann::json& j, const data_with_type& data);
		void to_json(nlohmann::json& j, const data_sum_with_type& data);

		/**
		 * @brief 从文件中解析出来的数据的集合
		 */
		using file_data_type	 = std::vector<data_with_type>;
		/**
		 * @brief 从文件中解析出来的数据的集合的求和版本
		 */
		using file_data_sum_type = std::vector<data_sum_with_type>;

		/**
		 * @brief 获取数据的求和版本
		 * @param data 要求和的数据
		 * @return 求和后的数据
		 */
		file_data_sum_type get_sum_of_file_data_type(const file_data_type& data);
	}// namespace data
}// namespace work

#endif//DATA_FORM_FWD_HPP
