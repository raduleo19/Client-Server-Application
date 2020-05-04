#include "./server.hh"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: ./server <PORT>\n";
    exit(EXIT_FAILURE);
  }

  int port = atoi(argv[1]);
  if(port < 1024 || port > 65535 ) {
    std::cerr << "Invalid port. Must be in range 1024-65535\n";
    exit(EXIT_FAILURE);
  }

  Server server(INADDR_ANY, port);
  server.Run();

  return 0;
}