#include "../../include/log/std_out_stream.h"
#include <cstdio>
#include "../../include/log/release_assert.h"

namespace pnlog {
  StdOutStream::StdOutStream(FILE* f) :file_(f) {
    release_assert(f != nullptr);
  }

  void StdOutStream::write(const char* ptr, size_type n) {
    auto code = fprintf(file_, ptr);
    release_assert(code >= 0);
  }

  void StdOutStream::close() {
    //do nothing.pnlog would not close stdout.
  }

  StdOutStream::~StdOutStream() {

  }
}//namespace pnlog
