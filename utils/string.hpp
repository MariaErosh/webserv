#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <exception>


	class String {
	public:
		//Split string by delimeter(string)
		static std::vector<std::string>  split(const std::string& source, const std::string& delim);

		//Split string by delimeter(symbol)
		static std::vector<std::string>  split(const std::string& source, char delim);


		/*  The function attempts to split a string only once using the specified delimiter.
			source       string to split
			delim        delimiter as string
		*/
		static std::vector<std::string>  splitOnce(const std::string& source, const std::string& delim);

		// The function splits(by space) string for parsing.
		static std::vector<std::string>  splitStr(std::string line);

		template <typename T>
		static std::string  to_string(T data);

		// Convert string to lowercase 
		static std::string  toLower(std::string text);
	};
  
	template <typename T>
	std::string	String::to_string(T data) {
		std::ostringstream result;
		result << data;
		return result.str();
  	}

