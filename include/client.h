#ifndef CLIENT_H
#define CLIENT_H

#include "poller.h"
#include "tcp_connection.h"
#include <string>

namespace puck {
class Client {

    using callback_type = std::function<void(TcpConnection*)>;

public:
  Client(std::string, uint16_t);

  void connect();

  void bind(Poller* loop) {
    loop_ = loop;
  }

  void setOnConnection(callback_type ct) {
    on_connection_ = ct;
  }

  void setOnMessage(callback_type ct) {
    on_message_ = ct;
  }

  void setOnWriteComplete(callback_type ct) {
    on_write_complete_ = ct;
  }

  void setOnClose(callback_type ct) {
    on_close_ = ct;
  }

private:
  Poller* loop_;
  std::string ip_;
  uint16_t port_;
  int fd_;

  bool isSelfConnect(int);

  void add_to_poller(TcpConnection* ptr);

  void succ_conn(TcpConnection* ptr);

  callback_type on_connection_;
  callback_type on_message_;
  callback_type on_write_complete_;
  callback_type on_close_;
};
}

#endif // CLIENT_H
