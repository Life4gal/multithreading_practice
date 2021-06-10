#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "data_form.hpp"
#include "dir_watchdog.hpp"
#include "file_manager.hpp"
#include "net_manager.hpp"
#include "thread_manager.hpp"

namespace work {
	class application {
	public:
		explicit application(std::string config_path);

		bool init();

	private:
		bool					  init_dir_watchdog();
		bool					  process_all_exist_file();

		std::string				  path;
		data::data_config_manager config;
		dir_watchdog			  watchdog;
		thread_manager			  thread;
		net_manager				  net;
	};
}// namespace work


#endif//APPLICATION_HPP
