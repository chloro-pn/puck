#include "../../include/util/thread_pool.h"

namespace tool {
  ThreadPool::ThreadPool() :th_counts_(-1), stop_(false) {

  }

  void ThreadPool::each_thread() {
    while (true) {
      std::unique_lock<lock_type> lock(mut_);
      while (tasks_.empty() == true && stop_ == false) {
        cv_.wait(lock);
      }
      if (stop_ == true && tasks_.empty() == true) {
        return;
      }
      auto tmp = std::move(tasks_.front());
      tasks_.erase(tasks_.begin());
      lock.unlock();
      try {
        tmp();
      }
      catch (std::exception e) {
        if (ec_) {
          ec_(e);
        }
      }
    }
  }

  void ThreadPool::start(size_type count) {
    th_counts_ = count;
    for (size_type i = 0; i < th_counts_; ++i) {
      threads_.emplace_back([this]()->void {
        this->each_thread();
      });
    }
  }

  void ThreadPool::stop() {
    mut_.lock();
    stop_ = true;
    mut_.unlock();
    cv_.notify_all();
    for (auto it = threads_.begin(); it != threads_.end(); ++it) {
      it->join();
    }
  }

  ThreadPool::~ThreadPool() {

  }
}//namespace pnlog
