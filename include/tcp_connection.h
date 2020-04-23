#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "buffer.h"
#include "codec.h"
#include <functional>
#include <sys/epoll.h>
#include <cassert>
#include <string>

namespace puck {
class TcpConnection {
  friend class TcpServer;
  friend class Poller;
  friend class EventLoop;
  friend class Client;
  friend class Codec;

public:
  enum class connState { go_on ,
                         succ_close,
                         read_error,
                         write_error,
                         shutdown_error,
                         force_colse,
                         out_of_data,
                         codec_error };

private:
  using callback_type = std::function<void(TcpConnection*)>;

  int fd() const {
    return fd_;
  }

  TcpConnection(int fd, int events, Codec* codec = nullptr);

  ~TcpConnection();

  bool isListenSocket() const {
    return listen_socket_;
  }

  void setListenSocketFlag() {
    listen_socket_ = true;
  }

  bool isEventFd() const {
    return event_fd_;
  }

  void setEventFdFlag() {
    event_fd_ = true;
  }

  bool isConning() const {
    return conning_;
  }

  void setConnectedFlag() {
    conning_ = false;
  }

  void setConningFlag() {
    conning_ = true;
  }

  int events() const {
    return events_;
  }

  bool isEventsChange() const {
    return events_change_;
  }

  void setState(connState s) {
    state_ = s;
  }

  bool shouldClose() const {
    return state_ != connState::go_on;
  }

  void onConnection() {
    if(on_connection_) {
      on_connection_(this);
    }
  }

  void onMessage() {
    if(on_message_) {
      on_message_(this);
    }
  }

  void onWriteComplete() {
    if(on_write_complete_) {
      on_write_complete_(this);
    }
  }

  void onClose() {
    if(on_close_) {
      on_close_(this);
    }
  }

  void onAccept() {
    assert(listen_socket_ == true);
    on_accept_(this);
  }

  void onSuccConn() {
    on_succ_conn_(this);
  }

  void setOnConnection(callback_type ct) {
    on_connection_ = ct;
  }

  void setOnMessage(callback_type ct) {
    if(codec_ == nullptr) {
      on_message_ = ct;
      return;
    }
    codec_->setOnMessage(ct);
    on_message_ = [this](TcpConnection* ptr)->void {
      this->codec_->onMessage(ptr);
    };
  }

  void setOnWriteComplete(callback_type ct) {
    on_write_complete_ = ct;
  }

  void setOnClose(callback_type ct) {
    on_close_ = ct;
  }

  void setOnAccept(callback_type ct) {
    assert(listen_socket_ == true);
    on_accept_ = ct;
  }

  void setOnSuccConn(callback_type ct) {
    on_succ_conn_ = ct;
  }

  bool eventsChange() const {
    return events_change_;
  }

  void handle(int events);

  void set_iport(const std::string& str) {
    iport_ = str;
  }

  void openHeartBeat(int ms) {
    heart_beat_ = true;
    assert(ms > 0);
    each_ms_ = ms;
  }

  bool isOpenHeartBeat() const {
    return heart_beat_;
  }

public:
  void setCodec(Codec* codec) {
    codec_ = codec;
    codec_->setOnMessage(on_message_);
    on_message_ = [this](TcpConnection* ptr)->void {
      this->codec_->onMessage(ptr);
    };
  }

  void send(const char* ptr) {
    send(ptr, strlen(ptr));
  }

  void send(const char* ptr, size_t n);

  void shutdownWr() {
    if(want_shutdown_wr_ == true) {
      logger()->fatal("multi shutdown_wr. ");
    }
    want_shutdown_wr_ = true;
    if(writing_ == false) {
      events_ = events_ | EPOLLOUT;
      writing_ = true;
    }
  }

  void forceClose() {
    setState(connState::force_colse);
  }

  bool isReadComplete() const {
    return read_zero_;
  }

  bool isWriteComplete() const {
    return really_shutdown_wr_;
  }

  Codec* getCodec() const {
    return codec_;
  }

  size_t size() const {
    return read_buf_.usedSize();
  }

  const char* data() const {
    return read_buf_.data();
  }

  void abandon(size_t n) {
    read_buf_.abandon(n);
  }

  std::string read(size_t n) {
    assert(n <= read_buf_.usedSize());
    std::string result;
    result.append(data(), n);
    read_buf_.abandon(n);
    return result;
  }

  const std::string& iport() const {
    return iport_;
  }

  connState getState() const {
    return state_;
  }

  const char* getStateStr();

  template<typename T>
  T* getContext() const {
    return static_cast<T*>(context_.get());
  }

  void setContext(std::shared_ptr<void> ptr) {
    context_ = ptr;
  }

private:
  Buffer read_buf_;
  Buffer write_buf_;

  int fd_;
  int events_;
  bool events_change_;

  connState state_;
  int error_value_;

  bool writing_;
  bool want_shutdown_wr_;
  bool really_shutdown_wr_;
  bool read_zero_;

  callback_type on_connection_;
  callback_type on_message_;
  callback_type on_write_complete_;
  callback_type on_close_;

  bool listen_socket_;
  callback_type on_accept_;

  bool event_fd_;

  bool conning_;
  callback_type on_succ_conn_;

  std::string iport_;

  bool alive_;
  bool heart_beat_;
  int each_ms_;

  std::shared_ptr<void> context_;

  Codec* codec_;

  bool added_to_poller_;
};
}

#endif // TCP_CONNECTION_H
