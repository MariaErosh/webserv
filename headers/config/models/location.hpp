#pragma once

#include <vector>

namespace WS { namespace Config	{
struct ServerLocation
	{
		std::string					path;
		std::string 				root;
		std::vector<std::string>	index;
		std::vector<std::string>	method;
		//std::string					autoindex; //ЗАМЕНИТЬ НА BOOLEAN!!
		bool						autoIndex;
		std::string					cgi_path;
		std::string					redirect;
	};
}}
