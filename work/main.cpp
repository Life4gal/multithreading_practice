#include "file_finder.hpp"
#include "thread_manager.hpp"
#include "file_seeker.hpp"
#include "url_sender.hpp"

int main() {
	auto		   files = file_finder::get_files_in_path("./logs", ".txt", false);

	thread_manager manager;
	for (const auto& file: files) {
		manager.push_function(
				[](const std::string& file) -> void {
					auto urls = file_seeker::get_column_in_file(file, 13, '\t');
					url_sender::send_urls(urls, "https://baidu.com", true);
				},
				file);
	}

	for(const auto& file : files)
	{
		auto map = file_seeker::count_columns_combination_in_file(file, {3, 1 ,4});

		for(const auto& kv : map)
		{
			std::cout << "Found " << kv.first << ' ' << kv.second << " times in file " << file << std::endl;
		}
	}

	return 0;
}
