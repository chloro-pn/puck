#include "../include/tcp_connection.h"
#include "../include/codec.h"
#include <unistd.h>
#include <sys/socket.h> // shutdown

namespace puck {

TcpConnection::TcpConnection(int fd, int events, Codec* codec):
                             read_buf_(1024*8),
                             write_buf_(1024 * 8),
                             fd_(fd),
                             events_(events),
                             old_events_(events),
                             state_(connState::go_on),
                             error_value_(0),
                             writing_(false),
                             want_shutdown_wr_(false),
                             really_shutdown_wr_(false),
                             read_zero_(false),
                             listen_socket_(false),
                             event_fd_(false),
                             alive_(true),
                             heart_beat_(false),
                             each_ms_(-1),
                             conning_(false),
                             codec_(codec),
                             added_to_poller_(false) {

}

TcpConnection::~TcpConnection() {
  delete codec_;
}

void TcpConnection::send(const char* ptr, size_t n) {
  std::unique_lock<std::mutex> mut(mut_);
  if(want_shutdown_wr_ == true) {
    return;
  }
  if(codec_ == nullptr) {
    write_buf_.push(ptr, n);
  }
  else {
    std::string ret = codec_->encode(ptr, n);
    write_buf_.push(ret.data(), ret.size());
  }
  if(writing_ == false) {
    events_ = events_ | EPOLLOUT;
    writing_ = true;
  }
}

const char* TcpConnection::getStateStr() {
  if(state_ == connState::go_on) {
    return "go_on";
  }
  else if(state_ == connState::read_error) {
    return "read error";
  }
  else if(state_ == connState::succ_close) {
    return "succ close";
  }
  else if(state_ == connState::force_colse) {
    return "force close";
  }
  else if(state_ == connState::write_error) {
    return "write error";
  }
  else if(state_ == connState::shutdown_error) {
    return "shutdown error";
  }
  else if(state_ == connState::out_of_data) {
    return "out of date";
  }
  else if(state_ == connState::codec_error) {
    return "codec error";
  }
  else {
    return "unknow error";
  }
}

void TcpConnection::handle(int events) {
  if(events & EPOLLIN) {
    alive_ = true;
    if(read_buf_.unusedSize() == 0) {
      logger()->warning("too many messages storaged in the read buf.");
      onMessage();
      if(shouldClose()) {
        return;
      }
    read_buf_.tryMoveToForward();
    }
    else {
    int n = -1;
    n = ::read(fd_, read_buf_.availableArea(), read_buf_.unusedSize());
    if(n > 0) {
      read_buf_.endMove(n);
      onMessage();
      if(shouldClose()) {
        return;
      }
    }
    else if(n < 0) {
      if(errno != EWOULDBLOCK) {
        setState(connState::read_error);
        error_value_ = errno;
        return;
      }
    }
    else { // n == 0.
      read_zero_ = true;
      onConnection();
      if(shouldClose()) {
        return;
      }
      if(really_shutdown_wr_ == true) {
        setState(connState::succ_close);
        return;
      }
      events_ = events_ & (~EPOLLIN);
    }
    }
  }

  if(events & EPOLLOUT) {
    int n = ::write(fd_, write_buf_.data(), write_buf_.usedSize());
    if (n == -1) {
      if(errno != EWOULDBLOCK) {
        setState(connState::write_error);
        error_value_ = errno;
        return;
      }
    }
    else {
      write_buf_.abandon(n);
    }
    if (write_buf_.usedSize() == 0) {
      if(want_shutdown_wr_ == true ) {
        int err = ::shutdown(fd_, SHUT_WR);
        if(err == -1) {
          setState(connState::shutdown_error);
          error_value_ = errno;
          return;
        }
        really_shutdown_wr_ = true;
        if(read_zero_ == true) {
          setState(connState::succ_close);
          return;
        }
      }
      events_ = events_ & (~EPOLLOUT);
      writing_ = false;
      onWriteComplete();
      if(shouldClose()) {
        return;
      }
    }
  }
}
}
