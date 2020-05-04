#include "server.hh"

Server::Server(in_addr_t address, int port) {
  FD_ZERO(&descriptors);
  FD_SET(STDIN_FILENO, &descriptors);
  max_fd = STDIN_FILENO;

  BindTcp(port, address);
  BindUdp(port, address);
}

Server::~Server() {
  for (int i = 1; i <= max_fd; ++i) {
    if (FD_ISSET(i, &descriptors)) {
      close(i);
    }
  }
}

void Server::Run() {
  forever {
    auto ready_descriptors = GetReadyDescriptors(descriptors, max_fd);
    for (int current_socket = 0; current_socket <= max_fd; ++current_socket) {
      if (FD_ISSET(current_socket, &ready_descriptors)) {
        if (current_socket == STDIN_FILENO) {
          string command;
          cin >> command;
          if (command == "exit") {
            return;
          } else {
            cerr << "Invalid command. Commands: exit.\n";
          }
        } else if (current_socket == udp_socket) {
          HandleUdp();
        } else if (current_socket == tcp_socket) {
          HandleTcp();
        } else {
          HandleClient(current_socket);
        }
      }
    }
  }
}

void Server::DIE(bool assertion, string err_message) {
  if (assertion) {
    cerr << err_message << '\n';
    exit(EXIT_FAILURE);
  }
}

void Server::BindUdp(int port, in_addr_t address) {
  udp_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  DIE(udp_socket < 0, "Unable to create UDP socket.");
  ReuseSocket(udp_socket);

  sockaddr_in udp_addr;
  udp_addr.sin_family = AF_INET;
  udp_addr.sin_port = htons(port);
  udp_addr.sin_addr.s_addr = address;

  DIE(bind(udp_socket, (sockaddr *)&udp_addr, sizeof(sockaddr_in)) < 0,
      "Error binding UDP socket.");

  FD_SET(udp_socket, &descriptors);
  max_fd = max(max_fd, udp_socket);
}

void Server::BindTcp(int port, in_addr_t address) {
  tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
  DIE(tcp_socket < 0, "Unable to create TCP socket.");
  ReuseSocket(tcp_socket);

  sockaddr_in tcp_addr;
  tcp_addr.sin_family = AF_INET;
  tcp_addr.sin_port = htons(port);
  tcp_addr.sin_addr.s_addr = address;

  DIE(bind(tcp_socket, (sockaddr *)&tcp_addr, sizeof(sockaddr_in)) < 0,
      "Error binding UDP socket.");

  DIE(listen(tcp_socket, MAX_LISTENING_QUEUE) < 0,
      "Error listening to socket.");

  FD_SET(tcp_socket, &descriptors);
  max_fd = max(max_fd, tcp_socket);
}

void Server::HandleUdp() {
  UdpHeader buffer{};
  socklen_t udp_socklen = sizeof(sockaddr);
  sockaddr_in udp_addr;
  int num_bytes = recvfrom(udp_socket, &buffer, sizeof(buffer), 0,
                           (sockaddr *)&udp_addr, &udp_socklen);
  if (num_bytes < 0) {
    cerr << "Error receiving message from UDP socket.\n";
  } else {
    // Add terminator
    *(((char *)&buffer) + num_bytes) = 0;
    ostringstream topic_name;
    for (int i = 0; i < MAX_TOPIC && buffer.topic_name[i]; ++i) {
      topic_name << buffer.topic_name[i];
    }
    Publish(topic_name.str(), TcpHeader(buffer, udp_addr));
  }
}

void Server::HandleTcp() {
  sockaddr_in new_tcp_sockaddr;
  socklen_t new_tcp_socklen = sizeof(sockaddr);
  int new_tcp_socket =
      accept(tcp_socket, (sockaddr *)&new_tcp_sockaddr, &new_tcp_socklen);

  if (new_tcp_socket < 0) {
    cerr << "Unable to accept client.\n";
  } else {
    DisableNagle(new_tcp_socket);
    FD_SET(new_tcp_socket, &descriptors);
    max_fd = max(max_fd, new_tcp_socket);
    socket_to_broker[new_tcp_socket] = TcpBroker(new_tcp_socket);
    socket_to_addr[new_tcp_socket] = new_tcp_sockaddr;
  }
}

void Server::HandleClient(int socket_fd) {
  TcpBroker &broker = socket_to_broker[socket_fd];
  int status = broker.Advance();
  if (status == -1) {
    Disconnect(socket_fd);
  } else if (status == 1) {
    TcpHeader message = broker.GetMessage();
    if (socket_to_id.find(socket_fd) != socket_to_id.end()) {
      string command(message.payload);
      auto error = Validate(command);
      if (error.empty()) {
        ApplyCommand(socket_fd, command);
      } else {
        // Send errors to clients for invalid commands(maybe malicious)
        TcpHeader error_message(string("[SERVER] ") + error);
        broker.PutMessage(error_message);
      }
    } else {
      string id = string(message.payload);
      if (not id.empty() && id.size() <= 10) {
        RegisterId(socket_fd, id);
      } else {
        // Send errors to clients for invalid commands(maybe malicious)
        TcpHeader error_message(string("[SERVER] ") + "Invalid Id");
        broker.PutMessage(error_message);
        Disconnect(socket_fd);
      }
    }
  }
}

void Server::RegisterId(int socket_fd, string id) {
  auto subscriber = id_to_subscriber.find(id);
  if (subscriber != id_to_subscriber.end() &&
      subscriber->second.socket_fd != -1) {
    cerr << "Connection refused, subscriber " << id << " already online.\n";
    close(socket_fd);
  } else {
    Connect(socket_fd, id);
  }
}

void Server::ApplyCommand(int socket_fd, string command) {
  istringstream command_stream(command);
  string command_type;
  string topic;
  command_stream >> command_type >> topic;
  if (command_type == "subscribe") {
    int flag;
    command_stream >> flag;
    Subscribe(socket_fd, topic, flag);
  } else if (command_type == "unsubscribe") {
    Unsubscribe(socket_fd, topic);
  }
}

void Server::Subscribe(int socket_fd, string topic, int flag) {
  string subscriber_id = socket_to_id[socket_fd];
  if (topic_to_subscribers[topic].find(subscriber_id) ==
      topic_to_subscribers[topic].end()) {
    topic_to_subscribers[topic][subscriber_id] = flag;
    id_to_subscriber[subscriber_id].num_subscribed_topics++;
    TcpHeader response("subscribed " + topic);
    socket_to_broker[socket_fd].PutMessage(response);
  } else {
    TcpHeader response("fail, already subscribing " + topic);
    socket_to_broker[socket_fd].PutMessage(response);
  }
}

void Server::Unsubscribe(int socket_fd, string topic) {
  string subscriber_id = socket_to_id[socket_fd];
  if (topic_to_subscribers[topic].find(subscriber_id) !=
      topic_to_subscribers[topic].end()) {
    id_to_subscriber[subscriber_id].num_subscribed_topics--;
    topic_to_subscribers[topic].erase(subscriber_id);
    TcpHeader response("unsubscribed " + topic);
    socket_to_broker[socket_fd].PutMessage(response);
  } else {
    TcpHeader response("fail, you are not subscribing " + topic);
    socket_to_broker[socket_fd].PutMessage(response);
  }
}

void Server::Connect(int socket_fd, string id) {
  auto connection_info = socket_to_addr[socket_fd];
  cout << "New client " << id << " connected from "
       << inet_ntoa(connection_info.sin_addr) << ":"
       << ntohs(connection_info.sin_port) << '\n';
  socket_to_id[socket_fd] = id;
  id_to_subscriber[id].socket_fd = socket_fd;
  SendPendingMessages(socket_fd, id);
}

void Server::SendPendingMessages(int socket_fd, string id) {
  auto &subscriber = id_to_subscriber[id];
  for (auto message : subscriber.pending_messages) {
    socket_to_broker[socket_fd].PutMessage(message->message);
    if ((--message->num_offline_subscribers) == 0) {
      messages.erase(message);
    }
  }
  subscriber.pending_messages.clear();
}

void Server::Disconnect(int socket_fd) {
  close(socket_fd);
  cout << "Client " << socket_to_id[socket_fd] << " disconnected.\n";
  FD_CLR(socket_fd, &descriptors);
  if (max_fd == socket_fd) {
    for (int i = max_fd - 1; i >= 0; ++i) {
      if (FD_ISSET(i, &descriptors)) {
        max_fd = i;
        break;
      }
    }
  }

  string subscriber_id = socket_to_id[socket_fd];
  if (id_to_subscriber[subscriber_id].num_subscribed_topics == 0) {
    id_to_subscriber.erase(subscriber_id);
  } else {
    id_to_subscriber[subscriber_id].socket_fd = -1;
  }
  socket_to_id.erase(socket_fd);
  socket_to_broker.erase(socket_fd);
  socket_to_addr.erase(socket_fd);
}

void Server::Publish(string topic, TcpHeader message) {
  auto subscribers = topic_to_subscribers.find(topic);
  if (subscribers != topic_to_subscribers.end()) {
    bool added = false;
    for (auto it : subscribers->second) {
      if (id_to_subscriber[it.first].socket_fd != -1) {
        socket_to_broker[id_to_subscriber[it.first].socket_fd].PutMessage(
            message);
      } else {
        if (it.second) {
          if (not added) {
            added = true;
            messages.push_back({1, message});
          } else {
            messages.back().num_offline_subscribers++;
          }
          id_to_subscriber[it.first].pending_messages.push_back(
              --messages.end());
        }
      }
    }
  }
}