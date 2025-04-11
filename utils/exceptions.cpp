#include "./exceptions.hpp"


    const char*     Exceptions::FileDoesNotExist::what() const throw() {
        return "Exception thrown: file doesn't exist";
    }

