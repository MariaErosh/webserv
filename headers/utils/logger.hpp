#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sys/types.h>
 
namespace Utils {
	class Logger {
		public:
			enum LogLevel
			{
				LOGLEV_ERROR = 0,
				LOGLEV_INFO = 0,
				LOGLEV_DEBUG = 0
			};

		private:
			static const bool         timestamp_enabled_  = true;
			static const int          prompt_max_width_   = 5;		
			static const bool         duplicate_to_file_ = true;
			static LogLevel           minimum_log_level_;
			static const std::string  logs_directory_;
			static std::ofstream      output_fstream_;

			/*  The function logs a message to a given output stream (os), optionally with a timestamp, 
				a custom prompt label (like "INFO" or "ERROR"), and optional ANSI color codes for terminal coloring.
			*/
			static void log(const std::string& message, std::ostream& os,const char *prompt = "LOG", const char *colors = NULL);
			
			Logger();
			Logger(Logger& other);
			Logger &operator=(const Logger& other);
			~Logger();
		

		public:
			static Logger instance_;

			/* Initialization of Logger.
				min_log_level    Minimum level for logging (LOGLEV_ERROR, LOGLEV_INFO or LOGLEV_DEBUG)
				dupl_to_file_   Creates log file if true, or use console output only otherwise.			
			*/
			static void init(LogLevel min_log_level = LOGLEV_INFO, bool dupl_to_file = true);		

			// Print info message to stdout.
			static void info(const std::string& message);

			// Print debug message to stdout.
			static void debug(const std::string& message);

			// Print error message to stderr. status_code - status to return from function
			static int  error(const std::string& message, int status_code = -1);
	};
}

