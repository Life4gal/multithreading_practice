#ifndef DATA_FORM_HPP
#define DATA_FORM_HPP

#include <array>
#include <cstdint>
#include <unordered_map>

#include "data_form_fwd.hpp"
#include "json.hpp"

namespace work {
	namespace data {
		struct start_time_detail {
			using time_type = unsigned long long;

			/**
			 * @brief 当前时间的年时间，例如1970 = 1970年
			 */
			uint16_t  year;
			/**
			 * @brief 当前时间的月时间，例如1231 = 12月31日
			 */
			uint16_t  month_day;
			/**
			 * @brief 当前时间的日时间，例如2359 = 23:59
			 */
			uint16_t  hour_minute;

			/**
			 * @brief 获取当前的完整时间
			 * @return 当前的完整时间
			 */
			time_type get_full_time() const {
				return static_cast<time_type>(year * 100000000) + static_cast<time_type>(month_day * 10000) + hour_minute;
			}

			/**
			 * @brief 与当前的完整时间进行比较
			 * @param your_year_mon_day_time 用户给定的完整时间
			 * @return 用户给定的时间是否比当前的完整时间更大？(在当前时间的未来)
			 */
			bool compare_time(time_type your_year_mon_day_time) const {
				return your_year_mon_day_time > get_full_time();
			}

			/**
			 * @brief 与当前的完整时间进行比较
			 * @param your_year_mon_time 用户给定的年和月时间
			 * @param your_day_time 用户给定的日时间
			 * @return 用户给定的时间是否比当前的完整时间更大？(在当前时间的未来)
			 */
			bool compare_time_year_mon(time_type your_year_mon_time, time_type your_day_time) const {
				time_type my_year_mon_time = year * 10000 + month_day;
				if (my_year_mon_time < your_year_mon_time) {
					return true;
				} else if (my_year_mon_time == your_year_mon_time) {
					return hour_minute <= your_day_time;
				}
				return false;
			}

			/**
			 * @brief 与当前的完整时间进行比较
			 * @param your_year_time 用户给定的年时间
			 * @param your_mon_day_time 用户给定的月和日时间
			 * @return 用户给定的时间是否比当前的完整时间更大？(在当前时间的未来)
			 */
			bool compare_time_year(time_type your_year_time, time_type your_mon_day_time) const {
				if (year < your_year_time) {
					return true;
				} else if (year == your_year_time) {
					return month_day * 10000 + hour_minute <= your_mon_day_time;
				}
				return false;
			}
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(start_time_detail, year, month_day, hour_minute)

		struct data_target {
			/**
			 * @brief 目标的url
			 */
			std::string						   url;
			/**
			 * @brief 是否求和
			 */
			bool							   sum;
			/**
			 * @brief 需要替换的字段名，被替换目标字段名<->替换后的字段名
			 */
			std::map<std::string, std::string> field_replace;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_target, url, sum, field_replace)

		struct data_source_code_detail {
			using size_type	 = size_t;
			using value_type = uint64_t;

			/**
			 * @brief 目标在哪一列
			 */
			size_type				column;
			/**
			 * @brief 排除值还是包含值
			 */
			bool					exclude;
			/**
			 * @brief 候选的值
			 */
			std::vector<value_type> values;

			/**
			 * @brief 当前的值是否是可接受的
			 * @param value 当前的值
			 * @return 是否可接受
			 */
			bool					accept(value_type value) const;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_source_code_detail, column, exclude, values);

		struct data_source_field_detail {
			using size_type	 = size_t;
			using value_type = uint64_t;

			/**
			 * @brief 用于判断当前行是否合法，code的名字<->code的详情
			 */
			std::unordered_map<std::string, data_source_code_detail> code;
			/**
			 * @brief 可选的字段名集合，字段名 <-> 所在的列
			 */
			std::unordered_map<std::string, size_type>				 field;
			/**
			 * @brief layer所在的列
			 */
			size_type												 layer;

			/**
			 * @brief 填充的数据，目标数据应该能够转为json，如果不能将不会填充
			 */
			std::string												 pad_data;
			/**
			 * @brief 那些字段要填充数据
			 */
			std::vector<std::string>								 pad_field_name;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_source_field_detail, code, field, layer, pad_data, pad_field_name)

		struct data_source_path_detail {
			/**
			 * @brief 获取文件时间的正则表达式
			 */
			std::string filename_pattern;
			/**
			 * @brief 文件的类型，支持的类型见`FILE_TYPE get_file_type(const std::string& type)`
			 */
			std::string type;
			/**
			 * @brief 是否递归搜索
			 * 注意，如果进行递归搜索，子文件夹中的文件依然要符合filename_pattern
			 * 且不会从该子文件夹名字获取任何信息
			 */
			bool		recursive;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_source_path_detail, filename_pattern, type, recursive)

		struct data_source {
			using data_source_path = std::unordered_map<std::string, data_source_path_detail>;

			/**
			 * @brief 路径的详细信息，父文件夹的绝对路径 <-> 里面的子文件的详细信息
			 */
			data_source_path		 path;
			/**
			 * @brief 子文件内容的解释详情(如何解析)
			 */
			data_source_field_detail detail;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_source, path, detail)

		using source_mapping = std::unordered_map<std::string, data_source>;
		using target_mapping = std::unordered_map<std::string, data_target>;

		struct data_config_manager {
			/**
			 * @brief 开始的时间
			 */
			start_time_detail start_time;
			/**
			 * @brief 源的集合，源的名字 <-> 源的信息
			 */
			target_mapping	  target;
			/**
			 * @brief 目标的集合，目标的名字 <-> 目标的信息
			 */
			source_mapping	  source;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(data_config_manager, start_time, target, source)

		struct basic_data {
			using value_type				 = data_source_field_detail::value_type;
			using size_type					 = data_source_field_detail::size_type;

			constexpr static size_type bound = 8;
			using data_layer				 = std::array<value_type, bound>;

			data_layer	   wins;
			data_layer	   imps;
			data_layer	   clks;
			data_layer	   cost;

			nlohmann::json pad_json;

			/**
			 * @brief 数据累计
			 * @param layer 目标所在层(下标)
			 * @param name 文件的类型，支持的类型见`FILE_TYPE get_file_type(const std::string& type)`
			 * @param price 增加的price
			 * @param count 增加的count
			 */
			void		   increase(size_type layer, FILE_TYPE name, value_type price = 0, value_type count = 1);

			/**
			 * @brief 转换为求和的的数据
			 * @return 求和的数据
			 */
						   operator basic_data_sum() const;//NOLINT 需要 implicit conversions
		};

		struct basic_data_sum {
			using value_type = basic_data::value_type;

			value_type wins;
			value_type imps;
			value_type clks;
			value_type cost;
		};

		using basic_data_with_id	 = std::unordered_map<std::string, basic_data>;
		using basic_data_sum_with_id = std::unordered_map<std::string, basic_data_sum>;

		struct data_with_type {
			/**
			 * @brief 这个数据所属类型(字段名)，来自data_source_field_detail的field
			 */
			std::string		   type;
			/**
			 * @brief 所储存的数据，数据的id <-> 数据的内容
			 */
			basic_data_with_id data;

			/**
			 * @brief 转换为求和的的数据
			 * @return 求和的数据
			 */
							   operator data_sum_with_type() const;//NOLINT 需要 implicit conversions
		};

		struct data_sum_with_type {
			/**
			 * @brief 这个数据所属类型(字段名)，来自data_source_field_detail的field
			 */
			std::string			   type;
			/**
			 * @brief 所储存的数据，数据的id <-> 数据的内容
			 */
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
