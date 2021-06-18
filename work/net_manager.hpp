#ifndef NET_MANAGER_HPP
#define NET_MANAGER_HPP

#include <string>
#include <vector>

namespace work {
	class net_manager {
	public:
		/**
		 * @brief 发送数据给目标url
		 * @param url 目标url
		 * @param what_to_post 发送的数据
		 */
		static void post_data_to_url(
				const std::string &url,
				const std::string &what_to_post);

		/**
		 * @brief 发送数据给目标url，并输出返回的数据
		 * @param url 目标url
		 * @param what_to_post 发送的数据
		 * @param out 用于输出的输出流
		 */
		static void post_data_to_url(
				const std::string &url,
				const std::string &what_to_post,
				std::ostream &	   out);
	};
}// namespace work

#endif//NET_MANAGER_HPP
