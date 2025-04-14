#pragma once

#include "../models/request.hpp"
#include "../models/response.hpp"
#include "../models/methods.hpp"
#include <sstream>

	class Parser {
		public:

			static std::string	createHttpResponse(const Response& responseData, bool includeBody=true);
			static Request		parseHttpRequest(const std::string& rawRequest);
			static bool			requiresBody(StatusCode statusCode);
			static Method		mapStringToMethod(const std::string& methodString);
			static std::string	mapStatusToString(StatusCode statusCode);
			static std::string	mapMethodToString(Method method);
		};

