#include "utils.hh"

fd_set GetReadyDescriptors(fd_set descriptors, int max_fd) {
  auto ready_descriptors = descriptors;
  int result =
      select(max_fd + 1, &ready_descriptors, nullptr, nullptr, nullptr);
  if (result < 0) {
    std::cerr << "Unable to select descriptors.\n";
    exit(EXIT_FAILURE);
  }
  return ready_descriptors;
}

void DisableNagle(int socket_fd) {
  int flag = 1;
  int result =
      setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
  if (result == -1) {
    std::cerr << "Error disabling nagle.\n";
  }
}

void ReuseSocket(int socket_fd) {
  int enable = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));
}
