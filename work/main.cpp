#include <boost/bind.hpp>

#include "logger.hpp"

int main() {
	LogLoader  loader{};
	LogSender  sender{};
	LogManager manager{};

	auto	   files = loader("./logs", ".txt");

	for (const auto& file: files) {
		manager.push_function([/*&manager, */ &loader, &sender](const std::string& file) -> void {
			auto urls = loader(file)(13, '\t');

			for (const auto& url: urls) {
				//				manager.push_function([&sender](const std::string& url){
				std::cout << "Received response code: " << sender(url)("http://baidu.com", true);
				//				}, url);
			}
		},
							  file);
	}

	return 0;
}
