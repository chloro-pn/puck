#include "../include/console.h"

namespace puck {
std::shared_ptr<pnlog::CapTure>& logger() {
  static std::shared_ptr<pnlog::CapTure> log(pnlog::backend->get_capture(0));
  return log;
}
}
