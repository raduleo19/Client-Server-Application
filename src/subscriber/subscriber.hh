#pragma once
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "../protocol/tcp_broker.hh"
#include "../protocol/tcp_header.hh"
#include "../utils/utils.hh"
#include "../validator/validator.hh"

using namespace std;

class Subscriber {
 public:
  Subscriber(int port, char *address, char *id);
  
  ~Subscriber();

  void Run();

 private:
  TcpBroker tcp_broker;
  fd_set descriptors;
  int server_socket_fd;
};