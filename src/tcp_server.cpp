#include "../include/tcp_server.h"
#include "../include/sockets.h"

namespace puck {
void TcpServer::accept_callback(TcpConnection* con) {
  while(true) {
    int clientfd = accept(con->fd(), nullptr, nullptr);
    if(clientfd == -1) {
      if(errno != EWOULDBLOCK) {
        logger()->fatal(piece("accept error, errno : ", strerror(errno)));
      }
      else {
        break;
      }
    }
    else {
      sockets::set_nonblock(clientfd);
      TcpConnection* ptr = new TcpConnection(clientfd, EPOLLIN);
      ptr->setOnMessage(on_message_);
      ptr->setOnConnection(on_connection_);
      ptr->setOnWriteComplete(on_write_complete_);
      ptr->setOnClose(on_close_);
      loop_->newConnection(ptr);
    }
  }
}

TcpServer::TcpServer(std::string ip, uint16_t port):listenfd_(-1), loop_(nullptr) {
  listenfd_ = sockets::get_nonblock_socket();

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  int result = inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
  if(result <= 0) {
    logger()->fatal(piece("inet pton error : ", strerror(errno)));
  }
  server_addr.sin_port = htons(port);
  server_addr.sin_family = AF_INET;

  socklen_t len = sizeof(server_addr);
  result = ::bind(listenfd_, (struct sockaddr*)&server_addr, len);
  if(result != 0) {
    logger()->fatal(piece("bind error : ", strerror(errno)));
  }
  result = listen(listenfd_, 1024000);
  if(result != 0) {
    logger()->fatal(piece("listen error : ", strerror(errno)));
  }
}

TcpServer::TcpServer(std::string unix_addr):listenfd_(-1), loop_(nullptr) {
  listenfd_ = socket(AF_LOCAL, SOCK_STREAM, 0);
  struct sockaddr_un server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sun_family = AF_LOCAL;
  bcopy(unix_addr.c_str(), (char*)server_addr.sun_path, sizeof(server_addr.sun_path));
  socklen_t len = sizeof(server_addr);
  int result = ::bind(listenfd_, (struct sockaddr*)&server_addr, len);
  if(result != 0) {
    logger()->fatal(piece("bind error : ", strerror(errno)));
  }
  result = listen(listenfd_, 1024);
  if(result != 0) {
    logger()->fatal(piece("listen error : ", strerror(errno)));
  }
}

void TcpServer::bind(EventLoop* poller_) {
  loop_ = poller_;
  epoll_event ev;
  ev.events = EPOLLIN;
  TcpConnection* ptr = new TcpConnection(listenfd_, EPOLLIN);
  ptr->setListenSocketFlag();
  ptr->setOnAccept([this](TcpConnection* con)->void {
    this->accept_callback(con);
  });
  ev.data.ptr = ptr;
  loop_->add(listenfd_, &ev);
}
}
