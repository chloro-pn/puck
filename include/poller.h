#ifndef POLLER_H
#define POLLER_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <map>
#include <queue>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <memory>
#include <atomic>
#include "console.h"
#include "tcp_connection.h"

namespace puck {

using namespace std::chrono;

class Poller {
  friend class Client;

private:
  int epfd_;
  int eventfd_;
  std::map<std::string, TcpConnection*> conns_;
  using task_type = std::function<bool()>;

  std::unique_ptr<std::mutex> mut_;
  std::vector<std::function<void()>> funcs_;

  std::atomic<bool> stop_;

  struct time_event {
    using tp_type = time_point<system_clock, milliseconds>;
    tp_type time_point_;
    milliseconds ms_;
    task_type task_;
    time_event(tp_type tp, milliseconds ms, const task_type& task):time_point_(tp),
                                                  ms_(ms),
                                                  task_(task)
                                                  {

    }
  };

  struct cmp_for_time_event {
    bool operator()(const time_event& t1, const time_event& t2) {
      return t1.time_point_ > t2.time_point_;
    }
  };

  std::priority_queue<time_event, std::vector<time_event>, cmp_for_time_event> timers_;

  int get_latest_time();

  void handle_timer_events();

  void handle_wake_up();

  void change(TcpConnection* ptr);

  bool handle_heart_beat(std::string key);

  void handle_funcs();

  TcpConnection* createEventConnection();
public:
  Poller();

  ~Poller();

  void stop() {
    stop_.store(true);
    wake_up();
  }

  void run_after(uint32_t ms, const task_type& task);

  void add(TcpConnection* ptr);

  void loop();

  void wake_up();

  void push_func(const std::function<void()>& func);

  void clean(TcpConnection* ptr);
};
}

#endif // POLLER_H
