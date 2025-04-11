#include "../utils/logger.hpp"
#include "../cgi/cgi.hpp"
#include "../utils/string.hpp"

	Handler   Handler::instance_;

	bool	Handler::addHeaderToEnv(const Request &request,
									const std::string& header_name, 
									const std::string& env_param) {    
		if (request.headers.find(header_name) != request.headers.end()) {
			env_[env_param] = request.headers.at(header_name);
			return true;
		}
		return false;
	}
	
	void	Handler::initEnv(const std::string& script_path,
							const Request& request,
							const ServerConfig& server) {
		Logger::debug("Handler::initEnv");

		env_["AUTH_TYPE"] = "Basic";
		env_["GATEWAY_INTERFACE"] = "CGI/1.1";
		if (request.method == POST) {
			addHeaderToEnv(request, "Content-Lenght", "CONTENT_LENGHT");
			addHeaderToEnv(request, "Content-Type", "CONTENT_TYPE");
		}
		addHeaderToEnv(request, "Accept", "HTTP_ACCEPT");
		addHeaderToEnv(request, "Referer", "HTTP_REFERER");
		addHeaderToEnv(request, "User-Agent", "HTTP_USER_AGENT");
		addHeaderToEnv(request, "Accept-Encoding", "HTTP_ACCEPT_ENCODING");
		addHeaderToEnv(request, "Accept-Language", "HTTP_ACCEPT_LANGUAGE");
		addHeaderToEnv(request, "Host", "SERVER_NAME");
		env_["REQUEST_METHOD"] = Parser::methodToString(request.method);
		env_["REQUEST_URI"] = request.uri;
		env_["SCRIPT_NAME"] = script_path;
		env_["SCRIPT_FILENAME"] = script_path;
		//addHeaderToEnv(request, "Host", "SERVER_NAME");    
		env_["SERVER_PORT"] = server.port;
		env_["SERVER_PROTOCOL"] = request.version;
		env_["SERVER_SOFTWARE"] = "webserv/1.0";
	}

	std::string	Handler::exec(const std::string& script_path,
							const Request& request,
							const ServerConfig& server) {
		Logger::debug("Handler::exec");
		initEnv(script_path, request, server);
		/*try {
			std::string result = executeCgi(script_path, (request.method == POST) ? request.body : ""); 
			return result;
		} catch (Handler::GatewayTimeoutException& e) {
			return RequestHandler::createErrorResponse(GatewayTimeOut, request, server);
		}*/
		return executeCgi(script_path, (request.method == POST) ? request.body : "");
	}
	

	char	**Handler::getCharEnv()
	{
		char  **env = new char*[env_.size() + 1];
		int j = 0;

		std::map<std::string, std::string>::iterator it;
		for (it = env_.begin(); it != env_.end(); it++) {
			std::string data = it->first + "=" + it->second;

			env[j] = new char[data.size() + 1];
			env[j] = strcpy(env[j], data.c_str());
			int k = 0;

			while (env[j][k]) {
				if (env[j][k] == '-')
					env[j][k] = '_';
				k++;
			}
			j += 1;
		}
		env[j] = NULL;
		return env;
	}

	std::string	Handler::executeCgi(const std::string& scriptFile, const std::string& body) {    
		Logger::debug("Handler::executeCgi");  
		Logger::debug("Handler::executeCgi : tmp files");  
		FILE  *input = tmpfile();
		FILE  *output = tmpfile();
		int input_fd = fileno(input);
		int output_fd = fileno(output);
		Logger::debug("Handler::executeCgi : write to input");  
		write(input_fd, body.c_str(), body.size());
		lseek(input_fd, 0, SEEK_SET); // move the pointer to the beginning of the input_fd
		
		Logger::debug("Handler::executeCgi : fork");		
		char**		env = getCharEnv();
		std::string	result_body;
		pid_t		pid = fork();

		if (pid == -1)
			throw ErrorMemoryException();
		else if (pid == 0) {
			// Forked
			char * const * nll = NULL;
			dup2(input_fd, STDIN_FILENO);
			dup2(output_fd, STDOUT_FILENO);
			if (execve(scriptFile.c_str(), nll, env) < 0) {
				Logger::error("EXECVE " + scriptFile + " WAS FAILED");
				exit(-1);
			}
		}
		else {
			fclose(input);
			// Main
			char buffer[1024] = {0};
			Logger::debug("Handler::executeCgi : wait");

			int status;
			time_t start_time = time(NULL);
			while (true) {
				pid_t result = waitpid(pid, &status, WNOHANG);
				if (result == pid) 
					break;
				if (difftime(time(NULL), start_time) >= 5) {
					// Kill process after 10s
					Logger::error("CGI process timeout, terminating");
					kill(pid, SIGKILL);
					waitpid(pid, &status, 0);
					// Clean up
					fclose(output);
					close(input_fd);
					close(output_fd);
					for (size_t i = 0; env[i]; i++)
						delete[] env[i];
					delete[] env;

					throw GatewayTimeoutException();					
				}
				usleep(100000);

			}
			//waitpid(pid, NULL, 0);			
			Logger::debug("Handler::executeCgi : seek");  
			lseek(output_fd, 0, SEEK_SET);
			int ret = 1;
			Logger::debug("Handler::executeCgi : read");  
			while (ret > 0) {
				memset(buffer, 0 , 1024);
				ret = read(output_fd, buffer, 1023);
				result_body += buffer;
			}
		}

		Logger::debug("Handler::executeCgi : clean up");  

		// Clean up
		fclose(output);
		close(input_fd);
		close(output_fd);

		for (size_t i = 0; env[i]; i++)
			delete[] env[i];
		delete[] env;

		return result_body;
	}

	const char	*Handler::ErrorMemoryException::what() const throw() {
		return "Exception thrown: fork error";
	}

	const char	*Handler::GatewayTimeoutException::what() const throw() {
		return "Gateway Timeout";
	};

	Handler::Handler() {};
	Handler::Handler(const Handler& other) {(void)other;};
	Handler& Handler::operator=(const Handler& other) {(void)other; return *this;};
	Handler::~Handler() {};


