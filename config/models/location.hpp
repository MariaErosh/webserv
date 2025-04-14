#pragma once
#include <vector>

struct ServerLocation {
		std::string					path;
		std::string 				root;
		std::vector<std::string>	index;
		std::vector<std::string>	method;
		bool						autoIndex;
		std::string					cgiPath;
		std::string					redirect;
};


