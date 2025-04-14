#pragma once
#include "../../http/http.hpp"
#include "./location.hpp"
#include <vector>
#include <string>

struct ServerConfig {
	std::vector<std::string>				serverName;
	std::string								ipAddress;
	std::string								port;
	std::string								max_body_size;
	std::map<StatusCode, std::string>		error_page;
	std::vector<ServerLocation>				location_list;
};

