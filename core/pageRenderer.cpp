#include "./pageRenderer.hpp"
#include "../utils/string.hpp"
#include "../utils/file.hpp"
#include "../utils/logger.hpp"
#include <vector>
#include <dirent.h>
#include <sstream>
#include <stdexcept>


		PageRenderer::PageRenderer() {}
		PageRenderer::~PageRenderer() {}
		PageRenderer::PageRenderer(const PageRenderer& other) {(void)other;}
		PageRenderer& PageRenderer::operator=(const PageRenderer& other) {(void)other; return *this;}

		const std::string PageRenderer::error_pages_directory_    = "resources/default_pages/errors/";
		const std::string PageRenderer::default_page_file_  = "resources/default_pages/default_page.html";

		std::string   PageRenderer::renderErrorPage(StatusCode statusCode) {
			Logger::debug("PageRenderer::renderErrorPage");

			return File::readFile((PageRenderer::error_pages_directory_
										+ String::convertToString(static_cast<int>(statusCode))
										+ ".html").c_str());
		}

		std::string   PageRenderer::renderErrorPage(const char* customErrorPagePath){
			Logger::debug("PageRenderer::renderErrorPage");
			return File::readFile(customErrorPagePath);
		}

		std::string  PageRenderer::renderDefaultPage() {
			Logger::debug("PageRenderer::renderDefaultPage");
			return File::readFile(PageRenderer::default_page_file_.c_str());
		}


		std::string  PageRenderer::renderDirectoryIndex(const std::string& directoryPath,
														std::string requestUri) {
			Logger::debug("PageRenderer::renderDirectoryIndex");
			Logger::debug("Index absolute_path : " + directoryPath);
			Logger::debug("Index request uri   : " + requestUri);

			if (requestUri[requestUri.size() - 1] != '/')
				requestUri.push_back('/');

			DIR *dir;
			dir = opendir(directoryPath.c_str());
			if (!dir) throw std::invalid_argument("renderDirectoryIndex exception: directory does not exists");

			std::stringstream ss;
			ss << "<!DOCTYPE html>\n"
					"<html>\n"
					"<style>\n"
					"html { color-scheme: light dark; }\n"
					"body { width: 35em; margin: 0 auto;\n"
					"font-family: Tahoma, Verdana, Arial, sans-serif; }\n"
					"</style>\n"
				"<title> Index of " << requestUri << "</title>\n"
				"<body>\n"
				"<div>\n";


			ss << "<h1>Index of " << requestUri << "</h1>\n";
				ss << "<hr>\n<pre>\n";

			for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir)) {
				ss << "<p><a href=\"" << requestUri + dirEntry->d_name << "\">";

				ss << dirEntry->d_name;
				if (dirEntry->d_type & DT_DIR)
					ss << "/";
				ss << "</a></p>\n";
			}
			closedir(dir);
			ss << "</pre>\n</hr>\n</div>\n</body>\n</html>";

			return ss.str();
		}

