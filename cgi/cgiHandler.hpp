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


	class CgiHandler {
		public:
			static CgiHandler  instance_;

			std::string runScript(const std::string& scriptPath,
								const Request& clientRequest,
								const ServerConfig& serverConfig);

			class	ErrorMemoryException: public std::exception {
				virtual const char  *what() const throw();
			};

			class	GatewayTimeoutException: public std::exception {
				virtual const char  *what() const throw();
			};

			class	InternalServerError: public std::exception {
				virtual const char  *what() const throw();
			};

		private:
			std::map<std::string, std::string> environment_;

			// Initialize environment variables for execve
			void	setupEnvironment(const std::string& scriptPath,
									const Request& clientRequest,
									const ServerConfig& serverConfig);

			// Convert environment variables to char** for execve
			char	**convertEnvironment(void);

			// Add HTTP header to environment variables
			bool	addHeaderToEnvironment(const Request& clienRequest,
										const std::string& headerName,
										const std::string& envVariable);

			// Execute CGI script
			std::string   executeScript(const std::string& scriptPath,
										const std::string& requestBody);

			CgiHandler();
			CgiHandler(const CgiHandler& other);
			~CgiHandler();
			CgiHandler &operator=(const CgiHandler& other);
	};

