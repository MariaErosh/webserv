#pragma once

#include "../http/http.hpp"
#include "../config/models/config.hpp"
#include <string>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <new>
#include <map>
#include <stdio.h>
#include <fcntl.h>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
 
namespace CGI {
	class Handler {
		public:
			static Handler  instance_;

			//Execute cgi file as bash script
			std::string exec(const std::string& script_path,
								const Http::Request& request,
								const Config::ServerConfig& server);

			// Exception for case when config data is uncorrect*/
			class	ErrorMemoryException: public std::exception {
				virtual const char  *what() const throw();
			};

			class	GatewayTimeoutException: public std::exception {
				virtual const char  *what() const throw();
			};

		private:			
			std::map<std::string, std::string> env_;	//Map which contains ENVS for CGI

			//Init enviroment variables for next execve
			void	initEnv(const std::string& script_path,
									const Http::Request& request,
									const Config::ServerConfig& server);

			char	**getCharEnv(void); //Convert ENVS to usable char** array

			/* Check header and add in ENV if it exist
			Return True if header exists and added to ENV, false otherwise
			*/
			bool	addHeaderToEnv(const Http::Request &request,
										const std::string& header, 
										const std::string& env_param);

			// Execute CGI script
			std::string   executeCgi(const std::string& script, 
										const std::string& body);
			
			Handler();
			Handler(const Handler& other);
			~Handler();
			Handler &operator=(const Handler& other);
	};
}
