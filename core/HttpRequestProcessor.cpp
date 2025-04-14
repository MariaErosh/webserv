#include "HttpRequestProcessor.hpp"
#include "../utils/logger.hpp"
#include "../utils/file.hpp"
#include "../utils/string.hpp"
#include "../utils/exceptions.hpp"
#include "../core/server.hpp"
#include "./pageRenderer.hpp"
#include "../cgi/cgiHandler.hpp"
#include <algorithm>
#include <unistd.h>
#include <exception>

std::string HttpRequestProcessor::handleClientInput(const std::string& rawPayload,
												const std::string& sourceIp,
												const std::string& sourcePort,
												const Config& settings,
												int& sessionState) {
	Logger::debug("HttpRequestProcessor::handleClientInput");

	const ServerLocation* routeMatch = NULL;
	const ServerConfig* serverMatch = NULL;
	Request incomingRequest;
	std::string outgoingResponse;

	try {
		incomingRequest = Parser::parseHttpRequest(rawPayload);

		if (incomingRequest.httpHeaders.find("Connection") != incomingRequest.httpHeaders.end()) {
			if (incomingRequest.httpHeaders.at("Connection") == "close")
				sessionState = CONNECTION_TERMINATE;
			else
				sessionState = CONNECTION_CONTINUE;
			}

		if (settings.serverConfigurations.empty()) {
			outgoingResponse = HttpRequestProcessor::produceFallbackPage();
		} else {
			serverMatch = HttpRequestProcessor::locateServerBlock(incomingRequest, sourceIp, sourcePort, settings);
			routeMatch = HttpRequestProcessor::locateRoutePath(incomingRequest, *serverMatch);
			if (!routeMatch)
				outgoingResponse = HttpRequestProcessor::craftErrorReply(NotFound, incomingRequest, serverMatch);
			else
				outgoingResponse = HttpRequestProcessor::craftHttpReply(incomingRequest, serverMatch, routeMatch);
		}
	} catch (const std::invalid_argument& parseEx) {
		Logger::error(parseEx.what());
		outgoingResponse = HttpRequestProcessor::craftErrorReply(BadRequest, incomingRequest, serverMatch);
	} catch (const std::exception& genericEx) {
		Logger::error(genericEx.what());
		outgoingResponse = HttpRequestProcessor::craftErrorReply(InternalServerError, incomingRequest, serverMatch);
	}
	return outgoingResponse;
}

const ServerConfig*	HttpRequestProcessor::locateServerBlock(const Request& req,
																	const std::string& ip,
																	const std::string& port,
																	const Config& settings) {
	Logger::debug("HttpRequestProcessor::locateServerBlock");
	const ServerConfig* backupServer = NULL;

	for (size_t index = 0; index < settings.serverConfigurations.size(); ++index) {
		const ServerConfig& currentServer = settings.serverConfigurations[index];
		if (currentServer.ipAddress == ip && currentServer.port == port) {
			const std::string& hostHeader = req.httpHeaders.at("Host");
			for (size_t alias = 0; alias < currentServer.serverName.size(); ++alias) {
				if (currentServer.serverName[alias] + ":" + currentServer.port == hostHeader)
					return &currentServer;
			}
			if (!backupServer)
				backupServer = &currentServer;
		}
	}
	return backupServer;
}


const ServerLocation* HttpRequestProcessor::locateRoutePath(const Request& req,
																	const ServerConfig& serverBlock) {
	Logger::debug("HttpRequestProcessor::locateRoutePath");
	size_t matchedLength = 0;
	const ServerLocation* bestRoute = NULL;

	for (size_t i = 0; i < serverBlock.location_list.size(); ++i) {
		const ServerLocation& candidateRoute = serverBlock.location_list[i];

		if (req.resourcePath == candidateRoute.path)
			return &candidateRoute;

		if (req.resourcePath.find(candidateRoute.path) == 0 && candidateRoute.path.size() > matchedLength) {
			bestRoute = &candidateRoute;
			matchedLength = candidateRoute.path.size();
		}
	}
	return bestRoute;
}

std::string	HttpRequestProcessor::craftHttpReply(const Request& req,
												const ServerConfig* server,
												const ServerLocation* route) {
	Logger::debug("HttpRequestProcessor::craftHttpReply");

	if (route) {
		if (!isRequestPermitted(req, *route))
			return HttpRequestProcessor::craftErrorReply(MethodNotAllowed, req, server);
		if (!route->redirect.empty())
			return HttpRequestProcessor::redirectClient(route->redirect);
		if (!route->cgiPath.empty())
			return HttpRequestProcessor::executeCGIScript(req, server, route);
		}

		std::string result;
		std::string resourcePath = HttpRequestProcessor::mapToAbsolutePath(req, route);

		if (req.method == GET)
			result = processGET(resourcePath, req, server, route);
		else if (req.method == POST)
			result = processPOST(resourcePath, req, server);
		else if (req.method == DELETE)
			result = processDELETE(resourcePath, req, server);

		return result;
}

std::string	HttpRequestProcessor::produceFallbackPage() {
	Logger::debug("HttpRequestProcessor::produceFallbackPage");
	Response  defaultReply(Ok);
	defaultReply.body = PageRenderer::renderDefaultPage();
	defaultReply.httpHeaders.insert(std::make_pair("Content-Type", "text/html"));
	return Parser::createHttpResponse(defaultReply);
}

std::string	HttpRequestProcessor::craftErrorReply(StatusCode errorCode,
												const Request& req,
												const ServerConfig* server) {
	Logger::debug("HttpRequestProcessor::craftErrorReply");
	try {
		Logger::error(req.httpHeaders.at("Host") + req.resourcePath + ": " + Parser::mapStatusToString(errorCode));
	}
	catch(...) {
		Logger::error("Unknown error while logging.");
	}

	Response  errorReply(errorCode);

	if (server && server->error_page.find(errorCode) != server->error_page.end()) {
		errorReply.body = PageRenderer::renderErrorPage(
		server->error_page.at(errorCode).c_str());
	}
	else {
		errorReply.body = PageRenderer::renderErrorPage(errorReply.statusCode);
	}

	return Parser::createHttpResponse(errorReply);
}

std::string	HttpRequestProcessor::redirectClient(const std::string& targetUrl) {
	Response  redirectReply(MovedPermanently);
	redirectReply.httpHeaders.insert(std::make_pair("Location", targetUrl));
	return Parser::createHttpResponse(redirectReply);
}

std::string	HttpRequestProcessor::executeCGIScript(const Request& req,
													const ServerConfig* server,
													const ServerLocation* route) {
	if (req.body.size() > static_cast<size_t>(::atoi(server->max_body_size.c_str())))
		return HttpRequestProcessor::craftErrorReply(PayloadTooLarge, req, server);

	std::string scriptPath;

	if (route->cgiPath[0] != '/')
		scriptPath = File::getCurrDir() + '/' + route->cgiPath;
	else
		scriptPath = route->cgiPath;

	Logger::debug("CGI Script Path:" + scriptPath);

	if (!File::pathExists(scriptPath.c_str()))
		return craftErrorReply(InternalServerError, req, server);

	std::string scriptOutput;
	try {
		scriptOutput = CgiHandler::instance_.runScript(scriptPath, req, *server);
		Logger::logInfo("{" + scriptOutput + "}");
	} catch (CgiHandler::GatewayTimeoutException& timeoutErr) {
		return HttpRequestProcessor::craftErrorReply(GatewayTimeOut, req, server);
	}

	Response  finalResponse(Ok);

	return Parser::createHttpResponse(finalResponse, false) + scriptOutput;
}

std::string	HttpRequestProcessor::processGET(const std::string& resolvedPath,
											const Request& req,
											const ServerConfig* server,
											const ServerLocation* route) {
	Logger::debug("HttpRequestProcessor::processGET");
	Logger::debug("Resolved path : " + resolvedPath);
	Logger::debug("Route path : " + route->path);

	if (route && File::isDir(resolvedPath.c_str())) {
		//Logger::debug("RequestHandler::responseFromGet CASE 1");
		if (!route->index.empty())
			return resolveIndexRoute(resolvedPath, req, *route);
		else if (route->autoIndex)
			return generateAutoIndexView(resolvedPath, req);
	}

	//Logger::debug("RequestHandler::responseFromGet CASE 2");
	std::string fileContent;
	try {
		fileContent = File::readFile(resolvedPath.c_str());
	}
	catch(const Exceptions::FileDoesNotExist& e) {
		return craftErrorReply(NotFound, req, server);
	} catch(...) {
		throw;
	}
	Response fileResponse(Ok);
	fileResponse.body = fileContent;
	fileResponse.httpHeaders.insert(std::make_pair("Content-Type",HttpRequestProcessor::inferMimeType(resolvedPath)));

	return Parser::createHttpResponse(fileResponse);
}

std::string	HttpRequestProcessor::processPOST(const std::string& resolvedPath,
														const Request& req,
														const ServerConfig* server)	{
	Logger::debug("HttpRequestProcessor::processPOST");

	if (req.body.size() > static_cast<size_t>(::atoi(server->max_body_size.c_str())))
		return HttpRequestProcessor::craftErrorReply(PayloadTooLarge, req, server);

	if (File::isDir(resolvedPath.c_str()))
		return HttpRequestProcessor::craftErrorReply(BadRequest, req, server);

	Response  uploadReply;

	if (File::pathExists(resolvedPath.c_str())){
		File::writeFile(req.body, resolvedPath.c_str(), true);
		uploadReply.statusCode = NoContent;
	}
	else {
		File::writeFile(req.body, resolvedPath.c_str());
		uploadReply.statusCode = Created;
	}

	return Parser::createHttpResponse(uploadReply);
}


std::string	HttpRequestProcessor::processDELETE(const std::string& resolvedPath,
															const Request& req,
															const ServerConfig* server) {
	Logger::debug("HttpRequestProcessor::processDELETE");

	if (!File::pathExists(resolvedPath.c_str()))
		return HttpRequestProcessor::craftErrorReply(NotFound, req, server);
	if (File::isDir(resolvedPath.c_str()))
		return HttpRequestProcessor::craftErrorReply(Conflict, req, server);

	Response deleteReply;

	std::string data = File::readFile(resolvedPath.c_str());
	::unlink(resolvedPath.c_str());

	deleteReply.statusCode = Ok;
	deleteReply.body = data;

	deleteReply.httpHeaders.insert(std::make_pair("Content-Type", HttpRequestProcessor::inferMimeType(resolvedPath)));

	return Parser::createHttpResponse(deleteReply);
}

std::string	HttpRequestProcessor::resolveIndexRoute(const std::string& resolvedPath,
													const Request& req,
													const ServerLocation& route) {
	Logger::debug("HttpRequestProcessor::resolveIndexRoute");

	size_t foundIdx = 0;
	for (; foundIdx < route.index.size(); ++foundIdx) {
		if (File::pathExists((resolvedPath + route.index[foundIdx]).c_str()))
			break;
	}

	if (foundIdx == route.index.size()) {
		if (route.autoIndex)
			return generateAutoIndexView(resolvedPath, req);
		throw Exceptions::FileDoesNotExist();
	}

	Response indexReply(Ok);
	indexReply.body = File::readFile((resolvedPath + route.index[foundIdx]).c_str());
	indexReply.httpHeaders.insert(std::make_pair("Content-Type", HttpRequestProcessor::inferMimeType(route.index[foundIdx])));

	return Parser::createHttpResponse(indexReply);
}


std::string	HttpRequestProcessor::generateAutoIndexView(const std::string& resolvedPath,
															const Request& req) {
	Logger::debug("HttpRequestProcessor::generateAutoIndexView");
	Response autoIndexReply(Ok);
	autoIndexReply.body = PageRenderer::renderDirectoryIndex(resolvedPath, req.resourcePath);
	autoIndexReply.httpHeaders.insert(std::make_pair("Content-Type", "text/html"));

	return Parser::createHttpResponse(autoIndexReply);
}

std::string HttpRequestProcessor::mapToAbsolutePath(const Request& req,
												const ServerLocation* route) {
	Logger::debug("HttpRequestProcessor::mapToAbsolutePath");

	if (route && !route->root.empty()) {
		std::string rootPrefix;
		if (route->root[0] != '/')
		rootPrefix = File::getCurrDir() + "/" + route->root;
		rootPrefix = route->root;
		std::string relativeUri = req.resourcePath;
		if (!File::pathExists((rootPrefix + req.resourcePath).c_str()))
		relativeUri.erase(0, route->path.size());

		std::string completePath = rootPrefix + relativeUri;
		while (completePath.find("//") != std::string::npos) {
			completePath.erase(completePath.find("//"), 1);
		}
		Logger::debug("HttpRequestProcessor::mapToAbsolutePath = " + completePath);
		return completePath;
	}
	std::string	completePath2 = File::getCurrDir() + req.resourcePath;
	Logger::debug("HttpRequestProcessor::mapToAbsolutePath = " + completePath2);
	return (completePath2);
}


bool	HttpRequestProcessor::isRequestPermitted(const Request& req, const ServerLocation& route) {
	Logger::debug("HttpRequestProcessor::isRequestPermitted");

	if (!route.method.size())
		return true;

	return (std::find(route.method.begin(), route.method.end(),
	Parser::mapMethodToString(req.method)) != route.method.end());
}


std::string HttpRequestProcessor::inferMimeType(const std::string& fileExt) {
	Logger::debug("HttpRequestProcessor::inferMimeType");

	size_t dotIndex = fileExt.rfind('.');
	if (dotIndex == std::string::npos) {
		return "text/plain";
	}
	std::string ext = String::toLowercase(fileExt.substr(dotIndex));

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
