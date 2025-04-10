#include "../../headers/utils/logger.hpp"
#include "../../headers/utils/string.hpp"
#include "../../headers/utils/time.hpp"
#include "../../headers/http/parser/parser.hpp"


namespace Http {
	bool Parser::needBodyForStatus(StatusCode status_code) {
		// 1xx, 204, and 304 -- NO BODY
		// all other: body or 'Content-Lenght: 0' if no body provided
		return !((status_code > 100 && status_code < 200) // 1xx
				|| status_code == 204
				|| status_code == 304);
	}

	std::string  Parser::serializeResponse(const Response& data, bool ending) {
		Utils::Logger::debug("Http::Parser::serializeResponse");

		//std::stringstream ss;
		std::stringstream result;

		result << data.version << ' '<< Parser::statusToString(data.status_code) << "\r\n";
		result << "Date: " << Utils::Time::getTimestamp("%a, %d %b %Y %H:%M:%S GMT", false) << "\r\n";

		/*{
			std::map<std::string, std::string>::const_iterator  iter;
			for (iter = data.headers.begin(); iter != data.headers.end(); ++iter)
				result << iter->first << ": " << iter->second << "\r\n";
		}*/
		for (std::map<std::string, std::string>::const_iterator iter = data.headers.begin(); iter != data.headers.end(); ++iter) {
			result << iter->first << ": " << iter->second << "\r\n";
		}

		// body
		if (ending) {
			if (Parser::needBodyForStatus(data.status_code)) {
				result << "Content-Length: " << data.body.size() << "\r\n\r\n" << data.body;
			}
			else {
				result << "\r\n";
			}
		}
		return result.str();
	}

	Request	Parser::deserializeRequest(const std::string& data)	{
		Utils::Logger::debug("Http::Parser::deserializeRequest");

		if (data.empty())
			throw std::invalid_argument("deserializeRequest exception: empty request");

		// split headers and body
		std::vector<std::string>  splitted_request = Utils::String::splitOnce(data, "\r\n\r\n");

		// split start-line and headers
		std::vector<std::string>  splitted_raw_headers = Utils::String::split(splitted_request[0], "\r\n");

		Request request;
		// start-line
		{      
			std::vector<std::string> splitted_startline = Utils::String::split(
				splitted_raw_headers[0], ' ');

			if (splitted_startline.size() != 3)
				throw std::invalid_argument("deserializeRequest exception: invalid start-line of request");

			request.method = Parser::stringToMethod(splitted_startline[0]);
			request.uri = splitted_startline[1];
			request.version = splitted_startline[2];
		}

		// headers
		{
			std::vector<std::string> splitted_header;
			size_t headers_index = 1;

			while (headers_index < splitted_raw_headers.size())
			{
				// clean up
				size_t newline_index = splitted_raw_headers[headers_index].rfind("\r\n");
				if (newline_index != std::string::npos)
				splitted_raw_headers[headers_index].erase(newline_index, 1);

				// insert
				splitted_header = Utils::String::split(splitted_raw_headers[headers_index], ": ");
				request.headers.insert(std::make_pair(splitted_header[0], splitted_header[1]));
				++headers_index;
			}
		}

		// body
		{
			if (request.method == POST)	{
				if (splitted_request.size() <= 1)
					throw std::invalid_argument("deserializeRequest exception: missing empty line after headers");
				request.body = splitted_request[1];
			}
		}
		return request;
	}  

	std::string Parser::methodToString(Method method) {
		Utils::Logger::debug("Http::Parser::methodToString");		
		
		if (method == POST) return "POST";
		if (method == GET) return "GET";
		if (method == DELETE) return "DELETE";
		throw std::invalid_argument("unsupported HTTP method");
	}

	Method	Parser::stringToMethod(const std::string& source) {
		Utils::Logger::debug("Http::Parser::stringToMethod");		
		
		if (source == "POST") return POST;
		if (source == "GET") return GET;
		if (source == "DELETE") return DELETE;
		throw std::invalid_argument("unsupported HTTP method");
	}	

	std::string Parser::statusToString(StatusCode status_code) {
		Utils::Logger::debug("Http::Parser::statusToString");
		
		if (status_code == Continue)            return "100 Continue";
		if (status_code == Processing)          return "102 Processing";
		if (status_code == Ok)                  return "200 OK";
		if (status_code == Created)             return "201 Created";
		if (status_code == Accepted)            return "202 Accepted";
		if (status_code == NoContent)           return "204 No Content";
		if (status_code == MovedPermanently)    return "301 Moved Permanently";
		if (status_code == BadRequest)          return "400 Bad Request";
		if (status_code == Forbidden)           return "403 Forbidden";
		if (status_code == NotFound)            return "404 Not Found";
		if (status_code == MethodNotAllowed)    return "405 Method Not Allowed";
		if (status_code == RequestTimeout)      return "408 Request Timeout";
		if (status_code == Conflict)            return "409 Conflict";
		if (status_code == LengthRequired)      return "411 Length Required";
		if (status_code == PayloadTooLarge)     return "413 Payload Too Large";
		if (status_code == InternalServerError) return "500 Internal Server Error";
		if (status_code == NotImplemented)      return "501 Not Implemented";
		if (status_code == GatewayTimeOut)      return "504 Gateway TimeOut";
		
		throw std::invalid_argument("unsupported status code");
	}	
}
