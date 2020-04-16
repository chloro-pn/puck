#pragma once
#include "char_array_allocator.h"
#include "char_array.h"
#include <memory>

namespace pnlog {
class CharArrayWrapper {
private:
  std::shared_ptr<CharArray> buf_;

public:
  explicit CharArrayWrapper(int index) {
    buf_ = CharArrayAllocator::instance()->apply(index);
  }

  CharArrayWrapper(const CharArrayWrapper&) = default;
  CharArrayWrapper(CharArrayWrapper&&) = default;
  CharArrayWrapper& operator=(const CharArrayWrapper&) = default;
  CharArrayWrapper& operator=(CharArrayWrapper&&) = default;

  std::shared_ptr<CharArray> get() const {
    return buf_;
  }

  ~CharArrayWrapper() {
    if (buf_) {
      buf_->setZero();
      buf_->index_ = -1;
      CharArrayAllocator::instance()->give_back(buf_);
    }
  }
};
}
