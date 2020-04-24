#include "../include/tcp_server.h"
#include "../include/sockets.h"
#include "../include/md5_codec.h"

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
      TcpConnection* ptr = createTcpConnection(clientfd);
      loop_->newConnection(ptr);
    }
  }
}

TcpConnection* TcpServer::createTcpConnection(int clientfd) {
  sockets::set_nonblock(clientfd);
  sockets::no_delay(clientfd);
  TcpConnection* ptr = new TcpConnection(clientfd, EPOLLIN);
  ptr->setOnMessage(on_message_);
  ptr->setOnConnection(on_connection_);
  ptr->setOnWriteComplete(on_write_complete_);
  ptr->setOnClose(on_close_);
  if(isOpenHeartBeat() == true) {
    ptr->openHeartBeat(each_ms_);
  }

  return ptr;
}

TcpConnection* TcpServer::createAcceptConnection() {
  TcpConnection* ptr = new TcpConnection(listenfd_, EPOLLIN);
  ptr->setListenSocketFlag();
  ptr->setOnAccept([this](TcpConnection* con)->void {
    this->accept_callback(con);
  });
  ptr->set_iport("acceptConnection");
  return ptr;
}

TcpServer::TcpServer(uint16_t port):listenfd_(-1),
                                    loop_(nullptr),
                                    each_ms_(-1),
                                    heart_beat_(false)
{
  listenfd_ = sockets::get_nonblock_socket();
  sockets::reuse_addr(listenfd_);

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);
  server_addr.sin_family = AF_INET;

  socklen_t len = sizeof(server_addr);
  int result = ::bind(listenfd_, (struct sockaddr*)&server_addr, len);
  if(result != 0) {
    logger()->fatal(piece("bind error : ", strerror(errno)));
  }
  result = listen(listenfd_, 1024000);
  if(result != 0) {
    logger()->fatal(piece("listen error : ", strerror(errno)));
  }
}

void TcpServer::bind(EventLoop* poller_) {
  loop_ = poller_;
  TcpConnection* ptr = createAcceptConnection();
  loop_->add(ptr);
}
}
