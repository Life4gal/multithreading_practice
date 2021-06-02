#include "file_finder.hpp"
#include "file_seeker.hpp"
#include "thread_manager.hpp"
#include "url_sender.hpp"

std::string vector_to_string(const std::vector<size_t>& vec) {
	std::string ret{'{'};
	for (auto i = vec.cbegin(); i != vec.cend(); ++i) {
		ret.append(std::to_string(*i));

		if (i != vec.cend() - 1) {
			ret.push_back(',');
		}
	}
	ret.push_back('}');
	return ret;
}

int main() {
	auto   files = file_finder::get_files_in_path("./logs", ".txt", false);

	//	thread_manager manager;
	//	for (const auto& file: files) {
	//		manager.push_function(
	//				[](const std::string& file) -> void {
	//					auto urls = file_seeker::get_column_in_file(file, 13, '\t');
	//					url_sender::send_urls(urls, "https://baidu.com", true);
	//				},
	//				file);
	//	}

	size_t i	 = 1;
	for (const auto& file: files) {
		std::map<std::string, size_t> map{};

		std::vector<size_t>			  combination{i, i - 1, i + 1};

		file_seeker::count_columns_combination_in_file(file, combination, map);

		for (const auto& kv: map) {
			std::cout << "Found `" << kv.first <<"` " << kv.second
					  << " time(s) in file `" << file << "` combined by "
					  << vector_to_string(combination) << std::endl;
		}
		++i;
	}

	return 0;
}
