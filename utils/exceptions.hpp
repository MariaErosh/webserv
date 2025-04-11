#pragma once

#include <exception>

    class Exceptions {
		public:
			struct	FileDoesNotExist: public std::exception {
				virtual const char  *what() const throw();
			};
		};
