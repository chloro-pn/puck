#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <cassert>
#include "console.h"

namespace puck {
class Buffer {
  friend class TcpConnection;

private:
  std::string buffer_;
  size_t begin_;
  size_t end_;

  void tryMoveToForward();

  void moveToForward();

  void beginMove(size_t n);

  void endMove(size_t n);

public:
  explicit Buffer(size_t size);

  const char* data() const {
    return &buffer_[begin_];
  }

  size_t usedSize() const {
    return end_ - begin_;
  }

  char* availableArea() {
    return &buffer_[end_];
  }

  size_t unusedSize() const {
    return buffer_.size() - end_;
  }

  void abandon(size_t n) {
    beginMove(n);
  }

  void push(const char* ptr, size_t n);
};
}


#endif // BUFFER_H
