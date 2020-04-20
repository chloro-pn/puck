#include "../include/puck_signal.h"
#include <cstdio>
#include <cstdlib>
#include <sys/signal.h>

namespace puck {

void handle_all_sigs(int sig) {
  auto& maps = Signal::instance().sig_handle_;
  if(maps.find(sig) ==maps.end()) {
    return;
  }
  maps[sig](sig);
}

Signal::Signal() {

}

Signal& Signal::instance() {
  static Signal object;
  return object;
}

void Signal::handle(int sig, const std::function<void (int)> &handle) {
  sig_handle_[sig] = handle;
  struct sigaction act, oact;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = handle_all_sigs;
  if(sigaction(sig, &act, &oact) < 0) {
    fprintf(stderr, "signal error.");
    ::exit(-1);
  }
}

void Signal::def(int sig) {
  struct sigaction act, oact;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = SIG_DFL;
  if(sigaction(sig, &act, &oact) < 0) {
    fprintf(stderr, "signal error.");
    ::exit(-1);
  }
}

void Signal::ign(int sig) {
  struct sigaction act, oact;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = SIG_IGN;
  if(sigaction(sig, &act, &oact) < 0) {
    fprintf(stderr, "signal error.");
    ::exit(-1);
  }
}
}
