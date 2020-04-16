#pragma once
#include "back_end.h"
#include "capture.h"
#include <memory>

namespace pnlog {
  extern std::shared_ptr<BackEnd> backend;
}

using pnlog::piece;
