#pragma once

#include "../models/config.hpp"
#include "../../utils/string.hpp"
#include <iostream>
#include <fstream>
#include <exception>

namespace WS { namespace Config {
	class Parser {

	public:
		static void	parseFile(const char *filename, Config &out);
		static void	parseConfig(std::ifstream& data, Config &out);
		static void	parseServerConfig(std::ifstream& data, Config &out);
		static void	parseServerLocation(std::ifstream& data, ServerConfig &out, std::string path);
		static bool	checkIp(std::string &ip_addr);
		static int	toInt(std::string &data);		

		class	WrongIpAddress: public std::exception {
			virtual const char  *what() const throw();
		};

		class	FileNotFoundException: public std::exception {
			virtual const char  *what() const throw();
		};

		class	WrongConfigException: public std::exception {
			virtual const char  *what() const throw();
		};
	};
}}
