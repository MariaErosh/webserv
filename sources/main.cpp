#include "../headers/core/server.hpp"
#include "../headers/utils/logger.hpp"

int main(int argc, const char *argv[]) {
  const char *config_path = "resources/configs/default.conf"; 
  if (argc >= 2)
    config_path = argv[1];

  try {
    Logger::init(Logger::LOGLEV_INFO, true);
    Server& server = Server::instance_;

    server.init(config_path);
    server.run();
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
