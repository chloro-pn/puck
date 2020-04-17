#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "sockets.h"
#include "event_loop.h"
#include "tcp_connection.h"
#include <cstring>
#include <string>
#include <functional>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/errno.h>

namespace puck {
class TcpServer {
private:
  int listenfd_;
  EventLoop* loop_;
  using callback_type = std::function<void(TcpConnection*)>;
  callback_type on_connection_;
  callback_type on_message_;
  callback_type on_write_complete_;

  void accept_callback(TcpConnection* con);

public:
  TcpServer(std::string ip, uint16_t port);

  TcpServer(std::string unix_addr);

  void setOnConnection(callback_type ct) {
    on_connection_ = ct;
  }

  void setOnMessage(callback_type ct) {
    on_message_ = ct;
  }

  void setOnWriteComplete(callback_type ct) {
    on_write_complete_ = ct;
  }

  void bind(EventLoop* poller_);
};

}

#endif // TCP_SERVER_H
