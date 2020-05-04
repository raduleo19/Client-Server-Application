#pragma once
#include <netinet/tcp.h>
#include <unistd.h>

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../protocol/tcp_broker.hh"
#include "../protocol/tcp_header.hh"
#include "../protocol/udp_header.hh"
#include "../utils/utils.hh"
#include "../validator/validator.hh"

using namespace std;

class Server {
 public:
  Server(in_addr_t address, int port);

  ~Server();

  void Run();

 private:
  void DIE(bool assertion, string err_message);

  void BindUdp(int port, in_addr_t address);

  void BindTcp(int port, in_addr_t address);

  void HandleUdp();

  void HandleTcp();

  void HandleClient(int socket_fd);

  void RegisterId(int socket_fd, string id);

  void ApplyCommand(int socket_fd, string command);

  void Subscribe(int socket_fd, string topic, int flag);

  void Unsubscribe(int socket_fd, string topic);

  void Connect(int socket_fd, string id); 

  void SendPendingMessages(int socket_fd, string id);

  void Disconnect(int socket_fd);

  void Publish(string topic, TcpHeader message);

  struct Message {
    int num_offline_subscribers;
    TcpHeader message;
  };

  struct Subscriber {
    int socket_fd;
    int num_subscribed_topics;
    vector<list<Message>::iterator> pending_messages;
  };

  list<Message> messages;
  unordered_map<int, sockaddr_in> socket_to_addr;
  unordered_map<int, string> socket_to_id;
  unordered_map<int, TcpBroker> socket_to_broker;
  unordered_map<string, Subscriber> id_to_subscriber;
  unordered_map<string, unordered_map<string, int>> topic_to_subscribers;
  fd_set descriptors;
  int max_fd;
  int udp_socket;
  int tcp_socket;
};
