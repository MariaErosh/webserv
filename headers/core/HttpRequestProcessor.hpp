#pragma once

#include "../http/http.hpp"
#include "../config/models/server.hpp"
#include "../config/models/location.hpp"
#include "../config/models/config.hpp"

#define CONNECTION_TERMINATE -1
#define CONNECTION_CONTINUE   0


namespace Core {
  class HttpRequestProcessor {
	private:
	HttpRequestProcessor() {}

	public:
		static std::string  handleClientInput(const std::string& rawPayload,
											const std::string& sourceIp,
											const std::string& sourcePort,
											const Config::Config& settings,
											int& sessionState);


	private:

		static const Config::ServerConfig*    locateServerBlock(const Http::Request& req,
																const std::string& ip,
																const std::string& port,
																const Config::Config& settings);

		static const Config::ServerLocation*  locateRoutePath(const Http::Request& req,
															const Config::ServerConfig& serverBlock);

		static std::string      craftHttpReply(const Http::Request& req,
												const Config::ServerConfig* server,
												const Config::ServerLocation* route);

		static std::string      craftErrorReply(Http::StatusCode errorCode,
														const Http::Request& req,
														const Config::ServerConfig* server);

		static std::string      produceFallbackPage(void);

		static std::string      redirectClient(const std::string& targetUrl);

		static std::string      executeCGIScript(const Http::Request& req,
												const Config::ServerConfig* server,
												const Config::ServerLocation* route);


	/// Methods
		static std::string      processGET(const std::string& resolvedPath,
												const Http::Request& req,
												const Config::ServerConfig* server,
												const Config::ServerLocation* route);

		static std::string      processPOST(const std::string& resolvedPath,
												const Http::Request& req,
												const Config::ServerConfig* server);

		static std::string      processDELETE(const std::string& resolvedPath,
													const Http::Request& req,
													const Config::ServerConfig* server);


	/// Index
		static std::string   resolveIndexRoute(const std::string& resolvedPath,
														const Http::Request& req,
														const Config::ServerLocation& route);

		static std::string   generateAutoIndexView(const std::string& resolvedPath,
													const Http::Request& req);


	/// Utils
		static std::string  mapToAbsolutePath(const Http::Request& req,
											const Config::ServerLocation* route);

		static bool         isRequestPermitted(const Http::Request& req,
											const Config::ServerLocation& route);

		static std::string  inferMimeType(const std::string& fileExt);

};
}
