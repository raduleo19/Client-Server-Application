#include "subscriber.hh"

Subscriber::Subscriber(int port, char *address, char *id) {
  FD_ZERO(&descriptors);
  server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (server_socket_fd < 0) {
    cerr << "Unable to create server socket.";
    exit(EXIT_FAILURE);
  }

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_aton(address, &server_addr.sin_addr) == 0) {
    cerr << "Invalid IP.\n";
    close(server_socket_fd);
    exit(EXIT_FAILURE);
  }

  int connection_status =
      connect(server_socket_fd, (sockaddr *)&server_addr, sizeof(server_addr));
  if (connection_status < 0) {
    cerr << "Connection error.\n";
    close(server_socket_fd);
    exit(EXIT_FAILURE);
  }
  DisableNagle(server_socket_fd);

  FD_SET(STDIN_FILENO, &descriptors);
  FD_SET(server_socket_fd, &descriptors);
  tcp_broker = TcpBroker(server_socket_fd);

  TcpHeader id_message = TcpHeader(string(id));
  tcp_broker.PutMessage(id_message);
}

Subscriber::~Subscriber() { shutdown(server_socket_fd, SHUT_RD); }

void Subscriber::Run() {
  forever {
    auto ready_descriptors =
        GetReadyDescriptors(descriptors, server_socket_fd);
    if (FD_ISSET(STDIN_FILENO, &ready_descriptors)) {
      string command;
      getline(cin, command);
      string error = Validate(command);
      if (error.empty()) {
        TcpHeader message(command);
        if (command == "exit") {
          shutdown(server_socket_fd, SHUT_WR);
        } else {
          tcp_broker.PutMessage(message);
        }
      } else {
        cout << error << '\n';
      }
    }
    if (FD_ISSET(server_socket_fd, &ready_descriptors)) {
      int status = tcp_broker.Advance();
      if (status == 1) {
        cout << tcp_broker.GetMessage().payload << '\n';
      } else if (status == -1) {
        break;
      }
    }
  }
}
