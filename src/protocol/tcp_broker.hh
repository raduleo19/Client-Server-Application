#pragma once

#include "./tcp_header.hh"

class TcpBroker {
 public:
  TcpBroker();

  TcpBroker(int socket);

  // Read available bytes
  int Advance();

  TcpHeader GetMessage();

  // Blocking send bytes
  void PutBytes(const char *message, int size);

  // Blocking send message
  void PutMessage(TcpHeader &message);

 private:
  enum ReadState { READING_SIZE, READING_PAYLOAD };
  ReadState state;
  TcpHeader received_message;
  int bytes_remaining;
  int bytes_read;
  int server_socket;
};