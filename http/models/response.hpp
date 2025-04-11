#pragma once

#include "codes.hpp"
#include <stdint.h>
#include <string>



class Response {
	public:
		std::string   version;
		StatusCode    status_code;
		std::map<std::string, std::string>  headers;
		std::string body;
				
		Response() : version("HTTP/1.1") {}
		Response(StatusCode status) : version("HTTP/1.1"), status_code(status) {}
};


