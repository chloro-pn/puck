#pragma once
#include "condition_variable_type.h"
#include "type.h"
#include "spin_lock.h"
#include <thread>
#include <vector>
#include <functional>
#include <list>
#include <mutex>

namespace pnlog {
  class ThreadPool {
  private:
    using lock_type = spin;
    std::vector<std::thread> threads_;
    using task_type = std::function<void()>;
    std::list<task_type> tasks_;
    size_type th_counts_;
    condition_variable_type<lock_type>::type cv_;
    lock_type mut_;
    bool stop_;

    using exception_callback = std::function<void(std::exception&)>;
    exception_callback ec_;

  public:
    ThreadPool(size_type count);

    void each_thread();

    void start();

    void stop();

    template<class F>
    void push_task(F&& func) {
      mut_.lock();
      tasks_.push_back(std::forward<F>(func));
      mut_.unlock();
      cv_.notify_one();
    }

    template<class T>
    void set_exception_callback(T&& ec) {
      ec_ = std::forward<T>(ec);
    }

    ~ThreadPool();
  };
}//namespace pnlog
