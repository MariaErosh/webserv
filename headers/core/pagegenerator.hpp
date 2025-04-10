#pragma once

#include <string>
#include "../http/http.hpp"

namespace Core {

  class PageGenerator {
	private:
		PageGenerator();
		~PageGenerator();
		PageGenerator(const PageGenerator& other);
		PageGenerator& operator=(const PageGenerator& other);

		static const std::string  error_pages_dir_;
		static const std::string  default_page_path_;

	public:
		// This function generates default page
		static std::string  generateDefaultPage(void);

		// This function generates default page
		static std::string  generateIndexPage(const std::string& path, std::string request_uri);

		// This function generates default error page with error code
		static std::string  generateErrorPage(Http::StatusCode status_code);

		// This function generates error page with custom html file
		static std::string  generateErrorPage(const char *error_page_path);
	};
}