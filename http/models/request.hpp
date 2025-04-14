#pragma once

#include <string>
#include <map>
#include "methods.hpp"


class Request {
	public:
		Method	method;
		std::string		resourcePath;
		std::string		protocolVersion;

		std::map<std::string, std::string>  httpHeaders;
		std::string body;
};

