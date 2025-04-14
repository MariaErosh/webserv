#pragma once

#include <string>
#include "../http/http.hpp"

class PageRenderer {
	private:
		PageRenderer();
		~PageRenderer();
		PageRenderer(const PageRenderer& other);
		PageRenderer& operator=(const PageRenderer& other);

		static const std::string  default_page_file_;
		static const std::string  error_pages_directory_;

	public:
		// Generates the default page
		static std::string  renderDefaultPage(void);

		// Generates an index page for a directory
		static std::string  renderDirectoryIndex(const std::string& directoryPath, std::string requestUri);

		// Generates a default error page with a status code
		static std::string  renderErrorPage(StatusCode statusCode);

		// Generates an error page using a custom HTML file
		static std::string  renderErrorPage(const char* customErrorPagePath);
	};
