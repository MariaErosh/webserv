#pragma once

#include <string>
#include <sys/stat.h>


  class File{
	public:
		static std::string	getCurrDir(void);
		static void			writeFile(const std::string& data, const char *filepath, bool override=true);
		static std::string	readFile(const char *filepath);
		static void			createPath(const char* path, mode_t mode = 0700);
		static void			createDir(const char* dir, mode_t mode = 0700);
		static bool			isFile(const char *path);
		static bool			isDir(const char *path);
		static bool			pathExists(const char *path);

	private:
		File(void) {}

	};


