#pragma once
#include <string>
#include <ctime>
#include <sys/time.h>

namespace pnlog {
class timer {
private:
  timer() = default;

public:
  static timer& instance() {
    static timer inst;
    return inst;
  }
  std::string now();
};
}

