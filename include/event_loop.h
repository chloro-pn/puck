#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "poller.h"
#include "../include/util/thread_pool.h"
#include <vector>
#include <sys/epoll.h>
#include <memory>

namespace puck {
class EventLoop {
private:
  Poller base_loop_;
  std::vector<std::unique_ptr<Poller>> loops_;
  tool::ThreadPool thread_pool_;

  void new_connection_callback(Poller* loop, TcpConnection* ptr);

public:
  explicit EventLoop(int n);

  ~EventLoop();

  void add(int fd, epoll_event* ev);

  void newConnection(TcpConnection* ptr);

  int getLoopIndex();

  void start() {
    base_loop_.loop();
  }
};
}

#endif // EVENT_LOOP_H
