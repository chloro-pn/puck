#include "../../include/log/fifo_out_stream.h"
#include "../../include/log/release_assert.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <errno.h>
#include <fcntl.h> //O_WRONLY
#include <unistd.h>

namespace pnlog {
  FifoOutStream::FifoOutStream(std::string filepath):fd(-1),closed(false) {
    fd = open(filepath.c_str(), O_WRONLY);
    if(fd == -1) {
      fprintf(stderr, "fifo open error : ", strerror(errno));
    }
  }

  void FifoOutStream::write(const char *ptr, size_type n) {
    int nw = ::write(fd, ptr, n);
    release_assert(nw == n);
  }

  void FifoOutStream::close() {
    if(closed == false) {
      int result = ::close(fd);
      release_assert(result == 0);
      closed = true;
    }
  }

  FifoOutStream::~FifoOutStream() {
    close();
  }
}
