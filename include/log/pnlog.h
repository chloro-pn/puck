#pragma once
#include "back_end.h"
#include "capture.h"
#include "file_out_stream.h"
#include <memory>

namespace pnlog {
  extern std::shared_ptr<BackEnd> backend;
}

using pnlog::piece;
