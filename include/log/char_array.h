#pragma once
#include "type.h"
#include <cstring>
#include <cstdint>
#include <cassert>
#include <cstdio>
#include <string>
#include <utility> // for std::swap.

namespace pnlog {
  class CharArray {
  private:
    char* buf_;
    size_type size_;
    size_type end_;
    bool error_;
    size_type index_;
    friend class CharArrayWrapper;
    friend class CharArrayAllocator;

  public:
    explicit CharArray(size_type size = 4096,size_type index = -1) :buf_(nullptr), size_(size), end_(0), error_(false),index_(index) {
      buf_ = new char[static_cast<unsigned int>(size)]();
    }

    ~CharArray() {
      delete[] buf_;
    }

    CharArray(const CharArray& other) = delete;

    CharArray& operator=(const CharArray&) = delete;

    CharArray(CharArray&& other) noexcept {
      buf_ = other.buf_;
      size_ = other.size_;
      end_ = other.end_;
      error_ = other.error_;
      index_ = other.index_;
      other.buf_ = nullptr;
    }

    CharArray& operator=(CharArray&& other) noexcept {
      delete[] buf_;
      buf_ = other.buf_;
      size_ = other.size_;
      end_ = other.end_;
      error_ = other.error_;
      index_ = other.index_;
      other.buf_ = nullptr;
      return *this;
    }

    void swap(CharArray& other) {
      std::swap(buf_, other.buf_);
      std::swap(size_, other.size_);
      std::swap(end_, other.end_);
      std::swap(error_, other.error_);
      std::swap(index_, other.index_);
    }

    void append(const char* ptr, size_type nbyte) {
      size_type capacity = size_ - 1 - end_;
      if (capacity < nbyte || error_ == true) {
        error_ = true;
        return;
      }
      memcpy(&buf_[end_], ptr, static_cast<size_t>(nbyte));
      end_ += nbyte;
      buf_[end_] = '\0';
      return;
    }

    void append(const char* ptr) {
      append(ptr, strlen(ptr));
    }

    void append(const std::string& str) {
        append(str.data(), str.size());
    }

    const char* getBuf() const {
      return buf_;
    }

    size_type getSize() const {
      return end_;
    }

    size_type getIndex() const {
      return index_;
    }

    bool error() const {
      return error_;
    }

    void setZero() {
      end_ = 0;
      buf_[end_] = '\0';
      error_ = false;
    }
  };
}//namespace pnlog
