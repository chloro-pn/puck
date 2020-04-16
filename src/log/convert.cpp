#include "../../include/log/convert.h"

namespace pnlog {
  template<>
  std::string convert_to_string(const std::string& str) {
    return str;
  }

  std::string convert_to_string(const char* ptr) {
    return std::string(ptr);
  }

  std::string convert_to_string(char* ptr) {
    return std::string(ptr);
  }
}//namespace pnlog
