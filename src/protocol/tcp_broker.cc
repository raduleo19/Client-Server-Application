#include "tcp_broker.hh"

#include <iostream>

// xxx.xxx.xxx.xxx:xxxxx - <50 chars> - STRING - <max 1500 chars>, to be sure we use 1700
#define MAX_BUFFER 1700

TcpBroker::TcpBroker() {
  server_socket = -1;
  bytes_remaining = sizeof(received_message.size);
  bytes_read = 0;
  state = READING_SIZE;
  TcpHeader{};
}

TcpBroker::TcpBroker(int socket) : TcpBroker() { server_socket = socket; }

int TcpBroker::Advance() {
  static char buffer[2000];
  int bytes_received =
      recv(server_socket, buffer + bytes_read, bytes_remaining, 0);
  if (bytes_received == 0) {
    return -1;
  } else if (bytes_received > 0) {
    bytes_read += bytes_received;
    bytes_remaining -= bytes_received;
    if (bytes_remaining == 0) {
      if (state == READING_SIZE) {
        uint16_t size;
        memcpy(&size, buffer, sizeof(size));
        received_message.size = size;
        bytes_remaining = received_message.size - bytes_read;
        state = READING_PAYLOAD;
      } else {
        bytes_read = 0;
        bytes_remaining = sizeof(received_message.size);
        state = READING_SIZE;
        received_message.payload =
            string(buffer + sizeof(received_message.size));
        return 1;
      }
    }
  }

  return 0;
}

TcpHeader TcpBroker::GetMessage() { return received_message; }

void TcpBroker::PutBytes(const char *message, int size) {
  int bytes_remaining = size;
  int bytes_sent = 0;
  while (bytes_remaining) {
    int bytes = send(server_socket, message + bytes_sent, bytes_remaining, 0);
    if (bytes != -1) {
      bytes_sent += bytes;
      bytes_remaining -= bytes;
    } else {
      std::cerr << "Sending error: " << bytes << '\n';
      std::cerr << "Retrying...\n";
    }
  }
}

void TcpBroker::PutMessage(TcpHeader &message) {
  PutBytes((char *)&(message.size), sizeof(message.size));
  PutBytes(message.payload.c_str(), message.size - sizeof(message.size));
}
