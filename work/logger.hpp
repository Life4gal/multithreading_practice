#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <boost/thread/thread.hpp>
#include <functional>
#include <string>
#include <vector>

class LogLoader {
public:
	std::vector<std::string>						   operator()(const std::string& path, const std::string& suffix);
	std::function<std::vector<std::string>(int, char)> operator()(const std::string& filename);
};

class LogSender {
public:
	std::function<int(const std::string&, bool)> operator()(const std::string& url);
};

class LogManager {
public:
	template<typename Func, typename... Args>
	void push_function(Func&& func, Args&&... args) {
		thread_group.template create_thread(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
		//		thread_group.template create_thread(boost::bind(std::forward<Func>(func), std::forward<Args>(args)...));
	}

	~LogManager();

private:
	boost::thread_group thread_group;
};

#endif//LOGGER_HPP
