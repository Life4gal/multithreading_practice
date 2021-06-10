#include "application.hpp"

#include "error_logger.hpp"

namespace work {
	application::application(std::string config_path)
		: path(std::move(config_path)) {
	}

	bool application::init() {
		config = file_manager::load_config(path);
		init_dir_watchdog();
	}

	bool application::init_dir_watchdog() {
		for (const auto& kv: config.source) {
			for (const auto& p: kv.second.path) {
				if (!watchdog.add_path(p)) {
					return false;
				}
			}
		}
	}

	bool application::process_all_exist_file() {
		for(const auto& kv : config.source)
		{
//			auto exist_file = file_manager::get_files_in_path()
		}

	}
}// namespace work
