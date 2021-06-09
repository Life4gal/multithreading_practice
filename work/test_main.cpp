#include "data_form.hpp"
#include "file_manager.hpp"
#include <iostream>

int main()
{
	using namespace work::data;
	using namespace work;

//	all_data data1;
//	data1.ad_data[1].data = 2;
//	data1.ad_data[2].data = 2;
//	data1.ad_group_data[1].data = 1;
//	data1.ad_group_data[2].data = 1;
//	data1.campaign_data[10].data = 10;
//	data1.advertiser_data[20].data = 20;
//	data1.spot_data[100].data = 20;
//	data1.spot_data[100].layer = {1, 2, 3};

	basic_data data2;
//	data2.id = "1";
	data2.wins = {1, 2, 3};
	data2.imps = {4, 5, 6};
	data2.clks = {7, 8, 9};
	data2.cost = {111, 222, 333, 444};

	data_with_type data3;
	data3.type = "ad";
	data3.data.emplace("1", data2);
	data3.data.emplace("2", data2);

	data_sum_with_type data4;
	data4.type = "ad_group";
	data4.data.emplace("2", data2.get_sum());
	data4.data.emplace("3", data2.get_sum());

	nlohmann::json j1{data3};
	nlohmann::json j2{data4};

	std::cout << j1.dump() << '\n' << j2.dump() << std::endl;

	auto config = file_manager::load_config("wins");
	nlohmann::json config_json{config};
	std::cout << config_json.dump() << std::endl;

	auto message = file_manager::load_file(config, "test");
	std::cout << message.dump() << std::endl;
}
