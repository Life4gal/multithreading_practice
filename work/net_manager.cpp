#include "net_manager.hpp"

#include <curl/curl.h>

#include <cstring>
#include <fstream>
#include <memory>

#include "error_logger.hpp"

namespace {
	struct response_data {
		char *					memory;
		size_t					size;

		constexpr static size_t reserved = 1;
		response_data()
			:// alloc 0 will return nullptr
			  memory(reinterpret_cast<char *>(malloc(reserved))),
			  size(0) {
		}
		~response_data() {
			free(memory);
		}
	};

	size_t receive_response_data(void *received_data, size_t size, size_t data_length, void *user_data) {
		size_t needed_size = size * data_length;
		auto * response	   = reinterpret_cast<response_data *>(user_data);

		response->memory   = reinterpret_cast<char *>(realloc(response->memory, response->size + needed_size + response_data::reserved));
		if (response->memory) {
			memcpy(response->memory + response->size, received_data, needed_size);
			response->size += needed_size;
			response->memory[response->size] = '\0';
		}
		return needed_size;
	}

	CURL *init_curl() {
		if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
			LOG2FILE(LOG_LEVEL::ERROR, "curl_global_init failed");
			return nullptr;
		}

		auto *curl = curl_easy_init();
		if (curl == nullptr) {
			LOG2FILE(LOG_LEVEL::ERROR, "curl_easy_init fail");
			return nullptr;
		}

		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);//允许重定向
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);//启用时会汇报所有的信息

		return curl;
	}

	curl_slist *init_header(CURL *curl) {
		curl_slist *header = nullptr;
		header			   = curl_slist_append(header, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
		header			   = curl_slist_append(header, "Accept:application/json, text/javascript, */*; q=0.01");
		header			   = curl_slist_append(header, "Accept-Language:zh-CN,zh;q=0.8");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

		curl_easy_setopt(curl, CURLOPT_HEADER, 0L);//启用时会将头文件的信息作为数据流输

		return header;
	}

	void destroy_curl(CURL *curl) {
		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}

	void destroy_header(curl_slist *header) {
		curl_slist_free_all(header);
	}

	std::pair<CURL *, curl_slist *> do_post(const std::string &url, const std::string &what_to_post) {
		CURL *		curl   = init_curl();
		curl_slist *header = init_header(curl);

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// POST配置项
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, what_to_post.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, what_to_post.size());

		return {curl, header};
	}
}// namespace

namespace work {
	void net_manager::post_data_to_url(const std::string &url, const std::string &what_to_post) {
		auto curl_header = do_post(url, what_to_post);

		// ignore return
		curl_easy_perform(curl_header.first);
		long response_code;
		// ignore return
		curl_easy_getinfo(curl_header.first, CURLINFO_RESPONSE_CODE, &response_code);

		destroy_header(curl_header.second);
		destroy_curl(curl_header.first);
	}

	void net_manager::post_data_to_url(const std::string &url, const std::string &what_to_post, std::ostream &out) {
		auto		  curl_header = do_post(url, what_to_post);

		//将返回结果通过回调函数写到自定义的对象中
		response_data response_data;
		curl_easy_setopt(curl_header.first, CURLOPT_WRITEDATA, &response_data);
		curl_easy_setopt(curl_header.first, CURLOPT_WRITEFUNCTION, receive_response_data);

		// ignore return
		curl_easy_perform(curl_header.first);
		long response_code;
		// ignore return
		curl_easy_getinfo(curl_header.first, CURLINFO_RESPONSE_CODE, &response_code);

		if (response_code == 200 || response_code == 201) {
			if (out.good()) {
				out.write(response_data.memory, (long)response_data.size);
			}
		}

		destroy_header(curl_header.second);
		destroy_curl(curl_header.first);
	}
}// namespace work