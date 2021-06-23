#include <iostream>

#include "application.hpp"

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Config file path not given, usage: ./" << argv[0] << " config_path" << std::endl;
		return -1;
	}

	work::Application application(argv[1]);
	if (application.Init()) {
		std::cout << "Init application successful.\nRunning..." << std::endl;
		application.Run();
	} else {
		std::cerr << "Init application failed." << std::endl;
	}
}
