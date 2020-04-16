#pragma once
#include <mutex>
#include <condition_variable>

namespace pnlog {
  template<class T>
  struct condition_variable_type {
    using type = std::condition_variable_any;
  };

  template<>
  struct condition_variable_type<std::mutex> {
    using type = std::condition_variable;
  };
}