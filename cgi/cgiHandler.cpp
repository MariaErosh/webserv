#include "../utils/logger.hpp"
#include "../cgi/cgiHandler.hpp"
#include "../utils/string.hpp"

	CgiHandler   CgiHandler::instance_;

	bool	CgiHandler::addHeaderToEnvironment(const Request& clientRequest,
									const std::string& headerName,
									const std::string& envVariable) {
		if (clientRequest.httpHeaders.find(headerName) != clientRequest.httpHeaders.end()) {
			environment_[envVariable] = clientRequest.httpHeaders.at(headerName);
			return true;
		}
		return false;
	}

	void	CgiHandler::setupEnvironment(const std::string& scriptPath,
							const Request& clientRequest,
							const ServerConfig& serverConfig) {
		Logger::debug("CgiHandler::setupEnvironment");

		environment_["AUTH_TYPE"] = "Basic";
		environment_["GATEWAY_INTERFACE"] = "CGI/1.1";
		if (clientRequest.method == POST) {
			addHeaderToEnvironment(clientRequest, "Content-Lenght", "CONTENT_LENGHT");
			addHeaderToEnvironment(clientRequest, "Content-Type", "CONTENT_TYPE");
		}
		addHeaderToEnvironment(clientRequest, "Accept", "HTTP_ACCEPT");
		addHeaderToEnvironment(clientRequest, "Referer", "HTTP_REFERER");
		addHeaderToEnvironment(clientRequest, "User-Agent", "HTTP_USER_AGENT");
		addHeaderToEnvironment(clientRequest, "Accept-Encoding", "HTTP_ACCEPT_ENCODING");
		addHeaderToEnvironment(clientRequest, "Accept-Language", "HTTP_ACCEPT_LANGUAGE");
		addHeaderToEnvironment(clientRequest, "Host", "SERVER_NAME");
		environment_["REQUEST_METHOD"] = Parser::mapMethodToString(clientRequest.method);
		environment_["REQUEST_URI"] = clientRequest.resourcePath;
		environment_["SCRIPT_NAME"] = scriptPath;
		environment_["SCRIPT_FILENAME"] = scriptPath;
		environment_["SERVER_PORT"] = serverConfig.port;
		environment_["SERVER_PROTOCOL"] = clientRequest.protocolVersion;
		environment_["SERVER_SOFTWARE"] = "webserv/1.0";
	}

	std::string	CgiHandler::runScript(const std::string& scriptPath,
							const Request& clientRequest,
							const ServerConfig& serverConfig) {
		Logger::debug("Handler::runScript");
		setupEnvironment(scriptPath, clientRequest, serverConfig);
		return executeScript(scriptPath, (clientRequest.method == POST) ? clientRequest.body : "");
	}


	char	**CgiHandler::convertEnvironment()
	{
		char  **env = new char*[environment_.size() + 1];
		int j = 0;

		std::map<std::string, std::string>::iterator it;
		for (it = environment_.begin(); it != environment_.end(); it++) {
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

	std::string	CgiHandler::executeScript(const std::string& scriptPath, const std::string& requestBody) {
		Logger::debug("CgiHandler::executeScript");
		Logger::debug("CgiHandler::executeScript : tmp files");
		FILE  *input = tmpfile();
		FILE  *output = tmpfile();
		int inputFd = fileno(input);
		int outputFd = fileno(output);
		Logger::debug("CgiHandler::executeScript : write to input");
		if (requestBody.size() == 0){
			return "";
		}
		int r = write(inputFd, requestBody.c_str(), requestBody.size());
		if (r<0) {
			Logger::error("WRITE ERROR");
			close(outputFd);
			throw InternalServerError();
		}
		lseek(inputFd, 0, SEEK_SET);

		Logger::debug("CgiHandler::executeScript : fork");
		char**		env = convertEnvironment();
		std::string	resultBody;
		pid_t		pid = fork();

		if (pid == -1)
			throw ErrorMemoryException();
		else if (pid == 0) {
			char * const * null = NULL;
			dup2(inputFd, STDIN_FILENO);
			dup2(outputFd, STDOUT_FILENO);
			if (execve(requestBody.c_str(), null, env) < 0) {
				Logger::error("EXECVE " + scriptPath + " WAS FAILED");
				exit(-1);
			}
		}
		else {
			fclose(input);
			char buffer[1024] = {0};
			Logger::debug("CgiHandler::executeScript : wait");

			int status;
			time_t startTime = time(NULL);
			while (true) {
				pid_t result = waitpid(pid, &status, WNOHANG);
				if (result == pid)
					break;
				if (difftime(time(NULL), startTime) >= 5) {
					Logger::error("CGI process timeout, terminating");
					kill(pid, SIGKILL);
					waitpid(pid, &status, 0);
					fclose(output);
					close(inputFd);
					close(outputFd);
					for (size_t i = 0; env[i]; i++)
						delete[] env[i];
					delete[] env;

					throw GatewayTimeoutException();
				}
				usleep(100000);

			}
			Logger::debug("CgiHandler::executeScript : seek");
			lseek(outputFd, 0, SEEK_SET);
			int r = 1;
			Logger::debug("CgiHandler::executeScript : read");
			while (r > 0) {
				memset(buffer, 0 , 1024);
				r = read(outputFd, buffer, 1023);
				if (r < 0){
					Logger::error("READ ERROR");
					close(inputFd);
					throw InternalServerError();
				}

				resultBody += buffer;
			}
		}

		Logger::debug("CgiHandler::executeScript : clean up");
		fclose(output);
		close(inputFd);
		close(outputFd);

		for (size_t i = 0; env[i]; i++)
			delete[] env[i];
		delete[] env;

		return resultBody;
	}

	const char	*CgiHandler::ErrorMemoryException::what() const throw() {
		return "Exception thrown: fork error";
	}

	const char	*CgiHandler::GatewayTimeoutException::what() const throw() {
		return "Gateway Timeout";
	};

	const char	*CgiHandler::InternalServerError::what() const throw() {
		return "Internal Server Error";
	};

	CgiHandler::CgiHandler() {};
	CgiHandler::CgiHandler(const CgiHandler& other) {(void)other;};
	CgiHandler& CgiHandler::operator=(const CgiHandler& other) {(void)other; return *this;};
	CgiHandler::~CgiHandler() {};


