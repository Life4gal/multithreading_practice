#ifndef DATA_FORM_FWD_HPP
#define DATA_FORM_FWD_HPP

#include "json_fwd.hpp"

namespace work {
	namespace data {
		constexpr static const char* wins_name		 = "wins";
		constexpr static const char* imps_name		 = "imps";
		constexpr static const char* clks_name		 = "clks";
		constexpr static const char* cost_name		 = "cost";

		constexpr static const char* file_type_win = "win";
		constexpr static const char* file_type_imp = "imp";
		constexpr static const char* file_type_clk = "clk";

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
		inline FILE_TYPE GetFileType(const std::string& type) {
			if (type == file_type_win) {
				return FILE_TYPE::WIN;
			} else if (type == file_type_imp) {
				return FILE_TYPE::IMP;
			} else if (type == file_type_clk) {
				return FILE_TYPE::CLK;
			}
			return FILE_TYPE::UNKNOWN;
		}

		struct StartTimeDetail;
		struct DataTarget;
		struct DataSourceFieldDetail;
		struct DataSourcePathDetail;
		struct DataSource;
		struct DataConfigManager;

		struct BasicData;
		struct BasicDataSum;
		struct DataWithType;
		struct DataSumWithType;

		void from_json(const nlohmann::json& j, StartTimeDetail& data);
		void to_json(nlohmann::json& j, const StartTimeDetail& data);
		void from_json(const nlohmann::json& j, DataTarget& data);
		void to_json(nlohmann::json& j, const DataTarget& data);
		void from_json(const nlohmann::json& j, DataSourceFieldDetail& data);
		void to_json(nlohmann::json& j, const DataSourceFieldDetail& data);
		void from_json(const nlohmann::json& j, DataSourcePathDetail& data);
		void to_json(nlohmann::json& j, const DataSourcePathDetail& data);
		void from_json(const nlohmann::json& j, DataSource& data);
		void to_json(nlohmann::json& j, const DataSource& data);
		void from_json(const nlohmann::json& j, DataConfigManager& data);
		void to_json(nlohmann::json& j, const DataConfigManager& data);

		void to_json(nlohmann::json& j, const BasicData& data);
		void to_json(nlohmann::json& j, const BasicDataSum& data);
		void to_json(nlohmann::json& j, const DataWithType& data);
		void to_json(nlohmann::json& j, const DataSumWithType& data);

		/**
		 * @brief 从文件中解析出来的数据的集合
		 */
		using FileDataType		= std::vector<DataWithType>;
		/**
		 * @brief 从文件中解析出来的数据的集合的求和版本
		 */
		using FileDataSumType = std::vector<DataSumWithType>;

		/**
		 * @brief 获取数据的求和版本
		 * @param data 要求和的数据
		 * @return 求和后的数据
		 */
		FileDataSumType GetSumOfFileDataType(const FileDataType& data);
	}// namespace data
}// namespace work

#endif//DATA_FORM_FWD_HPP
