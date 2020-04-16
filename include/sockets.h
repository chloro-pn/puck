#ifndef SOCKETS_H
#define SOCKETS_H

#include <string>

namespace puck {
class sockets {
public:
  static int get_nonblock_socket();

  static void set_nonblock(int fd);

  static std::string get_tcp_iport(int fd);
};
}
#endif // SOCKETS_H
