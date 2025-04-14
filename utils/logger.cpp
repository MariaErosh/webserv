#include <iostream>
#include <iomanip>

#include "./logger.hpp"
#include "./time.hpp"
#include "./file.hpp"


	Logger  Logger::instance_;
	Logger::LogLevel  Logger::currentLogLevel_ = LOG_ERROR;
	const std::string Logger::logDirectory_ = "resources/logs";
	std::ofstream     Logger::logFileStream_;

	Logger::Logger(){};
	Logger::~Logger(){};
	Logger::Logger(Logger& other) { (void)other; }
	Logger& Logger::operator=(const Logger& other) { (void)other; return *this;}


	void  Logger::initialize(LogLevel minLogLevel, bool writeToLogFile){
		if (writeToLogFile) {
			File::createPath(Logger::logDirectory_.c_str());
			std::string date = Time::getTimestamp("%Y-%m-%d");
			std::string filename = Logger::logDirectory_ + "/log_" + date + ".txt";
			const char* filepath = filename.c_str();
			Logger::logFileStream_.open(filepath
										, std::ios_base::app);
		}
		Logger::currentLogLevel_ = minLogLevel;
	}

	void Logger::logInfo(const std::string& message) {
		if (currentLogLevel_ < LOG_INFO)
			return;

		Logger::log(message, std::cout, "INFO", "\x1B[32m");

		if (Logger::logFileStream_.is_open())
			Logger::log(message, Logger::logFileStream_, "INFO");
	}

	void Logger::debug(const std::string& message) {
		if (currentLogLevel_ < LOG_DEBUG)
			return;

		Logger::log(message, std::cout, "DEBUG", "\x1B[33m");

		if (Logger::logFileStream_.is_open())
			Logger::log(message, Logger::logFileStream_, "DEBUG");
	}

	int  Logger::error(const std::string& message, int statusCode) {
		if (currentLogLevel_ < LOG_ERROR)
			return statusCode;
		Logger::log(message, std::cerr, "ERROR", "\x1B[1;31m");
		if (Logger::logFileStream_.is_open())
			Logger::log(message, Logger::logFileStream_, "ERROR");
		return statusCode;
	}

	void Logger::log(const std::string& message, std::ostream& os,
					const char *prompt, const char *colors) {
		if (Logger::enableTimestamp_)
			os << '[' << Time::getTimestamp() << "] ";

		if (colors)
			os << colors;

		os << '[' << std::setiosflags(std::ios_base::left)
			<< std::setw(Logger::maxPromptWidth_) << prompt << "] ";

		if (colors)
			os << "\x1B[m";


		os << message << std::endl;
	}


