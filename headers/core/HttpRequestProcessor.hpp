

#pragma once

#include "../http/http.hpp"
#include "../config/models/server.hpp"
#include "../config/models/location.hpp"
#include "../config/models/config.hpp"

#define CONNECTION_TERMINATE -1
#define CONNECTION_CONTINUE   0



  class HttpRequestProcessor {
	private:
	HttpRequestProcessor() {}

	public:
		static std::string  handleClientInput(const std::string& rawPayload,
											const std::string& sourceIp,
											const std::string& sourcePort,
											const /**/Config& settings,
											int& sessionState);


	private:

		static const /**/ServerConfig*    locateServerBlock(const Request& req,
																const std::string& ip,
																const std::string& port,
																const Config& settings);

		static const ServerLocation*  locateRoutePath(const Request& req,
															const ServerConfig& serverBlock);

		static std::string      craftHttpReply(const Request& req,
												const ServerConfig* server,
												const ServerLocation* route);

		static std::string      craftErrorReply(StatusCode errorCode,
														const Request& req,
														const ServerConfig* server);

		static std::string      produceFallbackPage(void);

		static std::string      redirectClient(const std::string& targetUrl);

		static std::string      executeCGIScript(const Request& req,
												const ServerConfig* server,
												const ServerLocation* route);


	/// Methods
		static std::string      processGET(const std::string& resolvedPath,
												const Request& req,
												const ServerConfig* server,
												const ServerLocation* route);

		static std::string      processPOST(const std::string& resolvedPath,
												const Request& req,
												const ServerConfig* server);

		static std::string      processDELETE(const std::string& resolvedPath,
													const Request& req,
													const ServerConfig* server);


	/// Index
		static std::string   resolveIndexRoute(const std::string& resolvedPath,
														const Request& req,
														const ServerLocation& route);

		static std::string   generateAutoIndexView(const std::string& resolvedPath,
													const Request& req);


	/// Utils
		static std::string  mapToAbsolutePath(const Request& req,
											const ServerLocation* route);

		static bool         isRequestPermitted(const Request& req,
											const ServerLocation& route);

		static std::string  inferMimeType(const std::string& fileExt);

};

