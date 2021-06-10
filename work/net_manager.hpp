#ifndef NET_MANAGER_HPP
#define NET_MANAGER_HPP

#include <string>
#include <vector>

namespace work
{
	class net_manager {
	public:
		static void post_data_to_url(const std::string &url, const std::string &what_to_post, std::ostream &out);

		static void post_data_to_url(const std::string &url, const std::vector<std::string> &what_to_post, std::ostream &out);
	};
}


#endif//NET_MANAGER_HPP
