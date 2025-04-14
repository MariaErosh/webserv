#pragma once

#include "codes.hpp"
#include <stdint.h>
#include <string>



class Response {
	public:
		std::string   protocolVersion;
		StatusCode    statusCode;
		std::map<std::string, std::string>  httpHeaders;
		std::string body;

		Response() : protocolVersion("HTTP/1.1") {}
		Response(StatusCode status) : protocolVersion("HTTP/1.1"), statusCode(status) {}
};


