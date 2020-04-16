#pragma once
#include <cstdlib>

#define release_assert(x) \
  if(!(x)) { \
    std::abort(); \
  } \
