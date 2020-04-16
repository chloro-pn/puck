#pragma once
#include "out_stream_base.h"
#include "char_array_wrapper.h"
#include "condition_variable_type.h"
#include "spin_lock.h"
#include "back_end.h"
#include <mutex>
#include <memory>
#include <atomic>

namespace pnlog {
  class BackEnd;

  class outer {
    friend class BackEnd;
  private:
    std::shared_ptr<out_stream_base> out_stream_;
    std::unique_ptr<CharArrayWrapper> buf_;
    using lock_type = std::mutex;
    mutable lock_type mut_;
    condition_variable_type<lock_type>::type reopen_or_close_cv_;
    BackEnd* back_;
    size_type index_;

    enum class state { closed, closing, writing };
    state state_;
    enum class syn { yes, no };
    syn syn_;

  public:
    outer(size_type index, BackEnd* back);

    void open(out_stream_base* stream);

    void open_syn(out_stream_base* stream);

    void reopen(out_stream_base* stream);

    void write(const char* buf, size_type length);

    void close();
  };
}
