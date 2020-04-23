#include "../include/poller.h"
#include "../include/tcp_connection.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/eventfd.h>
#include <cstring>
#include <vector>
#include <sstream>

namespace puck {
Poller::Poller():epfd_(-1), eventfd_(-1), mut_(new std::mutex()),stop_(false) {
  epfd_ = ::epoll_create(1); //the parameter has no means.
  if(epfd_ == -1) {
    logger()->fatal(piece("epoll create error : ", strerror(errno)));
  }

  TcpConnection* ptr = createEventConnection();
  add(ptr);
}

Poller::~Poller() {
  int n = ::close(eventfd_);
  if(n == -1) {
    logger()->fatal("eventfd close error.");
  }
  for(auto& each : conns_) {
    each.second->setState(TcpConnection::connState::force_colse);
    clean(each.second);
  }
  n = ::close(epfd_);
  if(n == -1) {
    logger()->fatal("epoll close error.");
  }
}

void Poller::run_after(uint32_t ms, const task_type& task) {
  time_point<system_clock, milliseconds> tp = time_point_cast<milliseconds>(system_clock::now()) + milliseconds(ms);
  time_event te(tp, milliseconds(ms), task);
  timers_.push(te);
}

void Poller::add(TcpConnection* ptr) {
  epoll_event ev;
  ev.events = ptr->events();
  ev.data.ptr = ptr;

  ptr->added_to_poller_ = true;
  int n = ::epoll_ctl(epfd_, EPOLL_CTL_ADD, ptr->fd(), &ev);
  if(n == -1) {
    logger()->fatal("epoll add error.");
  }

  if(ptr->isListenSocket() || ptr->isEventFd()) {
    return;
  }
  std::string key = ptr->iport();
  if(conns_.find(key) != conns_.end()) {
    logger()->fatal(piece("repeated iport : ", key));
  }
  conns_[key] = ptr;

  //std::ostringstream ss;
  //ss << std::this_thread::get_id();
  //logger()->info(piece("new connection on thread : ", ss.str()));
  if(ptr->isOpenHeartBeat()) {
    run_after(ptr->each_ms_, [this, key]()->bool {
      return handle_heart_beat(key);
    });
  }
}

void Poller::clean(TcpConnection *ptr) {
  int fd = ptr->fd();
  if(ptr->added_to_poller_ == true) {
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL,ptr->fd(), nullptr);
    if(ret != 0) {
      logger()->fatal(piece("epoll ctl error : ", strerror(errno)));
    }
  }
  if(!(ptr->isEventFd()) && !(ptr->isListenSocket())) {
    ptr->onClose();
  }
  if(ptr->getState() != TcpConnection::connState::succ_close) {
    logger()->warning(piece("fd ", fd, " close. state : ", ptr->getStateStr()));
  }
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

int Poller::get_latest_time() {
  time_point<system_clock, milliseconds> now = time_point_cast<milliseconds>(system_clock::now());
  while(timers_.empty() == false) {
    if(now >= timers_.top().time_point_) {
      bool conti = timers_.top().task_();
      auto task = std::move(timers_.top());
      timers_.pop();
      if(conti == true) {
        task.time_point_ = now + task.ms_;
        timers_.push(std::move(task));
      }
    }
    else {
      milliseconds ms = timers_.top().time_point_ - now;
      return ms.count();
    }
  }
  return -1;
}

void Poller::handle_timer_events() {
  time_point<system_clock, milliseconds> now = time_point_cast<milliseconds>(system_clock::now());
  while(timers_.empty() == false) {
    if(now >= timers_.top().time_point_) {
      bool conti = timers_.top().task_();
      auto task = std::move(timers_.top());
      timers_.pop();
      if(conti == true) {
        task.time_point_ = now + task.ms_;
        timers_.push(std::move(task));
      }
    }
    else {
      return;
    }
  }
}

TcpConnection* Poller::createEventConnection() {
  eventfd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if(eventfd_ == -1) {
    logger()->fatal(piece("eventfd error : ", strerror(errno)));
  }

  TcpConnection* ptr = new TcpConnection(eventfd_, EPOLLIN);
  ptr->setEventFdFlag();
  return ptr;
}

void Poller::wake_up() {
  uint64_t u = 2;
  ssize_t n = ::write(eventfd_, &u, sizeof(u));
  if(n != sizeof(u)) {
    logger()->fatal(piece("eventfd write error : ", strerror(errno)));
  }
}

void Poller::handle_wake_up() {
  uint64_t u;
  ssize_t n = ::read(eventfd_, &u, sizeof(u));
  if(n != sizeof(u)) {
    if(errno != EAGAIN) {
      logger()->fatal(piece("event read error : ", strerror(errno)));
    }
    else {
      return;
    }
  }
}

void Poller::loop() {
  while(stop_ == false) {
    std::vector<epoll_event> evs;
    evs.resize(1024);
    int ms = get_latest_time();
    int ready = epoll_wait(epfd_, &*evs.begin(), evs.size(), ms);
    if(epfd_ == -1) {
      logger()->fatal("epoll create error.");
    }
    for(int i = 0; i < ready; ++i) {
      TcpConnection* ptr = (TcpConnection*)evs[i].data.ptr;
      if(ptr->isListenSocket() == true) {
        ptr->onAccept();
        continue;
      }
      if(ptr->isEventFd() == true) {
        handle_wake_up();
        continue;
      }
      if(ptr->isConning() == true) {
        ptr->setConnectedFlag();
        ptr->events_ = EPOLLIN;
        int init_event = ptr->events();
        ptr->onSuccConn();
        if(ptr->events() != init_event) {
          ptr->events_change_ = true;
        }
      }
      else {
        ptr->handle(evs[i].events);
      }
      if(ptr->shouldClose()) {
        clean(ptr);
      }
      else if(ptr->eventsChange() == true){
        change(ptr);
      }
    }
    handle_timer_events();
    handle_funcs();
  }
}

void Poller::push_func(const std::function<void()>& func) {
  std::unique_lock<std::mutex> mut(*mut_);
  funcs_.push_back(func);
}

void Poller::handle_funcs() {
  std::vector<std::function<void()>> tmp;
  std::unique_lock<std::mutex> mut(*mut_);
  tmp.swap(funcs_);
  mut.unlock();
  for(auto& each : tmp) {
    each();
  }
}

bool Poller::handle_heart_beat(std::string key) {
  if(this->conns_.find(key) == this->conns_.end()) {
    logger()->info(piece("connection : ", key, " has been closed."));
    return false;
  }
  TcpConnection* ptr = conns_[key];
  if(ptr->alive_ == false) {
    logger()->info(piece("fd : ", ptr->fd(), key, " out of date."));
    ptr->setState(TcpConnection::connState::out_of_data);
    clean(ptr);
    return false;
  }
  else {
    ptr->alive_ = false;
  }
  return true;
}
}
