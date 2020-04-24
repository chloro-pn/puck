#ifndef SOCKETS_H
#define SOCKETS_H

#include <string>

namespace puck {
class sockets {
public:
  static int get_nonblock_socket();

  static void set_nonblock(int fd);

  static void no_delay(int fd);

  static void reuse_addr(int fd);

  static uint16_t hostToNetwork(uint16_t n);

  static uint16_t networkToHost(uint16_t n);

  static uint32_t hostToNetwork(uint32_t n);

  static uint32_t networkToHost(uint32_t n);

  static std::string get_tcp_iport(int fd);

  static std::string get_local_addr(int fd);

  static std::string get_peer_addr(int fd);
};
}
#endif // SOCKETS_H
