#pragma once
#include <memory> // std::shared_ptr
#include <vector> // std::vector
#include "char_array.h"
#include "condition_variable_type.h"
#include <mutex>

namespace pnlog {
class CharArrayAllocator {
private:
  std::vector<std::shared_ptr<CharArray>> bufs_; //unused bufs.
  int size_; // total bufs size, including unused and using bufs.
  static constexpr int max_size_ = 32; // max size_.

  using lock_type = std::mutex;
  lock_type m_;
  condition_variable_type<lock_type>::type cv_;

  CharArrayAllocator();
  CharArrayAllocator(const CharArrayAllocator&) = delete;
  CharArrayAllocator(CharArrayAllocator&&) = delete;
  CharArrayAllocator& operator=(const CharArrayAllocator&) = delete;
  CharArrayAllocator& operator=(CharArrayAllocator&&) = delete;

public:

  static std::shared_ptr<CharArrayAllocator> instance();

  std::shared_ptr<CharArray> apply(int index);

  void give_back(std::shared_ptr<CharArray> item);

  ~CharArrayAllocator();
};
}
