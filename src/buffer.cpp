#include "buffer.h"
#include <cstring>

namespace puck {
void Buffer::tryMoveToForward() {
  if(end_ - begin_ >= 4096) {
    moveToForward();
  }
}

void Buffer::moveToForward() {
  size_t size = usedSize();
  std::string tmp = buffer_.substr(begin_, size);
  tmp.copy(&buffer_[0], size);
  begin_ = 0;
  end_ = size;
}

Buffer::Buffer(size_t size):begin_(0), end_(0) {
  buffer_.resize(size, '\0');
}

void Buffer::beginMove(size_t n) {
  if(begin_ + n > end_) {
    logger()->fatal(piece("Buffer error, ", begin_ + n, " should <= ", end_));
  }
  begin_ += n;
  if(begin_ == end_) {
    begin_ = end_ = 0;
  }
}

void Buffer::endMove(size_t n) {
  if(end_ + n > buffer_.size()) {
    logger()->fatal(piece("Buffer error, ", end_ + n, " should > ", buffer_.size()));
  }
  end_ += n;
  if(unusedSize() <= 1024) {
    if(begin_ >= 1024) {
      moveToForward();
    }
    else {
      logger()->warning(piece(usedSize(), " bytes message storage in the buffer."));
    }
  }
}

void Buffer::push(const char *ptr, size_t n) {
  if(n <= unusedSize()) {
    memcpy(availableArea(), ptr, n);
    end_ += n;
  }
  else {
    moveToForward();
    if(n > unusedSize()) {
      logger()->fatal(piece("Buffer push error, too many messages."));
    }
    memcpy(availableArea(), ptr, n);
    end_ += n;
  }
}
}
