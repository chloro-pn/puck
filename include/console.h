#ifndef CONSOLE_H
#define CONSOLE_H

#include "log/pnlog.h"
namespace puck {
  std::shared_ptr<pnlog::CapTure>& logger();
}

#endif // CAPTURE_H
