#include "../../utils/string.hpp"
#include "./parserConfig.hpp"
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <cctype>


	void	ParserConfig::parseFile(const char *fileName, Config &out) {
		std::ifstream	configFile(fileName);
		std::string		data;

		if (!configFile.is_open())
			throw FileNotFoundException();

		parseConfig(configFile, out);
		configFile.close();
	}

	void	ParserConfig::parseConfig(std::ifstream& configFile, Config &out) {
		std::string     data;
		ServerConfig    new_server;
		std::vector<std::string>  result;

		while (getline(configFile, data)) {  
			if (data.empty())
				continue;	
			if (data == "server")
				parseServerConfig(configFile, out);
			else
				throw WrongConfigException();
		}
	}

	bool	isNumeric(const std::string& s) {
		if (s.empty())
			return false;
		for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
			if (!std::isdigit(static_cast<unsigned char>(*it)))
				return false;
		}
		return true;
	}

	void	ParserConfig::parseServerConfig(std::ifstream& configFile, Config &out) {
		std::vector<std::string> 	result;
		ServerConfig				newServer;
		std::string					data;    

		while(getline(configFile, data)) {
			result = String::splitStr(data);
			int len = (int)result.size();
			if (len ==0)
				continue;
			if (result[0] == "listen" && len == 2){
				size_t colon_pos = result[1].find(':');

				if (colon_pos != std::string::npos) {
					newServer.ip_addr = result[1].substr(0, colon_pos);
					newServer.port = result[1].substr(colon_pos + 1);
				} 
				else {
					if (isNumeric(result[1]))
						newServer.port = result[1];
					else
						newServer.ip_addr = result[1];
				}
			}
			else if (result[0] == "server_name" && len > 1) {
				for (int i = 1; i < len; i++) 
				newServer.server_name.push_back(result[i]);
			}
			else if (result[0] == "max_body_size" && len == 2)
				newServer.max_body_size = result[1];
			else if (result[0] == "error_page") {
				const std::string& error_page_uri = result.back();

				for (size_t i = 1; i < result.size() - 1; ++i)
					newServer.error_page.insert(std::make_pair(
						static_cast<StatusCode>(atoi(result[i].c_str())), error_page_uri));
			}
			else if (result[0] == "location" && len == 2) {
				parseServerLocation(configFile, newServer, result[1]);
				break;
			}
			else {
				throw WrongConfigException();
			}
		}
		if (newServer.ip_addr == "" || newServer.ip_addr == "localhost")
			newServer.ip_addr = "127.0.0.1";
		if (newServer.port == "")
			newServer.port = "8080";
		if (checkIp(newServer.ip_addr))
			out.server_list.push_back(newServer);
		else
			throw WrongIpAddress();
	}

	void	ParserConfig::parseServerLocation(std::ifstream& conffile, ServerConfig &out, std::string path)
	{
		std::vector<std::string>	result;
		ServerLocation				newLocation;
		std::string					data;		

		if (path[0] != '/')
			throw WrongConfigException();
		newLocation.path = path;
		
		while(!conffile.eof()) {
			getline(conffile, data);
			if (data.size() == 0)
				break;
			result = String::splitStr(data);
			int len = (int)result.size();
			if (result[0] == "method" && len > 1) {
				for (int i = 1; i < len; i++)
					newLocation.method.push_back(result[i]);
			}			
			else if (result[0] == "index" && len == 2)
				newLocation.index.push_back(result[1]);
			else if (result[0] == "root" && len == 2) {
				if (result[1][result[1].size() - 1] != '/')
					result[1].push_back('/');
				newLocation.root = result[1];
			}
			else if (result[0] == "autoindex" && len == 2) {
				result[1] = String::toLower(result[1]);
				if (result[1] == "on")
					newLocation.autoIndex = true;
				else if (result[1] == "off")
					newLocation.autoIndex = false;
				else
					throw WrongConfigException();
			}
			else if (result[0] == "redirect" && len == 2)
				newLocation.redirect = result[1];
			else if (result[0] == "cgi_path" && len == 2)
				newLocation.cgi_path = result[1];			
			else if (result[0] == "location" && len == 2){
				out.location_list.push_back(newLocation);
				newLocation.path = result[1];
				newLocation.root = "";
				newLocation.cgi_path = "";
				newLocation.redirect = "";
				newLocation.autoIndex = false; //bad practice to show internal directory => by default - false
				newLocation.method.clear();
			}
			else if (len == 0)
				break;
			else
				throw WrongConfigException();
		}
		out.location_list.push_back(newLocation);
	}


	bool	ParserConfig::checkIp(std::string &ip_addr) {
		std::vector<std::string> 	data;

		int flag = 0;
		data = String::split(ip_addr, '.');
		for (size_t i = 0; i < data.size(); i++) {
			if (toInt(data[i]) == 255)
				flag += 1;
		}
		if (flag == 4)
			return false;
		return true;
	}

	int	ParserConfig::toInt(std::string &data) {
		std::stringstream degree(data);

		int res = 0;
		degree >> res;
		return res;
	}

	const char	 *ParserConfig::WrongIpAddress::what() const throw() {
		return "Exception: wrong ip address";
	}

	const char	*ParserConfig::WrongConfigException::what() const throw() {
		return "Exception: wrong configuration file";
	}

	const char	*ParserConfig::FileNotFoundException::what() const throw() {
		return "Exception: could not open configuration file";
	} 

