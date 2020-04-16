#include "../include/poller.h"
#include "../include/tcp_connection.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/errno.h>
#include <cstring>
#include <vector>

namespace puck {
Poller::Poller():epfd_(-1) {
  epfd_ = epoll_create(1); //the parameter has no means.
  if(epfd_ == -1) {
    logger()->fatal("epoll create error.");
  }
}

Poller::~Poller() {
  int n = ::close(epfd_);
  if(n == -1) {
    logger()->fatal("epoll close error.");
  }
}

void Poller::clean(TcpConnection *ptr) {
  int fd = ptr->fd();
  epoll_ctl(epfd_, EPOLL_CTL_DEL,ptr->fd(), nullptr);
  logger()->info(piece("fd ", fd, " close. state : ", ptr->getState()));
  int n = close(fd);
  if(n == -1) {
    logger()->fatal(piece("close error. errno : ", strerror(errno)));
  }
  conns_.erase(ptr->iport());
  delete ptr;
}

void Poller::change(TcpConnection *ptr) {
  epoll_event ev;
  ev.events = ptr->events();
  ev.data.ptr = ptr;
  epoll_ctl(epfd_, EPOLL_CTL_MOD, ptr->fd(), &ev);
}

void Poller::loop() {
  while(true) {
    std::vector<epoll_event> evs;
    evs.resize(1024);
    int ready = epoll_wait(epfd_, &*evs.begin(), evs.size(), -1);
    if(epfd_ == -1) {
      logger()->fatal("epoll create error.");
    }
    for(int i = 0; i < ready; ++i) {
      TcpConnection* ptr = (TcpConnection*)evs[i].data.ptr;
      if(ptr->isListenSocket() == true) {
        ptr->onAccept();
        continue;
      }
      ptr->handle(evs[i].events);
      if(ptr->shouldClose()) {
        clean(ptr);
      }
      else if(ptr->eventsChange() == true){
        change(ptr);
      }
    }
  }
}
}
