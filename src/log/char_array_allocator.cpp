#include "../../include/log/char_array_allocator.h"
#include <cassert>

namespace pnlog {
CharArrayAllocator::CharArrayAllocator():size_(0) {

}

std::shared_ptr<CharArrayAllocator> CharArrayAllocator::instance() {
  static std::shared_ptr<CharArrayAllocator> instance_(new CharArrayAllocator());
  return instance_;
}

std::shared_ptr<CharArray> CharArrayAllocator::apply(int index) {
  std::unique_lock<lock_type> mut(m_);
  if(bufs_.empty() == true) {
    if (size_ < max_size_) {
      bufs_.emplace_back(new CharArray(4096, index));
      ++size_;
    }
    else {
      cv_.wait(mut, [this]()->bool {
                 return !bufs_.empty();
               });
    }
    //bufs_.emplace_back(new CharArray(4096, index));
  }
  std::shared_ptr<CharArray> result = bufs_.back();
  bufs_.pop_back();
  result->index_ = index;
  return result;
}

void CharArrayAllocator::give_back(std::shared_ptr<CharArray> item) {
  std::unique_lock<lock_type> mut(m_);
  bufs_.push_back(item);
  cv_.notify_one();
}

CharArrayAllocator::~CharArrayAllocator() {
  //assert(size_ == static_cast<int>(bufs_.size()));
}
}

