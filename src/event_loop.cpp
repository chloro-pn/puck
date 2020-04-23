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
  stop();
}

void EventLoop::stop() {
  base_loop_.stop();
  for(auto& each : loops_) {
    each->stop();
  }
  thread_pool_.stop();
  logger()->info("EventLoop stop.");
}

void EventLoop::add(TcpConnection* ptr) {
  base_loop_.add(ptr);
}

void EventLoop::new_connection_callback(Poller* loop, TcpConnection* ptr) {
  int clientfd = ptr->fd();
  std::string key = sockets::get_tcp_iport(clientfd);
  ptr->set_iport(key);
  ptr->onConnection();
  if(ptr->shouldClose()) {
    base_loop_.clean(ptr);
  }
  else {
    loop->add(ptr);
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
