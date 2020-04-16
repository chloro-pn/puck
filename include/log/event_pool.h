#ifndef EVENT_POOL_H
#define EVENT_POOL_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <queue> // for priority queue.
#include <chrono>
#include <string>
#include <atomic>
#include <thread>
#include <cassert>

 // -> https://github.com/chloro-pn/event_pool.

using namespace std::chrono;
class event_pool;

class event_handle : public std::enable_shared_from_this<event_handle> {
  friend class event_pool;

public:
  enum class type { once, every };
  type type_;
  std::string id_;
  std::function<void(std::shared_ptr<event_handle>)> func_;
  std::shared_ptr<void> args_;
  void wake_up();

private:
  std::weak_ptr<event_pool> event_;
  bool trigger_;
  std::atomic<bool> done_; //only for once state event_handle.
  std::shared_ptr<std::promise<void>> promise_;

public:
  event_handle():trigger_(false),done_(false) {

  }
};

class time_handle : public std::enable_shared_from_this<time_handle> {
  friend class event_pool;
public:

  enum class type { duration, timepoint };
  type type_;
  std::string id_;
  std::function<void(std::shared_ptr<time_handle>)> func_;
  std::shared_ptr<void> args_;
  time_point<system_clock,milliseconds> time_point_; //only for timepoint
  milliseconds duration_; //only fo duration

private:
  std::shared_ptr<std::promise<void>> promise_;
};

class event_pool : public std::enable_shared_from_this<event_pool> {
  friend class event_handle;
  friend class time_handle;

private:
  std::mutex mut_;
  std::condition_variable at_least_one_wake_up_;
  std::condition_variable too_many_events_;
  std::vector<std::shared_ptr<event_handle>> handles_;
  std::vector<std::shared_ptr<event_handle>> active_handles_;
  uint64_t triggers_;
  uint64_t events_;
  uint64_t max_events_;
  bool stop_;
  std::thread thread_;
  //for timer.
  using ptr_time_handle = std::shared_ptr<time_handle>;
  struct cmp_for_ptr_handle {
    bool operator()(ptr_time_handle h1,ptr_time_handle h2) {
      return h1->time_point_ > h2->time_point_;
    }
  };

  std::priority_queue<ptr_time_handle,std::vector<ptr_time_handle>,cmp_for_ptr_handle> timers_;

  //hold mutex.
  void remove_all_triggered_handle_and_reset() {
    for(auto it = handles_.begin();it!=handles_.end();) {
      if((*it)->trigger_ == true) {
        it = handles_.erase(it);
      }
      else {
        ++it;
      }
    }
    triggers_ = 0;
  }

  std::vector<std::shared_ptr<event_handle>> get_ready() {
    std::vector<std::shared_ptr<event_handle>> result;
    std::vector<std::shared_ptr<time_handle>> tmp;
    std::unique_lock<std::mutex> mut(mut_);
    if (timers_.empty() == true) {
      at_least_one_wake_up_.wait(mut,[this]()->bool {
        return this->active_handles_.empty() == false || this->timers_.empty() == false || this->stop_ == true;
      });
    }
    else {
      time_point<system_clock,milliseconds> now = time_point_cast<milliseconds>(system_clock::now());
      milliseconds dt = timers_.top()->time_point_ - now;
      at_least_one_wake_up_.wait_for(mut,milliseconds(dt));
    }
    //即使被stop唤醒，也尽量完成event。

    //惰性删除已经被处理的事件：event_handle个数如果大于等于1024，则一次性删除一次。
    if(triggers_ >= 1024) {
      remove_all_triggered_handle_and_reset();
    }

    time_point<system_clock,milliseconds> now = time_point_cast<milliseconds>(system_clock::now());
    while(timers_.empty() != true) {
      if(timers_.top()->time_point_ <= now) {
        tmp.push_back(timers_.top());
        timers_.pop();
        --events_;
      }
      else {
        break;
      }
    }
    events_ -= active_handles_.size();
    result.swap(active_handles_);

    mut.unlock();
    too_many_events_.notify_all();
    for(auto each : tmp) {
      each->func_(each);
      if(each->promise_) {
        each->promise_->set_value();
        each->promise_.reset();
      }
      if(each->type_ == time_handle::type::duration) {
        push_timer(each);
      }
    }
    return result;
  }

public:
  event_pool():triggers_(0),events_(0),max_events_(1024),stop_(false) {

  }

  void start() {
    thread_ = std::move(std::thread([this]()->void {
      this->run();
    })
    );
  }

  std::future<void> push_event(std::shared_ptr<event_handle> h) {
    std::unique_lock<std::mutex> mut(mut_);
    too_many_events_.wait(mut,[this]()->bool{return this->events_ <= max_events_;});
    h->event_ = shared_from_this();
    h->promise_.reset(new std::promise<void>());
    auto result = h->promise_->get_future();
    handles_.push_back(h);
    ++events_;
    return result;
  }

  std::future<void> push_timer(std::shared_ptr<time_handle> h) {
    std::unique_lock<std::mutex> mut(mut_);
    too_many_events_.wait(mut,[this]()->bool{return this->events_ <= max_events_;});
    if(h->type_ == time_handle::type::duration) {
      h->time_point_ = time_point_cast<milliseconds>(system_clock::now()) + h->duration_;
    }
    h->promise_.reset(new std::promise<void>());
    auto result = h->promise_->get_future();
    //no matter which type of h,we always use time_point_ data member.
    timers_.push(h);
    ++events_;
    at_least_one_wake_up_.notify_all();
    return result;
  }

  void set_max_events(uint64_t max_events) {
    max_events_ = max_events;
  }

  void stop() {
    std::unique_lock<std::mutex> mut(mut_);
    stop_ = true;
    at_least_one_wake_up_.notify_all();
    mut.unlock();
    thread_.join();
  }

  ~event_pool() {
    assert(thread_.joinable() == false);
  }

  void run() {
    while(true) {
      std::vector<std::shared_ptr<event_handle>> ready = get_ready();
      for(std::shared_ptr<event_handle> each : ready) {
        if(each->func_) {
          each->func_(each);
          if(each->promise_) {
            each->promise_->set_value();
            each->promise_.reset();
          }
        }
      }
      std::unique_lock<std::mutex> mut(mut_);
      if(stop_ == true) {
        return ;
      }
    }
  }
};

//对于once的event来说，只能被唤醒一次，但wake_up函数可多次调用（线程安全）
inline
void event_handle::wake_up() {
  std::shared_ptr<event_pool> hold = event_.lock();
  if (!hold) {
    //invalid event_.
    return;
  }
  bool except_ = false;
  if (type_ == type::once) {
    if(done_.compare_exchange_strong(except_,true) == false) {
      //have been waked_up.
      return;
    }
  }
  std::unique_lock<std::mutex> mut(hold->mut_);
  if(hold->stop_ == true) {
    //event_ stop.
    return;
  }
  assert(trigger_ == false);
  hold->active_handles_.push_back(shared_from_this());
  if(type_ == type::once) {
    trigger_ = true;
    hold->triggers_ += 1;
  }// esle type_ == type::every, and trigger_ always remain false,so event_ will never delete it.
  hold->at_least_one_wake_up_.notify_all();
}

#endif // EVENT_POOL_H
