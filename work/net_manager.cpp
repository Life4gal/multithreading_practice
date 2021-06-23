#include "net_manager.hpp"

#include <curl/curl.h>

#include <cstring>
#include <fstream>

#include "error_logger.hpp"

namespace {
	struct ResponseData {
		char *									memory_;
		size_t									size_;

		constexpr static size_t reserved = 1;
		ResponseData()
			:// alloc 0 will return nullptr
				memory_(reinterpret_cast<char *>(malloc(reserved))),
				size_(0) {
		}
		~ResponseData() {
			free(memory_);
		}
	};

	/**
	 * @brief 接收返回的数据
	 * @param received_data 接受的数据
	 * @param size 数据尺寸，一般都为1
	 * @param data_length 数据的长度
	 * @param user_data 已有的用户数据
	 * @return 当前的用户数据长度
	 */
	size_t ReceiveResponseData(void *received_data, size_t size, size_t data_length, void *user_data) {
		size_t needed_size = size * data_length;
		auto * response		 = reinterpret_cast<ResponseData *>(user_data);

		response->memory_	 = reinterpret_cast<char *>(realloc(response->memory_, response->size_ + needed_size + ResponseData::reserved));
		if (response->memory_) {
			memcpy(response->memory_ + response->size_, received_data, needed_size);
			response->size_ += needed_size;
			response->memory_[response->size_] = '\0';
		}
		return needed_size;
	}

	/**
	 * @brief 初始化 curl
	 * @return 初始化完的curl指针
	 */
	CURL *InitCurl() {
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

	/**
	 * @brief 填充请求头
	 * @param curl 填充的curl
	 * @return 填充之后的请求头
	 */
	curl_slist *InitHeader(CURL *curl) {
		curl_slist *header = nullptr;
		header						 = curl_slist_append(header, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
		header						 = curl_slist_append(header, "Accept:application/json, text/javascript, */*; q=0.01");
		header						 = curl_slist_append(header, "Accept-Language:zh-CN,zh;q=0.8");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

		curl_easy_setopt(curl, CURLOPT_HEADER, 0L);//启用时会将头文件的信息作为数据流输

		return header;
	}

	/**
	 * @brief 销毁curl
	 * @param curl 要销毁的curl
	 */
	void DestroyCurl(CURL *curl) {
		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}

	/**
	 * @brief 销毁请求头
	 * @param header 要销毁的请求头
	 */
	void DestroyHeader(curl_slist *header) {
		curl_slist_free_all(header);
	}

	/**
	 * @brief 实际进行post行为
	 * @param url 目标url
	 * @param what_to_post post的数据
	 * @return 用于post的curl以及请求头
	 */
	std::pair<CURL *, curl_slist *> DoPost(const std::string &url, const std::string &what_to_post) {
		CURL *			curl	 = InitCurl();
		curl_slist *header = InitHeader(curl);

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// POST配置项
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, what_to_post.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, what_to_post.size());

		return {curl, header};
	}
}// namespace

namespace work {
	void NetManager::PostDataToUrl(const std::string &url, const std::string &what_to_post) {
		auto curl_header = DoPost(url, what_to_post);

		// ignore return
		curl_easy_perform(curl_header.first);
		long response_code;
		// ignore return
		curl_easy_getinfo(curl_header.first, CURLINFO_RESPONSE_CODE, &response_code);

		DestroyHeader(curl_header.second);
		DestroyCurl(curl_header.first);
	}

	void NetManager::PostDataToUrl(const std::string &url, const std::string &what_to_post, std::ostream &out) {
		auto				 curl_header = DoPost(url, what_to_post);

		//将返回结果通过回调函数写到自定义的对象中
		ResponseData response_data;
		curl_easy_setopt(curl_header.first, CURLOPT_WRITEDATA, &response_data);
		curl_easy_setopt(curl_header.first, CURLOPT_WRITEFUNCTION, ReceiveResponseData);

		// ignore return
		curl_easy_perform(curl_header.first);
		long response_code;
		// ignore return
		curl_easy_getinfo(curl_header.first, CURLINFO_RESPONSE_CODE, &response_code);

		if (response_code == 200 || response_code == 201) {
			if (out.good()) {
				out.write(response_data.memory_, (long)response_data.size_);
			}
		}

		DestroyHeader(curl_header.second);
		DestroyCurl(curl_header.first);
	}
}// namespace work