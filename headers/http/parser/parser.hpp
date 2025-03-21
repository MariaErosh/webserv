#pragma once

#include "../models/request.hpp"
#include "../models/response.hpp"
#include "../models/methods.hpp"
#include <sstream>

namespace WS {
	namespace Http {
		class Parser
		{
		public:
			/* The function for deserializing Request from client. It returns filled Request structure*/
			static Request	deserializeRequest(const std::string& data);

			/* Function for serializing server Response to ready-to-send data in std::string container.
			data - string to parse
			ending - is 'r\n\r\n' between headers and body required
			The function filled Response structure data and returns raw serialized request in std::string container
			*/
			static std::string	serializeResponse(const Response& data, bool ending=true);
			static Method		stringToMethod(const std::string& source);
			static std::string	methodToString(Method method);
			static std::string	statusToString(StatusCode status_code);
			static bool			needBodyForStatus(StatusCode status_code);
		};
	}
}
