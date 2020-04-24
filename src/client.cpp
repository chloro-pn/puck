#include "../include/client.h"
#include "../include/sockets.h"
#include "../include/tcp_connection.h"
#include "../include/md5_codec.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cassert>
#include <unistd.h>

namespace puck {
Client::Client(std::string ip, uint16_t port):loop_(nullptr),
                                              ip_(ip),
                                              port_(port),
                                              fd_(-1) {

}

void Client::connect(int ms) {
  while(true) {
    fd_ = sockets::get_nonblock_socket();
    sockets::no_delay(fd_);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_port = htons(port_);
    server_addr.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, ip_.c_str(), &server_addr.sin_addr);
    if(ret <= 0) {
      logger()->fatal(piece("inet_pton error : ", strerror(errno)));
    }

    socklen_t len = sizeof(server_addr);
    ret = ::connect(fd_, (struct sockaddr*)& server_addr, len);
    if(ret == 0) {
      if(isSelfConnect(fd_) == true) {
        ::close(fd_);
        fd_ = -1;
        continue;
      }
      else {
        TcpConnection* ptr = getConnectedConnection();
        ptr->onConnection();
        if(ptr->shouldClose()) {
          loop_->clean(ptr);
        }
        else {
          add_to_poller(ptr);
        }
        return;
      }
    }
    else {
      assert(ret == -1);
      if(errno == EINPROGRESS) {
        TcpConnection* ptr = getConningConnection();
        std::string key = ptr->iport();

        add_to_poller(ptr);
        loop_->run_after(ms, [this, key]()->bool {
          this->connectCheck(key);
          return false;
        });
      }
      else {
        logger()->fatal(piece("connect error : ", strerror(errno)));
      }
      return;
    }
  }
}

TcpConnection* Client::getConningConnection() {
  TcpConnection* ptr = new TcpConnection(fd_, EPOLLOUT);
  ptr->setOnMessage(on_message_);
  ptr->setOnConnection(on_connection_);
  ptr->setOnWriteComplete(on_write_complete_);
  ptr->setOnClose(on_close_);
  ptr->setOnSuccConn([this](TcpConnection* ptr)->void {
    this->succ_conn(ptr);
  });
  ptr->setConningFlag();

  auto key = sockets::get_tcp_iport(fd_);
  ptr->set_iport(key);

  return ptr;
}

TcpConnection* Client::getConnectedConnection() {
  TcpConnection* ptr = new TcpConnection(fd_, EPOLLIN);
  ptr->setOnMessage(on_message_);
  ptr->setOnConnection(on_connection_);
  ptr->setOnWriteComplete(on_write_complete_);
  ptr->setOnClose(on_close_);
  ptr->setConnectedFlag();
  auto key = sockets::get_tcp_iport(fd_);
  ptr->set_iport(key);

  return ptr;
}

void Client::connectCheck(std::string key) {
  if(loop_->conns_.find(key) == loop_->conns_.end()) {
    return;
  }
  else {
    TcpConnection* ptr = loop_->conns_[key];
    if(ptr->isConning() == true) {
      ptr->setState(TcpConnection::connState::out_of_data);
      loop_->clean(ptr);
      return;
    }
    else {
      return;
    }
  }
}

void Client::succ_conn(TcpConnection *ptr) {
  ptr->onConnection();
}

bool Client::isSelfConnect(int fd) {
  return sockets::get_peer_addr(fd) == sockets::get_local_addr(fd);
}

void Client::add_to_poller(TcpConnection* ptr) {
  loop_->push_func([ptr, this]()->void {
    loop_->add(ptr);
  });
  loop_->wake_up();
}
}
