#ifndef URL_SENDER_HPP
#define URL_SENDER_HPP

#include <vector>
#include <string>

class url_sender
{
public:
	/**
	 * @brief 往所给域名发送和 url 组合的请求
	 * @param url 不算域名的 url
	 * @param domain 域名
	 * @param decode 是否解码
	 * @return 响应代码
	 */
	static int send_url(const std::string& url, const std::string& domain, bool decode);

	/**
	 * @brief 往所给域名发送和 url 组合的请求
	 * @param urls url集合
	 * @param domain 域名
	 * @param decode 是否解码
	 * @return 响应代码集合
	 */
	static std::vector<int> send_urls(const std::vector<std::string>& urls, const std::string& domain, bool decode);
};

#endif//URL_SENDER_HPP
