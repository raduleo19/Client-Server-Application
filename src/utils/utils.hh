#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/tcp.h>

#include <iostream>
#include <string>

#define forever for (;;)
#define MAX_LISTENING_QUEUE 1024

fd_set GetReadyDescriptors(fd_set descriptors, int max_fd);

void DisableNagle(int socket_fd);

void ReuseSocket(int socket_fd);
