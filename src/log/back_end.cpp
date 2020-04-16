#include "../../include/log/back_end.h"
#include "../../include/log/capture.h"
#include "../../include/log/std_out_stream.h"
#include "../../include/log/str_appender.h"
#include "../../include/log/blocking_queue.h"
#include "../../include/log/file_out_stream.h"
#include "../../include/log/outer.h"
#include "../../include/log/timer.h"
#include "../../include/log/release_assert.h"

namespace pnlog {
  std::shared_ptr<BackEnd> BackEnd::get_instance() {
    static std::shared_ptr<BackEnd> backend(new BackEnd(128));
    return backend;
  }

  void BackEnd::write(size_type index, const char* ptr, size_type n) {
    outers_.at(static_cast<unsigned int>(index))->write(ptr, n);
  }

  BackEnd::BackEnd(size_type size) :pool_(1),
                                    event_pool_(new event_pool()),
                                    size_of_streams_and_bufs_(size),
                                    ca_allocator_(CharArrayAllocator::instance()),
                                    stop_(false){
    release_assert(size > 0);
    for (int i = 0; i < size; ++i) {
      outers_.emplace_back(new outer(i, this));
    }
    event_pool_->start();

    options op;
    op.asyn = false;
    open(op, 0, new StdOutStream(stdout));
    pool_.start();
  }

  std::shared_ptr<CapTure> BackEnd::get_capture(size_type index) {
      return std::shared_ptr<CapTure>(new CapTure(shared_from_this(), index));
  }

  void BackEnd::open(options option, size_type index, out_stream_base* out) {
    if (option.asyn == false) {
      outers_.at(static_cast<unsigned int>(index))->open_syn(out);
    }
    else {
      outers_.at(static_cast<unsigned int>(index))->open(out);
    }
    if (option.duration_rotating == true) {
      std::string path = option.path;
      std::shared_ptr<time_handle> ev(new time_handle());
      ev->type_ = time_handle::type::duration;
      ev->args_ = nullptr;
      ev->duration_ = option.duration;
      ev->func_ = [this, index, path](std::shared_ptr<time_handle> self)->void {
        outers_.at(index)->reopen(new FileOutStream(path +std::string("_") + timer::instance().now()));
      };
      event_pool_->push_timer(ev);
    }
  }

  void BackEnd::reopen(size_type index, out_stream_base* out) {
    outers_.at(static_cast<unsigned int>(index))->reopen(out);
  }

  void BackEnd::close(size_type index) {
    outers_.at(static_cast<unsigned int>(index))->close();
  }

  void BackEnd::stop() {
    bool exp = false;
    if (stop_.compare_exchange_strong(exp, true)) {
      for (auto& each : outers_) {
        each->close();
      }
      event_pool_->stop();
      pool_.stop();
    }
  }

  void BackEnd::abort(std::string error_message) {
    bool exp = false;
    if (stop_.compare_exchange_strong(exp, true)) {
      for (auto& each : outers_) {
        each->close();
      }
      event_pool_->stop();
      pool_.stop();
      if (error_message != "") {
        fprintf(stderr, error_message.c_str());
      }
      std::abort();
    }
  }

  BackEnd::~BackEnd() {
    stop();
  }

  bool BackEnd::rangecheck(size_type index) const {
    if (index < 0 || index >= size_of_streams_and_bufs_) {
      return false;
    }
    return true;
  }

  std::future<void> BackEnd::push_buf(CharArrayWrapper&& buf) {
    auto result = bufs_.push(std::move(buf));
    pool_.push_task([this]()->void {
      this->run_in_back();
    });
    return result;
  }

  void BackEnd::run_in_back() {
    auto bufs = bufs_.get_all();
    if (bufs.empty() == true) {
      return;
    }
    for (auto& each : bufs) {
      CharArrayWrapper& buf = each.first;
      size_type index = buf.get()->getIndex();
      outers_.at(static_cast<unsigned int>(index))->out_stream_->write(buf.get()->getBuf(), buf.get()->getSize());
      each.second.set_value();
    }
  }
}//namespace pnlog
