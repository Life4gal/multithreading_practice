#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "data_form.hpp"
#include "dir_watchdog.hpp"
#include "file_manager.hpp"

namespace work {
	class application {
	public:
		explicit application(std::string config_path);

		bool init();

		void run();

	private:
		bool						 init_watchdog();
		bool						 process_all_exist_file();
		void						 wake_up_watchdog();

		void						 do_resolve_and_post(const std::string& filename, const std::string& dir_name, const std::string& filename_pattern, const data::data_source_field_detail& detail, const std::string& type);
		std::pair<bool, std::string> get_target_full_time(const std::string& time_str, const std::string& from) const;

		std::string					 path;
		data::data_config_manager	 config;
		dir_watchdog				 watchdog;
	};
}// namespace work


#endif//APPLICATION_HPP
