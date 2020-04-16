#pragma once
#include "out_stream_base.h"
#include <string>

namespace pnlog {
class unix_domain_out_stream : public out_stream_base {
private:
  int unix_fd;
  bool closed;

public:
  unix_domain_out_stream(std::string filepath);

  void write(const char *ptr, size_type n) override;

  void close() override;

  ~unix_domain_out_stream();
};
}
