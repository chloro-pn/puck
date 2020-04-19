#include "../include/event_loop.h"
#include "../include/sockets.h"
#include "../include/tcp_connection.h"
#include <unistd.h>

namespace puck {
EventLoop::EventLoop(int n) {
  if(n > 0) {
    for(int i = 0; i < n; ++i) {
      loops_.emplace_back(new Poller());
    }
    for(auto& each : loops_) {
      Poller* loop = each.get();
      thread_pool_.push_task([loop]()->void {
                               loop->loop();
                             });
    }
    thread_pool_.start(n);
  }
}

EventLoop::~EventLoop() {
  base_loop_.stop();
  for(auto& each : loops_) {
    each->stop();
  }
  thread_pool_.stop();
}

void EventLoop::add(int fd, epoll_event* ev) {
  base_loop_.add(fd, ev);
}

void EventLoop::new_connection_callback(Poller* loop, TcpConnection* ptr) {
  int clientfd = ptr->fd();
  std::string key = sockets::get_tcp_iport(clientfd);
  ptr->set_iport(key);
  ptr->onConnection();
  if(ptr->shouldClose()) {
    ptr->onClose();
    delete ptr;
    int n = ::close(clientfd);
    if(n == -1) {
      logger()->fatal(piece("close error : ", strerror(errno)));
    }
  }
  else {
    epoll_event ev;
    ev.data.ptr = ptr;
    ev.events = ptr->events();
    loop->add(clientfd, &ev);
  }
}

int EventLoop::getLoopIndex() {
  static int index = 0;
  int result = index % loops_.size();
  ++index;
  return result;
}

void EventLoop::newConnection(TcpConnection* ptr) {
  Poller* poll = loops_[getLoopIndex()].get();
  poll->push_func([poll, this, ptr]()->void {
                   this->new_connection_callback(poll, ptr);
                 });
  poll->wake_up();
}
}
