#include "file_finder.hpp"

#include <boost/filesystem.hpp>

#include "error_logger.hpp"

namespace {
	template<typename Iterator>
	void do_find(std::vector<std::string> &out, const std::string &suffix, Iterator begin, Iterator end = {}) {
		bool need_extension = !suffix.empty();
		for (; begin != end; ++begin) {
			auto p = begin->path();

			if (need_extension) {
				if (p.has_extension() && p.extension().string() == suffix) {
					out.emplace_back(p.string());
				}
			} else {
				out.emplace_back(p.string());
			}
			log_to_file("Info", __PRETTY_FUNCTION__, "Found file: " + p.string());
		}
	}
}// namespace

std::vector<std::string> file_finder::get_files_in_path(const std::string &path, const std::string &suffix, bool recursive) {
	std::vector<std::string> ret{};

	if (!path.empty()) {
		namespace fs = boost::filesystem;

		fs::path dir_path{path};
		if (recursive) {
			do_find(ret, suffix, fs::recursive_directory_iterator{dir_path});
		} else {
			do_find(ret, suffix, fs::directory_iterator{dir_path});
		}
	} else {
		log_to_file("Error", __PRETTY_FUNCTION__, "Empty path");
	}

	return ret;
}
