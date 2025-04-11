#pragma once
#include "../../http/http.hpp"
#include "./location.hpp"
#include <vector>
#include <string>

struct ServerConfig {
	std::vector<std::string>				server_name;
	std::string								ip_addr;
	std::string								port;
	std::string								max_body_size;		
	std::map<StatusCode, std::string>	error_page;		
	std::vector<ServerLocation>				location_list;
};

