#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include "logger.hpp"
#include "file.hpp"
#include "exceptions.hpp"


	std::string	File::getCurrDir(void) {
		char pwd[1024];
		getcwd(pwd, 1024);
		return pwd;
	}

	void  File::writeFile(const std::string& data, const char *filepath, bool override)
	{
		std::ofstream fout(filepath, (override ? std::ios_base::trunc : std::ios_base::app));

		if (!fout.is_open())
			throw std::runtime_error("writeFile exception: can't open file " + std::string(filepath));

		fout << data << std::flush;

		fout.close();
	}

	std::string File::readFile(const char *filename)
	{
		std::string absolutePath;
		if (filename[0] != '/')
			absolutePath = File::getCurrDir() + "/" + std::string(filename);
		else
			absolutePath = std::string(filename);


		Logger::debug(absolutePath);
		std::ifstream file(absolutePath.c_str());

		if (!file.is_open())
		throw Exceptions::FileDoesNotExist();

		std::stringstream buffer;
		buffer << file.rdbuf();

		std::string text = buffer.str();
		Logger::debug(text);
		return text;
	}

	void  File::createPath(const char* path, mode_t mode) {

		char *copyPath = strdup(path);
		char *segmentStart = copyPath;
		char *segmentEnd = NULL;

		while ((segmentEnd = strchr(segmentStart, '/')) != 0) {
			if (segmentStart != segmentEnd) {
				*segmentEnd = '\0';
				File::createDir(copyPath, mode);
				*segmentEnd = '/';
			}
			segmentStart = segmentEnd + 1;
		}
		File::createDir(copyPath, mode);
		free(copyPath);
	}

	void  File::createDir(const char* dir, mode_t mode) {
		struct stat   st;

		if (stat(dir, &st) != 0) {
			if (mkdir(dir, mode) != 0)
				throw std::runtime_error(strerror(errno));
		}
		else if (!S_ISDIR(st.st_mode)) {
			throw std::runtime_error(strerror(ENOTDIR));
		}
	}

	bool	File::isFile(const char *path) {
		struct stat st;

		if (stat(path, &st) != 0)
			return false;
		return S_ISREG(st.st_mode) != 0;
	}

	bool	File::isDir(const char *path) {
		struct stat st;
		if (stat(path, &st) != 0 || S_ISREG(st.st_mode))
			return false;
		return S_ISDIR(st.st_mode) != 0;
	}

	bool	File::pathExists(const char *path) {
		struct stat st;
		return (stat(path, &st) == 0);
	}


