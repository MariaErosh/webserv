#pragma once

#include <string>
#include <map>
#include "methods.hpp"

namespace WS { 
	namespace Http {
		// Http request
		class Request {
		public:
			// start-line :
			Http::Method	method;
			std::string		uri;
			std::string		version;

			std::map<std::string, std::string>  headers;
			std::string body;
		};
	}
}
