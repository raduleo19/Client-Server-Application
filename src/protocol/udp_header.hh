#pragma once

#include <cinttypes>

#define MAX_TOPIC 50
#define MAX_PUBLICATION 1501

struct __attribute__((packed)) UdpHeader {
  char topic_name[MAX_TOPIC];
  uint8_t type;
  char payload[MAX_PUBLICATION];
};