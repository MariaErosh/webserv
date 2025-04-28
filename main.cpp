#include "./core/server.hpp"
#include "./utils/logger.hpp"


int main(int argc, const char *argv[]) {
  const char *configPath = "resources/configs/default.conf";
  if (argc >= 2)
    configPath = argv[1];

  try {
    Logger::initialize(Logger::LOG_INFO, true);
    Server& server = Server::instance_;

    server.configure(configPath);
    server.run();
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
