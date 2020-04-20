#include "../include/sockets.h"
#include "../include/console.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h> // TCP_NODELAY

namespace puck {
int sockets::get_nonblock_socket() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0) {
    logger()->fatal(piece("soket error. ", strerror(errno)));
  }
  int flags = fcntl(fd, F_GETFL, 0);
  if(flags < 0) {
    logger()->fatal(piece("soket error. ", strerror(errno)));
  }
  flags |= O_NONBLOCK;
  int result = fcntl(fd, F_SETFL, flags);
  if(result < 0) {
    logger()->fatal(piece("soket error. ", strerror(errno)));
  }
  return fd;
}

void sockets::set_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if(flags < 0) {
    logger()->fatal(piece("soket error. ", strerror(errno)));
  }
  flags |= O_NONBLOCK;
  int result = fcntl(fd, F_SETFL, flags);
  if(result < 0) {
    logger()->fatal(piece("soket error. ", strerror(errno)));
  }
}

void sockets::no_delay(int fd) {
  int onoff = 1;
  int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY , &onoff, sizeof(onoff));
  if(ret < 0) {
    logger()->fatal(piece("setsockopt error. ", strerror(errno)));
  }
}

std::string sockets::get_tcp_iport(int fd) {
  std::string result;
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(addr);
  int n = getsockname(fd, (struct sockaddr*)&addr, &len);
  if(n != 0) {
    logger()->fatal(piece("getsockname error. ", strerror(errno)));
  }
  char buf[64] = {'\0'};
  const char* ptr = inet_ntop(AF_INET, &addr, buf, sizeof(buf) - 1);
  if(ptr == nullptr) {
    logger()->fatal(piece("inet_ntop error. ", strerror(errno)));
  }
  result.append(buf, strlen(buf));
  result.push_back(':');
  result.append(std::to_string(ntohs(addr.sin_port)));
  result.push_back('-');

  memset(buf, '\0', sizeof(buf));
  len =sizeof(addr);
  memset(&addr, 0, sizeof(addr));
  n = getpeername(fd, (struct sockaddr*)& addr, &len);
  if(n != 0) {
    logger()->fatal(piece("getpeername error. ", strerror(errno)));
  }
  ptr = inet_ntop(AF_INET, &addr, buf, sizeof(buf) - 1);
  if(ptr == nullptr) {
    logger()->fatal(piece("inet_ntop error. ", strerror(errno)));
  }
  result.append(buf, strlen(buf));
  result.push_back(':');
  result.append(std::to_string(ntohs(addr.sin_port)));
  return result;
}
}
