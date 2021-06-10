#include <fstream>
#include <iostream>

#include "data_form.hpp"
#include "dir_watchdog.hpp"
#include "file_manager.hpp"
#include "json.hpp"
#include "net_manager.hpp"

int main() {
	using namespace work::data;
	using namespace work;

	auto		   c = file_manager::load_config("config/wins.json");
	nlohmann::json j1{c};
	std::cout << j1.dump() << std::endl;
	auto		   m = file_manager::load_file(c.source["dsp"].detail, file_type::WIN, "test");
	nlohmann::json j2;
	nlohmann::json j3;
	j2["11111"] = m;
	j3["22222"] = get_sum_of_file_data_type(m);
	std::cout << j2.dump() << '\n'
			  << j3.dump() << std::endl;

	auto files = file_manager::get_files_in_path(".", true);
	for(const auto & p : files)
	{
		std::cout << p << '\t';
	}

	//	std::ofstream output{"post_output.txt"};
	//	net_manager::post_data_to_url("http://pacing.test.amnetapi.com/api/meta/commit", json1.dump(), output);
	//	net_manager::post_data_to_url("http://pacing.test.amnetapi.com/api/meta/commit", json2.dump(), output);

//	dir_watchdog watchdog(".");
//	watchdog.register_callback(dir_watchdog::IN_ALL_EVENTS,
//							   [](uint32_t event_code, std::string filename) {
//								   std::cout << "Event " << event_code << ": " << filename << std::endl;
//							   });
//
//	if (watchdog.prepared()) {
//		watchdog.run();
//	}
}
