#pragma once

#include <string>
#include <sys/stat.h>

namespace Utils {
  class File{
	public:
		// The function return current work directory
		static std::string  getCurrDir(void);

		// Write or override file filepath with data content.
		static void         writeFile(const std::string& data, const char *filepath, bool override=true);

		// Read the file to string and return it
		static std::string  readFile(const char *filepath);		

		// The function creates all intermediate directories in a given file path.
		static void         createPath(const char* path, mode_t mode = 0700);

		// The function creates single directory given as dir
		static void         createDir(const char* dir, mode_t mode = 0700);

		static bool         isFile(const char *path);
		static bool         isDir(const char *path);	
		static bool         pathExists(const char *path);	
	
	private:
		File(void) {}

	};
}

