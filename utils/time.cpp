#include "time.hpp"
#include <ctime>


std::string Time::getTimestamp(const char *format, bool localtime) {
    time_t      rawtime = std::time(NULL);

    struct tm*  tmInfo = NULL;
    if (localtime)
      tmInfo = std::localtime(&rawtime);
    else
      tmInfo = std::gmtime(&rawtime);

    char    buffer[30] = { '\0' };
    strftime(buffer, sizeof(buffer), format, tmInfo);

    return std::string(buffer);
}

