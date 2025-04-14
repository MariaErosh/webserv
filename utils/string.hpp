#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <exception>


	class String {
	public:
		static std::vector<std::string>  tokenize(const std::string& input, const std::string& delimiter);
		static std::vector<std::string>  tokenize(const std::string& input, char delimiter);
		static std::vector<std::string>  tokenizeOnce(const std::string& input, const std::string& delimiter);
		static std::vector<std::string>  splitByWhitespace(std::string line);

		template <typename T>
		static std::string  convertToString(T data);

		static std::string  toLowercase(std::string text);
	};

	template <typename T>
	std::string	String::convertToString(T data) {
		std::ostringstream result;
		result << data;
		return result.str();
  	}

