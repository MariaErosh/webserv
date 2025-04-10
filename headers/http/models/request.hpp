#pragma once

#include <string>
#include <map>
#include "methods.hpp"

namespace Http {
	class Request {
		public:
			Http::Method	method;
			std::string		uri;
			std::string		version;

			std::map<std::string, std::string>  headers;
			std::string body;
	};
}
