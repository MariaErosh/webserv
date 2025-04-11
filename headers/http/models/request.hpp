#pragma once

#include <string>
#include <map>
#include "methods.hpp"


	class Request {
		public:
			Method	method;
			std::string		uri;
			std::string		version;

			std::map<std::string, std::string>  headers;
			std::string body;
	};

