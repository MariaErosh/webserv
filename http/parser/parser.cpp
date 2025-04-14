#include "../../utils/logger.hpp"
#include "../../utils/string.hpp"
#include "../../utils/time.hpp"
#include "../../http/parser/parser.hpp"

	bool Parser::requiresBody(StatusCode statusCode) {
		return !((statusCode > 100 && statusCode < 200)
				|| statusCode == 204
				|| statusCode == 304);
	}

	std::string  Parser::createHttpResponse(const Response& responseData, bool includeBody) {
		Logger::debug("createHttpResponse");

		std::stringstream result;

		result << responseData.protocolVersion << ' '<< mapStatusToString(responseData.statusCode) << "\r\n";
		result << "Date: " << Time::getTimestamp("%a, %d %b %Y %H:%M:%S GMT", false) << "\r\n";
		for (std::map<std::string, std::string>::const_iterator iter = responseData.httpHeaders.begin(); iter != responseData.httpHeaders.end(); ++iter) {
			result << iter->first << ": " << iter->second << "\r\n";
		}
		if (includeBody) {
			if (requiresBody(responseData.statusCode)) {
				result << "Content-Length: " << responseData.body.size() << "\r\n\r\n" << responseData.body;
			}
			else {
				result << "\r\n";
			}
		}
		return result.str();
	}

	Request	Parser::parseHttpRequest(const std::string& rawRequest) {
		Logger::debug("parseHttpRequest");

		if (rawRequest.empty())
			throw std::invalid_argument("parseHttpRequest exception: empty request");

		std::vector<std::string>  requestParts = String::tokenizeOnce(rawRequest, "\r\n\r\n");

		std::vector<std::string>  headerLines = String::tokenize(requestParts[0], "\r\n");

		Request parsedRequest;
		{
			std::vector<std::string> startLineTokens = String::tokenize(headerLines[0], ' ');

			if (startLineTokens.size() != 3)
				throw std::invalid_argument("parseHttpRequest exception: invalid start-line of request");

			parsedRequest.method = mapStringToMethod(startLineTokens[0]);
			parsedRequest.resourcePath = startLineTokens[1];
			parsedRequest.protocolVersion = startLineTokens[2];
		}
		{
			std::vector<std::string> headerTokens;
			size_t headerIndex = 1;

			while (headerIndex < headerLines.size())
			{
				size_t newline_index = headerLines[headerIndex].rfind("\r\n");
				if (newline_index != std::string::npos)
				headerLines[headerIndex].erase(newline_index, 1);
				headerTokens = String::tokenize(headerLines[headerIndex], ": ");
				parsedRequest.httpHeaders.insert(std::make_pair(headerTokens[0], headerTokens[1]));
				++headerIndex;
			}
		}
		{
			if (parsedRequest.method == POST)	{
				if (requestParts.size() <= 1)
					throw std::invalid_argument("parseHttpRequest exception: missing empty line after headers");
				parsedRequest.body = requestParts[1];
			}
		}
		return parsedRequest;
	}

	std::string Parser::mapMethodToString(Method method) {
		Logger::debug("mapMethodToString");

		if (method == POST) return "POST";
		if (method == GET) return "GET";
		if (method == DELETE) return "DELETE";
		throw std::invalid_argument("unsupported HTTP method");
	}

	Method	Parser::mapStringToMethod(const std::string& methodString) {
		Logger::debug("mapStringToMethod");

		if (methodString == "POST") return POST;
		if (methodString == "GET") return GET;
		if (methodString == "DELETE") return DELETE;
		throw std::invalid_argument("unsupported HTTP method");
	}

	std::string Parser::mapStatusToString(StatusCode statusCode) {
		Logger::debug("mapStatusToString");

		if (statusCode == Continue)            return "100 Continue";
		if (statusCode == Processing)          return "102 Processing";
		if (statusCode == Ok)                  return "200 OK";
		if (statusCode == Created)             return "201 Created";
		if (statusCode == Accepted)            return "202 Accepted";
		if (statusCode == NoContent)           return "204 No Content";
		if (statusCode == MovedPermanently)    return "301 Moved Permanently";
		if (statusCode == BadRequest)          return "400 Bad Request";
		if (statusCode == Forbidden)           return "403 Forbidden";
		if (statusCode == NotFound)            return "404 Not Found";
		if (statusCode == MethodNotAllowed)    return "405 Method Not Allowed";
		if (statusCode == RequestTimeout)      return "408 Request Timeout";
		if (statusCode == Conflict)            return "409 Conflict";
		if (statusCode == LengthRequired)      return "411 Length Required";
		if (statusCode == PayloadTooLarge)     return "413 Payload Too Large";
		if (statusCode == InternalServerError) return "500 Internal Server Error";
		if (statusCode == NotImplemented)      return "501 Not Implemented";
		if (statusCode == GatewayTimeOut)      return "504 Gateway TimeOut";

		throw std::invalid_argument("unsupported status code");
	}
