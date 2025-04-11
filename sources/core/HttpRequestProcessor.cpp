#include "../../headers/core/HttpRequestProcessor.hpp"
#include "../../headers/utils/logger.hpp"
#include "../../headers/utils/file.hpp"
#include "../../headers/utils/string.hpp"
#include "../../headers/utils/exceptions.hpp"
#include "../../headers/core/server.hpp"
#include "../../headers/core/pagegenerator.hpp"
#include "../../headers/cgi/cgi.hpp"
#include <algorithm>
#include <unistd.h>
#include <exception>

namespace Core {

std::string HttpRequestProcessor::handleClientInput(const std::string& rawPayload,
												const std::string& sourceIp,
												const std::string& sourcePort,
												const Config::Config& settings,
												int& sessionState) {
	Utils::Logger::debug("HttpRequestProcessor::handleClientInput");

	const Config::ServerLocation* routeMatch = NULL;
	const Config::ServerConfig* serverMatch = NULL;
	Http::Request incomingRequest;
	std::string outgoingResponse;

	try {
		incomingRequest = Http::Parser::deserializeRequest(rawPayload);

		if (incomingRequest.headers.find("Connection") != incomingRequest.headers.end()) {
			if (incomingRequest.headers.at("Connection") == "close")
				sessionState = CONNECTION_TERMINATE;
			else
				sessionState = CONNECTION_CONTINUE;
			}

		if (settings.server_list.empty()) {
			outgoingResponse = HttpRequestProcessor::produceFallbackPage();
		} else {
			serverMatch = HttpRequestProcessor::locateServerBlock(incomingRequest, sourceIp, sourcePort, settings);
			routeMatch = HttpRequestProcessor::locateRoutePath(incomingRequest, *serverMatch);
			if (!routeMatch)
				outgoingResponse = HttpRequestProcessor::craftErrorReply(Http::NotFound, incomingRequest, serverMatch);
			else
				outgoingResponse = HttpRequestProcessor::craftHttpReply(incomingRequest, serverMatch, routeMatch);
		}
	} catch (const std::invalid_argument& parseEx) {
		Utils::Logger::error(parseEx.what());
		outgoingResponse = HttpRequestProcessor::craftErrorReply(Http::BadRequest, incomingRequest, serverMatch);
	} catch (const std::exception& genericEx) {
		Utils::Logger::error(genericEx.what());
		outgoingResponse = HttpRequestProcessor::craftErrorReply(Http::InternalServerError, incomingRequest, serverMatch);
	}
	return outgoingResponse;
}

const Config::ServerConfig*	HttpRequestProcessor::locateServerBlock(const Http::Request& req,
																	const std::string& ip,
																	const std::string& port,
																	const Config::Config& settings) {
	Utils::Logger::debug("HttpRequestProcessor::locateServerBlock");
	const Config::ServerConfig* backupServer = NULL;

	for (size_t index = 0; index < settings.server_list.size(); ++index) {
		const Config::ServerConfig& currentServer = settings.server_list[index];
		if (currentServer.ip_addr == ip && currentServer.port == port) {
			const std::string& hostHeader = req.headers.at("Host");
			for (size_t alias = 0; alias < currentServer.server_name.size(); ++alias) {
				if (currentServer.server_name[alias] + ":" + currentServer.port == hostHeader)
					return &currentServer;
			}
			if (!backupServer)
				backupServer = &currentServer;
		}
	}
	return backupServer;
}


const Config::ServerLocation* HttpRequestProcessor::locateRoutePath(const Http::Request& req,
																	const Config::ServerConfig& serverBlock) {
	Utils::Logger::debug("HttpRequestProcessor::locateRoutePath");
	size_t matchedLength = 0;
	const Config::ServerLocation* bestRoute = NULL;

	for (size_t i = 0; i < serverBlock.location_list.size(); ++i) {
		const Config::ServerLocation& candidateRoute = serverBlock.location_list[i];

		if (req.uri == candidateRoute.path)
			return &candidateRoute;

		if (req.uri.find(candidateRoute.path) == 0 && candidateRoute.path.size() > matchedLength) {
			bestRoute = &candidateRoute;
			matchedLength = candidateRoute.path.size();
		}
	}
	return bestRoute;
}

std::string	HttpRequestProcessor::craftHttpReply(const Http::Request& req,
												const Config::ServerConfig* server,
												const Config::ServerLocation* route) {
	Utils::Logger::debug("HttpRequestProcessor::craftHttpReply");

	if (route) {
		if (!isRequestPermitted(req, *route))
			return HttpRequestProcessor::craftErrorReply(Http::MethodNotAllowed, req, server);
		if (!route->redirect.empty())
			return HttpRequestProcessor::redirectClient(route->redirect);
		if (!route->cgi_path.empty())
			return HttpRequestProcessor::executeCGIScript(req, server, route);
		}

		std::string result;
		std::string resourcePath = HttpRequestProcessor::mapToAbsolutePath(req, route);

		if (req.method == Http::GET)
			result = processGET(resourcePath, req, server, route);
		else if (req.method == Http::POST)
			result = processPOST(resourcePath, req, server);
		else if (req.method == Http::DELETE)
			result = processDELETE(resourcePath, req, server);

		return result;
}

std::string	HttpRequestProcessor::produceFallbackPage() {
	Utils::Logger::debug("HttpRequestProcessor::produceFallbackPage");
	Http::Response  defaultReply(Http::Ok);
	defaultReply.body = PageGenerator::generateDefaultPage();
	defaultReply.headers.insert(std::make_pair("Content-Type", "text/html"));
	return Http::Parser::serializeResponse(defaultReply);
}

std::string	HttpRequestProcessor::craftErrorReply(Http::StatusCode errorCode,
												const Http::Request& req,
												const Config::ServerConfig* server) {
	Utils::Logger::debug("HttpRequestProcessor::craftErrorReply");
	try {
		Utils::Logger::error(req.headers.at("Host") + req.uri + ": " + Http::Parser::statusToString(errorCode));
	}
	catch(...) {
		Utils::Logger::error("Unknown error while logging.");
	}

	Http::Response  errorReply(errorCode);

	if (server && server->error_page.find(errorCode) != server->error_page.end()) {
		errorReply.body = PageGenerator::generateErrorPage(
		server->error_page.at(errorCode).c_str());
	}
	else {
		errorReply.body = PageGenerator::generateErrorPage(errorReply.status_code);
	}

	return Http::Parser::serializeResponse(errorReply);
}

std::string	HttpRequestProcessor::redirectClient(const std::string& targetUrl) {
	Http::Response  redirectReply(Http::MovedPermanently);
	redirectReply.headers.insert(std::make_pair("Location", targetUrl));
	return Http::Parser::serializeResponse(redirectReply);
}

std::string	HttpRequestProcessor::executeCGIScript(const Http::Request& req,
													const Config::ServerConfig* server,
													const Config::ServerLocation* route) {
	if (req.body.size() > static_cast<size_t>(::atoi(server->max_body_size.c_str())))
		return HttpRequestProcessor::craftErrorReply(Http::PayloadTooLarge, req, server);

	std::string scriptPath;

	if (route->cgi_path[0] != '/')
		scriptPath = Utils::File::getCurrDir() + '/' + route->cgi_path;
	else
		scriptPath = route->cgi_path;

	Utils::Logger::debug("CGI Script Path:" + scriptPath);

	if (!Utils::File::pathExists(scriptPath.c_str()))
		return craftErrorReply(Http::InternalServerError, req, server);

	std::string scriptOutput;
	try {
		scriptOutput = CGI::Handler::instance_.exec(scriptPath, req, *server);
		Utils::Logger::info("{" + scriptOutput + "}");
	} catch (CGI::Handler::GatewayTimeoutException& timeoutErr) {
		return HttpRequestProcessor::craftErrorReply(Http::GatewayTimeOut, req, server);
	}

	Http::Response  finalResponse(Http::Ok);

	return Http::Parser::serializeResponse(finalResponse, false) + scriptOutput;
}

std::string	HttpRequestProcessor::processGET(const std::string& resolvedPath,
											const Http::Request& req,
											const Config::ServerConfig* server,
											const Config::ServerLocation* route) {
	Utils::Logger::debug("HttpRequestProcessor::processGET");
	Utils::Logger::debug("Resolved path : " + resolvedPath);
	Utils::Logger::debug("Route path : " + route->path);

	if (route && Utils::File::isDir(resolvedPath.c_str())) {
		//Utils::Logger::debug("RequestHandler::responseFromGet CASE 1");
		if (!route->index.empty())
			return resolveIndexRoute(resolvedPath, req, *route);
		else if (route->autoIndex)
			return generateAutoIndexView(resolvedPath, req);
	}

	//Utils::Logger::debug("RequestHandler::responseFromGet CASE 2");
	std::string fileContent;
	try {
		fileContent = Utils::File::readFile(resolvedPath.c_str());
	}
	catch(const Utils::Exceptions::FileDoesNotExist& e) {
		return craftErrorReply(Http::NotFound, req, server);
	} catch(...) {
		throw;
	}
	Http::Response fileResponse(Http::Ok);
	fileResponse.body = fileContent;
	fileResponse.headers.insert(std::make_pair("Content-Type",HttpRequestProcessor::inferMimeType(resolvedPath)));

	return Http::Parser::serializeResponse(fileResponse);
}

std::string	HttpRequestProcessor::processPOST(const std::string& resolvedPath,
														const Http::Request& req,
														const Config::ServerConfig* server)	{
	Utils::Logger::debug("HttpRequestProcessor::processPOST");

	if (req.body.size() > static_cast<size_t>(::atoi(server->max_body_size.c_str())))
		return HttpRequestProcessor::craftErrorReply(Http::PayloadTooLarge, req, server);

	if (Utils::File::isDir(resolvedPath.c_str()))
		return HttpRequestProcessor::craftErrorReply(Http::BadRequest, req, server);

	Http::Response  uploadReply;

	if (Utils::File::pathExists(resolvedPath.c_str())){
		Utils::File::writeFile(req.body, resolvedPath.c_str(), true);
		uploadReply.status_code = Http::NoContent;
	}
	else {
		Utils::File::writeFile(req.body, resolvedPath.c_str());
		uploadReply.status_code = Http::Created;
	}

	return Http::Parser::serializeResponse(uploadReply);
}


std::string	HttpRequestProcessor::processDELETE(const std::string& resolvedPath,
															const Http::Request& req,
															const Config::ServerConfig* server) {
	Utils::Logger::debug("HttpRequestProcessor::processDELETE");

	if (!Utils::File::pathExists(resolvedPath.c_str()))
		return HttpRequestProcessor::craftErrorReply(Http::NotFound, req, server);
	if (Utils::File::isDir(resolvedPath.c_str()))
		return HttpRequestProcessor::craftErrorReply(Http::Conflict, req, server);

	Http::Response deleteReply;

	std::string data = Utils::File::readFile(resolvedPath.c_str());
	::unlink(resolvedPath.c_str());

	deleteReply.status_code = Http::Ok;
	deleteReply.body = data;

	deleteReply.headers.insert(std::make_pair("Content-Type", HttpRequestProcessor::inferMimeType(resolvedPath)));

	return Http::Parser::serializeResponse(deleteReply);
}

std::string	HttpRequestProcessor::resolveIndexRoute(const std::string& resolvedPath,
													const Http::Request& req,
													const Config::ServerLocation& route) {
	Utils::Logger::debug("HttpRequestProcessor::resolveIndexRoute");

	size_t foundIdx = 0;
	for (; foundIdx < route.index.size(); ++foundIdx) {
		if (Utils::File::pathExists((resolvedPath + route.index[foundIdx]).c_str()))
			break;
	}

	if (foundIdx == route.index.size()) {
		if (route.autoIndex)
			return generateAutoIndexView(resolvedPath, req);
		throw Utils::Exceptions::FileDoesNotExist();
	}

	Http::Response indexReply(Http::Ok);
	indexReply.body = Utils::File::readFile((resolvedPath + route.index[foundIdx]).c_str());
	indexReply.headers.insert(std::make_pair("Content-Type", HttpRequestProcessor::inferMimeType(route.index[foundIdx])));

	return Http::Parser::serializeResponse(indexReply);
}


std::string	HttpRequestProcessor::generateAutoIndexView(const std::string& resolvedPath,
															const Http::Request& req) {
	Utils::Logger::debug("HttpRequestProcessor::generateAutoIndexView");
	Http::Response autoIndexReply(Http::Ok);
	autoIndexReply.body = PageGenerator::generateIndexPage(resolvedPath, req.uri);
	autoIndexReply.headers.insert(std::make_pair("Content-Type", "text/html"));

	return Http::Parser::serializeResponse(autoIndexReply);
}

std::string HttpRequestProcessor::mapToAbsolutePath(const Http::Request& req,
												const Config::ServerLocation* route) {
	Utils::Logger::debug("HttpRequestProcessor::mapToAbsolutePath");

	if (route && !route->root.empty()) {
		std::string rootPrefix;
		if (route->root[0] != '/')
		rootPrefix = Utils::File::getCurrDir() + "/" + route->root;
		rootPrefix = route->root;
		std::string relativeUri = req.uri;
		if (!Utils::File::pathExists((rootPrefix + req.uri).c_str()))
		relativeUri.erase(0, route->path.size());

		std::string completePath = rootPrefix + relativeUri;
		while (completePath.find("//") != std::string::npos) {
			completePath.erase(completePath.find("//"), 1);
		}
		Utils::Logger::debug("HttpRequestProcessor::mapToAbsolutePath = " + completePath);
		return completePath;
	}
	std::string	completePath2 = Utils::File::getCurrDir() + req.uri;
	Utils::Logger::debug("HttpRequestProcessor::mapToAbsolutePath = " + completePath2);
	return (completePath2);
}


bool	HttpRequestProcessor::isRequestPermitted(const Http::Request& req, const Config::ServerLocation& route) {
	Utils::Logger::debug("HttpRequestProcessor::isRequestPermitted");

	if (!route.method.size())
		return true;

	return (std::find(route.method.begin(), route.method.end(),
	Http::Parser::methodToString(req.method)) != route.method.end());
}


std::string HttpRequestProcessor::inferMimeType(const std::string& fileExt) {
	Utils::Logger::debug("HttpRequestProcessor::inferMimeType");

	size_t dotIndex = fileExt.rfind('.');
	if (dotIndex == std::string::npos) {
		return "text/plain";
	}
	std::string ext = Utils::String::toLower(fileExt.substr(dotIndex));

	if (ext == ".html") return "text/html";
	if (ext == ".css")  return "text/css";
	if (ext == ".js")   return "text/javascript";
	if (ext == ".gif")  return "image/gif";
	if (ext == ".jpg")  return "image/jpeg";
	if (ext == ".jpeg") return "image/jpeg";
	if (ext == ".png")  return "image/png";
	if (ext == ".json") return "application/json";
	if (ext == ".pdf")  return "application/pdf";

	return "text/plain";
}
}
