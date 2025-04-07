#include "../../headers/utils/logger.hpp"
#include "../../headers/cgi/cgi.hpp"
#include "../../headers/utils/string.hpp"

namespace WS { namespace CGI
{
	Handler   Handler::instance_;

	bool	Handler::addHeaderToEnv(const Http::Request &request,
									const std::string& header_name, 
									const std::string& env_param) {    
		if (request.headers.find(header_name) != request.headers.end()) {
			env_[env_param] = request.headers.at(header_name);
			return true;
		}
		return false;
	}
	
	void	Handler::initEnv(const std::string& script_path,
							const Http::Request& request,
							const Config::ServerConfig& server) {
		Utils::Logger::debug("CGI::Handler::initEnv");

		/// Basic
		env_["AUTH_TYPE"] = "Basic";
		env_["GATEWAY_INTERFACE"] = "CGI/1.1";

		/// POST body
		if (request.method == Http::POST) {
			addHeaderToEnv(request, "Content-Lenght", "CONTENT_LENGHT");
			addHeaderToEnv(request, "Content-Type", "CONTENT_TYPE");
		}

		// HTTP
		addHeaderToEnv(request, "Accept", "HTTP_ACCEPT");
		addHeaderToEnv(request, "Referer", "HTTP_REFERER");
		addHeaderToEnv(request, "User-Agent", "HTTP_USER_AGENT");
		addHeaderToEnv(request, "Accept-Encoding", "HTTP_ACCEPT_ENCODING");
		addHeaderToEnv(request, "Accept-Language", "HTTP_ACCEPT_LANGUAGE");

		// Request
		env_["REQUEST_URI"] = request.uri;
		env_["REQUEST_METHOD"] = Http::Parser::methodToString(request.method);

		//  Script
		env_["SCRIPT_NAME"] = script_path;
		env_["SCRIPT_FILENAME"] = script_path;

		// Server
		addHeaderToEnv(request, "Host", "SERVER_NAME");    
		env_["SERVER_PORT"] = server.port;
		env_["SERVER_PROTOCOL"] = request.version;
		env_["SERVER_SOFTWARE"] = "webserv/1.0";
	}

	std::string	Handler::exec(const std::string& script_path,
							const Http::Request& request,
							const Config::ServerConfig& server) {
		Utils::Logger::debug("CGI::Handler::exec");
		initEnv(script_path, request, server);
		/*try {
			std::string result = executeCgi(script_path, (request.method == Http::POST) ? request.body : ""); 
			return result;
		} catch (Handler::GatewayTimeoutException& e) {
			return RequestHandler::createErrorResponse(Http::GatewayTimeOut, request, server);
		}*/
		return executeCgi(script_path, (request.method == Http::POST) ? request.body : "");
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
		Utils::Logger::debug("CGI::Handler::executeCgi");  

		// Init tmp files
		Utils::Logger::debug("CGI::Handler::executeCgi : tmp files");  
		FILE  *input = tmpfile();
		FILE  *output = tmpfile();
		int input_fd = fileno(input);
		int output_fd = fileno(output);

		// Write body to input fd
		Utils::Logger::debug("CGI::Handler::executeCgi : write to input");  
		write(input_fd, body.c_str(), body.size());
		lseek(input_fd, 0, SEEK_SET); // move the pointer to the beginning of the input_fd

		// Fork and exec
		Utils::Logger::debug("CGI::Handler::executeCgi : fork");  
		
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
				Utils::Logger::error("EXECVE " + scriptFile + " WAS FAILED");
				exit(-1);
			}
		}
		else {
			fclose(input);
			// Main
			char buffer[1024] = {0};
			Utils::Logger::debug("CGI::Handler::executeCgi : wait");

			int status;
			time_t start_time = time(NULL);
			while (true) {
				pid_t result = waitpid(pid, &status, WNOHANG);
				if (result == pid) 
					break;
				if (difftime(time(NULL), start_time) >= 5) {
					// Kill process after 10s
					Utils::Logger::error("CGI process timeout, terminating");
					kill(pid, SIGKILL);
					waitpid(pid, &status, 0);
					///
					// Clean up
					fclose(output);
					close(input_fd);
					close(output_fd);
					for (size_t i = 0; env[i]; i++)
						delete[] env[i];
					delete[] env;
					///
					throw GatewayTimeoutException();					
				}
				usleep(100000);

			}
			//waitpid(pid, NULL, 0);
			
			Utils::Logger::debug("CGI::Handler::executeCgi : seek");  
			lseek(output_fd, 0, SEEK_SET);
			int ret = 1;
			Utils::Logger::debug("CGI::Handler::executeCgi : read");  
			while (ret > 0) {
				memset(buffer, 0 , 1024);
				ret = read(output_fd, buffer, 1023);
				result_body += buffer;
			}
		}

		Utils::Logger::debug("CGI::Handler::executeCgi : clean up");  

		// Clean up
		fclose(output);
		close(input_fd);
		close(output_fd);

		for (size_t i = 0; env[i]; i++)
			delete[] env[i];
		delete[] env;

		// return
		return result_body;
	}

	// Exceptions
	const char	*Handler::ErrorMemoryException::what() const throw() {
		return "Exception thrown: fork error";
	}

	const char	*Handler::GatewayTimeoutException::what() const throw() {
		return "Gateway Timeout";
	};
}}
