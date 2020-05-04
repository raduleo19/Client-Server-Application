#include "tcp_header.hh"

TcpHeader::TcpHeader() { size = 0; }

TcpHeader::TcpHeader(string message) : payload(message) { size = message.size() + 3; }

// Parse the publication("Render")
TcpHeader::TcpHeader(UdpHeader udp_message, sockaddr_in publisher) {
  ostringstream payload_builder(payload);
  payload_builder << inet_ntoa(publisher.sin_addr) << ':'
                  << ntohs(publisher.sin_port) << " - ";
  for (int i = 0; i < MAX_TOPIC && udp_message.topic_name[i]; ++i) {
    payload_builder << udp_message.topic_name[i];
  }
  payload_builder << " - ";

  switch (udp_message.type) {
    case INT: {
      payload_builder << "INT - ";
      uint8_t sign = udp_message.payload[0];
      uint32_t num;
      memcpy(&num, udp_message.payload + 1, 4);
      if (sign) {
        payload_builder << '-';
      }
      payload_builder << ntohl(num);
      break;
    }
    case SHORT_REAL: {
      payload_builder << "SHORT_REAL - ";
      uint16_t abs;
      memcpy(&abs, udp_message.payload, 2);
      abs = ntohs(abs);
      if (abs % 100) {
        payload_builder << abs / 100 << '.' << (abs % 100) / 10
                        << (abs % 100) % 10;
      } else {
        payload_builder << abs / 100;
      }
      break;
    }
    case FLOAT: {
      payload_builder << "FLOAT - ";
      uint8_t sign = udp_message.payload[0];
      uint32_t abs;
      memcpy(&abs, udp_message.payload + 1, 4);
      abs = ntohl(abs);
      uint8_t exp = udp_message.payload[5];

      int num_length = 0;
      for (int num = abs; num; num /= 10, num_length++)
        ;
      if (sign) {
        payload_builder << '-';
      }
      if (exp) {
        if (exp > num_length) {
          payload_builder << "0.";
          int zeros = exp - num_length;
          for (int i = 0; i < zeros; ++i) {
            payload_builder << '0';
          }
          payload_builder << abs;
        } else {
          string result = std::to_string(abs);
          result.insert(num_length - exp, ".");
          payload_builder << result;
        }
      } else {
        payload_builder << abs;
      }
      break;
    }
    case STRING: {
      payload_builder << "STRING - ";
      payload_builder << udp_message.payload;
      break;
    }
  }

  payload = payload_builder.str();
  size = payload.size() + 3;
}