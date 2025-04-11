#pragma once

#include <string>

class Time {
	private:
		Time(void) {}

	public:
		static std::string getTimestamp(const char *format = "%Y-%m-%d %H:%M:%S", bool localtime=true);
};
