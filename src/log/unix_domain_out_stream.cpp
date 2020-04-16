#include "../../include/log/unix_domain_out_stream.h"
#include "../../include/log/release_assert.h"
#include <sys/un.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

namespace pnlog {
unix_domain_out_stream::unix_domain_out_stream(std::string filepath):unix_fd(-1),closed(false) {
  unix_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  struct sockaddr_un unix_addr;
  bzero(&unix_addr, sizeof(unix_addr));

  unix_addr.sun_family = AF_LOCAL;
  bcopy(filepath.c_str(),(char*)unix_addr.sun_path, sizeof(unix_addr.sun_path));
  socklen_t len = sizeof(unix_addr);
  int result = connect(unix_fd, (struct sockaddr*)&unix_addr, len);
  //std::cout << strerror(errno) << std::endl;
  release_assert(result == 0);
}

void unix_domain_out_stream::write(const char *ptr, size_type n) {
  int wn = ::write(unix_fd, ptr, n);
  release_assert(wn == n);
}

void unix_domain_out_stream::close() {
  if(closed == false) {
    ::close(unix_fd);
    closed = true;
  }
}

unix_domain_out_stream::~unix_domain_out_stream() {
  close();
}
}

