#pragma once

#include <exception>

namespace Utils {
    class Exceptions {
		public:
			struct	FileDoesNotExist: public std::exception {
				virtual const char  *what() const throw();
			};
		};
}