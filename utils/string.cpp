#include "string.hpp"
#include <cctype>

std::string   String::toLowercase(std::string text) {
	for (size_t i = 0; i < text.size(); ++i)
	text[i] = ::tolower(text[i]);

	return text;
}

	std::vector<std::string> splitHelper(const std::string& input, const std::string& delimiter) {
		std::vector<std::string> tokens;
		const size_t delimiterLength = delimiter.length();

		size_t startPos = 0;
		size_t endPos = input.find(delimiter);
		tokens.push_back(input.substr(startPos, endPos));

		while (endPos != std::string::npos) {
			startPos = endPos + delimiterLength;
			endPos = input.find(delimiter, startPos);
			if (endPos == std::string::npos || endPos > startPos)
				tokens.push_back(input.substr(startPos, endPos - startPos));
		}
		return tokens;
	}


	std::vector<std::string> String::tokenize(const std::string& input, const std::string& delimiter) {
		return splitHelper(input, delimiter);
	}

	std::vector<std::string> String::tokenize(const std::string& input, char delimiter) {
		std::string delimiterStr(1, delimiter);
		return splitHelper(input, delimiterStr);
	}

	std::vector<std::string>  String::tokenizeOnce(const std::string& input, const std::string& delimiter) {
		std::vector<std::string> tokens;
		const size_t delimiterLength = delimiter.length();

		size_t startPos = 0;
		size_t endPos = input.find(delimiter);
		tokens.push_back(input.substr(startPos, endPos));

		if (endPos != std::string::npos) {
			startPos = endPos;
			endPos = input.find(delimiter, startPos + delimiterLength);
			while (endPos != std::string::npos && (endPos - startPos == delimiterLength))
				startPos += delimiterLength;
			tokens.push_back(input.substr(startPos + delimiterLength, std::string::npos));
		}

		return tokens;
	}

	std::vector<std::string> String::splitByWhitespace(std::string line) {
		std::vector<std::string> tokens;
		std::istringstream stream(line);
		std::string word;

		while (stream >> word) {
			tokens.push_back(word);
		}

		return tokens;
	}

