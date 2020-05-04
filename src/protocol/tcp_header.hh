#pragma once

#include <arpa/inet.h>

#include <cinttypes>
#include <cstring>
#include <sstream>
#include <string>

#include "udp_header.hh"

using std::ostringstream;
using std::string;

class TcpHeader {
 public:
  uint16_t size;
  string payload{};

  // Default constructor
  TcpHeader();

  // Construct commands and answers
  TcpHeader(string message);

  // Construct publications
  TcpHeader(UdpHeader udp_message, sockaddr_in publisher);

 private:
  enum PublicationType { INT, SHORT_REAL, FLOAT, STRING };
};
