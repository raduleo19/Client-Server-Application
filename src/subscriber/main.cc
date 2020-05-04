#include "subscriber.hh"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./subscriber <ID_Client> <IP_Server> <Port_Server>.\n";
    exit(EXIT_FAILURE);
  }

  if (strlen(argv[1]) > 10) {
    std::cerr << "<ID_Client> can have maximum 10 digits.";
    exit(EXIT_FAILURE);
  }

  int port = atoi(argv[3]);
  if(port < 1024 || port > 65535 ) {
    std::cerr << "Invalid port. Must be in range 1024-65535\n";
    exit(EXIT_FAILURE);
  }

  Subscriber subscriber(port, argv[2], argv[1]);
  subscriber.Run();

  return 0;
}