#ifndef PUCK_SIGNAL_H
#define PUCK_SIGNAL_H

#include <sys/signal.h>
#include <map>
#include <functional>

namespace puck {
class Signal {
  friend void handle_all_sigs(int);

private:
  std::map<int, std::function<void(int)>> sig_handle_;

  Signal();

public:
  void handle(int sig,const std::function<void(int)>& handle);

  static Signal& instance();

  void ign(int sig);

  void def(int sig);
};
}

#endif // PUCK_SIGNAL_H
