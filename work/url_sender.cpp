#include "url_sender.hpp"

#include <curl/curl.h>

#include "error_logger.hpp"

int url_sender::send_url(const std::string &url, const std::string &domain, bool decode) {
	if (url.empty()) {
		log_to_file("Error", __PRETTY_FUNCTION__, "Empty Url");
		return -1;
	}

	auto *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

	auto real_url = domain + url;

	if (decode) {
		char *encoded_url = curl_easy_unescape(curl, real_url.c_str(), static_cast<int>(real_url.size()), nullptr);
		curl_easy_setopt(curl, CURLOPT_URL, encoded_url);
		log_to_file("Info", __PRETTY_FUNCTION__, std::string{"Send url:"} + encoded_url);
		delete encoded_url;
	} else {
		curl_easy_setopt(curl, CURLOPT_URL, real_url.c_str());
		log_to_file("Info", __PRETTY_FUNCTION__, std::string{"Send url:"} + real_url);
	}

	return curl_easy_perform(curl);
}

std::vector<int> url_sender::send_urls(const std::vector<std::string> &urls, const std::string &domain, bool decode) {
	std::vector<int> ret;
	ret.reserve(urls.size());

	for (const auto &url: urls) {
		ret.push_back(send_url(url, domain, decode));
	}

	return ret;
}
