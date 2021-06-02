#include "file_finder.hpp"

#include <boost/filesystem.hpp>

#include "error_logger.hpp"

std::vector<std::string> file_finder::get_files_in_path(const std::string &path, const std::string &suffix, bool recursive) {
	std::vector<std::string> ret{};

	if (!path.empty()) {
		namespace fs = boost::filesystem;

		fs::path dir_path{path};
		bool	 need_extension = !suffix.empty();

		// todo: 去除冗余，C++11不支持lambda参数类型使用占位符
		if (recursive) {
			for (auto it = fs::recursive_directory_iterator{dir_path}; it != fs::recursive_directory_iterator{}; ++it) {
				auto p = it->path();

				if (need_extension) {
					if (p.has_extension() && p.extension().string() == suffix) {
						ret.emplace_back(p.string());
					}
				} else {
					ret.emplace_back(p.string());
				}
				log_to_file("Info", __PRETTY_FUNCTION__, "Found file: " + p.string());
			}
		} else {
			for (auto it = fs::directory_iterator{dir_path}; it != fs::directory_iterator{}; ++it) {
				auto p = it->path();

				if (need_extension) {
					if (p.has_extension() && p.extension().string() == suffix) {
						ret.emplace_back(p.string());
					}
				} else {
					ret.emplace_back(p.string());
				}
				log_to_file("Info", __PRETTY_FUNCTION__, "Found file: " + p.string());
			}
		}
	} else {
		log_to_file("Error", __PRETTY_FUNCTION__, "Empty path");
	}

	return ret;
}
