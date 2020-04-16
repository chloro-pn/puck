#include "../../include/log/file_out_stream.h"
#include "../../include/log/release_assert.h"
#include <cassert>
#include <iostream>

namespace pnlog {
  FileOutStream::FileOutStream(std::string filename) :file_(nullptr),closed_(false) {
    file_ = fopen( filename.c_str(), "w");
    release_assert(file_ != nullptr);
  }

  void FileOutStream::write(const char* ptr, size_type n) {
    release_assert(file_ != nullptr);
    auto code = fwrite(ptr, static_cast<size_t>(n), static_cast<size_t>(1), file_);
    release_assert(code == 1 || code == 0);
    fflush(file_);
  }

  void FileOutStream::close() {
    if (closed_ == false) {
      fclose(file_);
      closed_ = true;
    }
  }

  FileOutStream::~FileOutStream() {
    close();
  }
}//namesapce pnlog
