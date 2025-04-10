#pragma once
#include "./server.hpp"

namespace Config {
  struct Config {
    std::vector<ServerConfig> server_list;
  }; 
}

