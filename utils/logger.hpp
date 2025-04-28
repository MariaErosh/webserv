#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sys/types.h>


	class Logger {
		public:
			enum LogLevel
			{
				LOG_ERROR = 0,
				LOG_INFO = 5,
				LOG_DEBUG = 10
			};

		private:
			static const bool         enableTimestamp_  = true;
			static const int          maxPromptWidth_   = 5;
			static const bool         logToFile_ = true;
			static LogLevel           currentLogLevel_;
			static const std::string  logDirectory_;
			static std::ofstream      logFileStream_;


			static void log(const std::string& message, std::ostream& os,const char *prompt = "LOG", const char *colors = NULL);
			Logger();
			Logger(Logger& other);
			Logger &operator=(const Logger& other);
			~Logger();


		public:
			static Logger instance_;

			static void initialize(LogLevel minLogLevel = LOG_INFO, bool writeToLogFile = true);
			static void logInfo(const std::string& message);
			static void debug(const std::string& message);
			static int  error(const std::string& message, int status_code = -1);
	};


