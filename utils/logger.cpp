#include <iostream>
#include <iomanip>

#include "./logger.hpp"
#include "./time.hpp"
#include "./file.hpp"


	Logger  Logger::instance_;
	Logger::LogLevel  Logger::minimum_log_level_ = LOGLEV_ERROR;
	const std::string Logger::logs_directory_ = "resources/logs";
	std::ofstream     Logger::output_fstream_;

	Logger::Logger(){};
	Logger::~Logger(){};
	Logger::Logger(Logger& other) { (void)other; }                
	Logger& Logger::operator=(const Logger& other) { (void)other; return *this;}
	

	void  Logger::init(LogLevel min_log_level, bool dupl_to_file){
		if (dupl_to_file) {
			File::createPath(Logger::logs_directory_.c_str());
			std::string date = Time::getTimestamp("%Y-%m-%d");
			std::string filename = Logger::logs_directory_ + "/log_" + date + ".txt";
			const char* filepath = filename.c_str();
			Logger::output_fstream_.open(filepath						
										, std::ios_base::app
        								);
      	}
		Logger::minimum_log_level_ = min_log_level;
	}

	void Logger::info(const std::string& message) {
		if (minimum_log_level_ < LOGLEV_INFO)
			return;

		Logger::log(message, std::cout, "INFO", "\x1B[32m");

		if (Logger::output_fstream_.is_open())
			Logger::log(message, Logger::output_fstream_, "INFO");
	}

	void Logger::debug(const std::string& message) {
		if (minimum_log_level_ < LOGLEV_DEBUG)
			return;

		Logger::log(message, std::cout, "DEBUG", "\x1B[33m");

		if (Logger::output_fstream_.is_open())
			Logger::log(message, Logger::output_fstream_, "DEBUG");
	}

	int  Logger::error(const std::string& message, int status_code) {
		if (minimum_log_level_ < LOGLEV_ERROR)
			return status_code;
		Logger::log(message, std::cerr, "ERROR", "\x1B[1;31m");
		if (Logger::output_fstream_.is_open())
			Logger::log(message, Logger::output_fstream_, "ERROR");
		return status_code;
	}
    
	void Logger::log(const std::string& message, std::ostream& os,
					const char *prompt, const char *colors) {
		if (Logger::timestamp_enabled_)
			os << '[' << Time::getTimestamp() << "] ";

		if (colors)
			os << colors;

		os << '[' << std::setiosflags(std::ios_base::left)
			<< std::setw(Logger::prompt_max_width_) << prompt << "] ";

		if (colors)
			os << "\x1B[m";


		os << message << std::endl;
	}


