#pragma once
#include "out_stream_base.h"
#include <string>

namespace pnlog {
class FifoOutStream : public out_stream_base {
private:
  int fd;
  bool closed;

public:
  FifoOutStream(std::string filepath);

  void write(const char *ptr, size_type n) override;

  void close() override;

  ~FifoOutStream();
};
}

