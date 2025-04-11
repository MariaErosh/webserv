#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include "../../headers/utils/logger.hpp"
#include "../../headers/utils/file.hpp"
#include "../../headers/utils/exceptions.hpp"


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
		std::string absolute_path;
		if (filename[0] != '/')
		absolute_path = File::getCurrDir() + "/" + std::string(filename);
		else
		absolute_path = std::string(filename);


		Logger::debug(absolute_path);
		std::ifstream file(absolute_path.c_str());

		if (!file.is_open())
		throw Exceptions::FileDoesNotExist();

		std::stringstream buffer;
		buffer << file.rdbuf();

		std::string text = buffer.str();
		Logger::debug(text);
		return text;
	}

	void  File::createPath(const char* path, mode_t mode) {

		char *copy_path = strdup(path);
		char *sbegin_ptr = copy_path;
		char *send_ptr = NULL;

		while ((send_ptr = strchr(sbegin_ptr, '/')) != 0) {
			if (sbegin_ptr != send_ptr)	{
				*send_ptr = '\0';
				File::createDir(copy_path, mode);
				*send_ptr = '/';
			}
			sbegin_ptr = send_ptr + 1;
		}
		File::createDir(copy_path, mode);
		free(copy_path);
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

		if (stat(path, &st) != 0) // case when file doesn't exists
			return false;
		return S_ISREG(st.st_mode) != 0;
	}

	bool	File::isDir(const char *path) {
		struct stat st;
		if (stat(path, &st) != 0 || S_ISREG(st.st_mode)) // case when dir doesn't exists or is a file
			return false;
		return S_ISDIR(st.st_mode) != 0;
	}	

	bool	File::pathExists(const char *path) {
		struct stat st;
		return (stat(path, &st) == 0);
	}	
  

