#ifndef POLLER_H
#define POLLER_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <map>
#include "console.h"
#include "tcp_connection.h"

namespace puck {
class Poller {
private:
  int epfd_;
  std::map<std::string, TcpConnection*> conns_;

public:
  Poller();

  ~Poller();

  void add(int fd, epoll_event* ev) {
    int n = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, ev);
    if(n == -1) {
      logger()->fatal("epoll add error.");
    }
    TcpConnection* ptr = static_cast<TcpConnection*>(ev->data.ptr);
    if(ptr->isListenSocket()) {
      return;
    }
    std::string key = ptr->iport();
    if(conns_.find(key) != conns_.end()) {
      logger()->fatal(piece("repeated iport : ", key));
    }
    conns_[key] = ptr;
  }

  void loop();

  void clean(TcpConnection* ptr);

  void change(TcpConnection* ptr);
};
}

#endif // POLLER_H
