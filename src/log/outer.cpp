#include "../../include/log/outer.h"
#include "../../include/log/back_end.h"
#include "../../include/log/release_assert.h"

namespace pnlog {
  outer::outer(size_type index, BackEnd* back) : back_(back),
                                                 index_(index),
                                                 state_(state::closed),
                                                 syn_(syn::no){

  }

  void outer::open(out_stream_base* stream) {
    std::unique_lock<lock_type> mut(mut_);
    release_assert(out_stream_ == nullptr);
    out_stream_.reset(stream);
    release_assert(buf_ == nullptr);
    buf_.reset(new CharArrayWrapper(index_));
    release_assert(state_ == state::closed);
    state_ = state::writing;
    syn_ = syn::no;
  }

  void outer::open_syn(out_stream_base* stream) {
    std::unique_lock<lock_type> mut(mut_);
    release_assert(out_stream_ == nullptr);
    out_stream_.reset(stream);
    release_assert(buf_ == nullptr);
    syn_ = syn::yes;
    release_assert(state_ == state::closed);
    state_ = state::writing;
  }

  void outer::reopen(out_stream_base* stream) {
    std::unique_lock<lock_type> mut(mut_);
    reopen_or_close_cv_.wait(mut,[this]()->bool{
        return this->state_ != state::closing;
    });
    if (state_ == state::closed) {
      if(syn_ == syn::yes) {
        mut.unlock();
        open_syn(stream);
      }
      else {
        mut.unlock();
        open(stream);
      }
      return;
    }
    else {
      if (syn_ == syn::yes) {
        out_stream_.reset(stream);
        //state_ == state::writing.
        return;
      }
      std::future<void> w = back_->push_buf(std::move(*buf_));
      buf_.reset();
      state_ = state::closing;
      mut.unlock();
      w.get();//blocking until back thread write all data.

      mut.lock();
      release_assert(state_ == state::closing);
      out_stream_.reset(stream);
      buf_.reset(new CharArrayWrapper(index_));
      syn_ = syn::no;
      state_ = state::writing;
      reopen_or_close_cv_.notify_all();
    }
  }

  void outer::write(const char* buf, size_type length) {
    std::unique_lock<lock_type> mut(mut_);
    //等待closing的最终状态。
    reopen_or_close_cv_.wait(mut,[this]()->bool{
        return this->state_ != state::closing;
    });
    if (state_ == state::closed) {
      return;
    }
    //state_ == state::writing.
    if (syn_ == syn::yes) {
      out_stream_->write(buf, length);
      return;
    }
    buf_->get()->append(buf, length);
    if (buf_->get()->error() == true) {
      back_->push_buf(std::move(*buf_));
      buf_.reset(new CharArrayWrapper(index_));
      buf_->get()->append(buf, length);
      release_assert(buf_->get()->error() == false);
    }
  }

  void outer::close() {
    std::unique_lock<lock_type> mut(mut_);
    if (state_ == state::closed || state_ == state::closing) {
      return;
    }
    if (syn_ == syn::yes) {
      out_stream_.reset();
      state_ = state::closed;
      return;
    }
    std::future<void> w = back_->push_buf(std::move(*buf_));
    buf_.reset();
    //under the closing state,buf_ is unavailable, but there are some data waiting to be written.
    state_ = state::closing;
    mut.unlock();
    w.get();//blocking until back thread write all data.

    mut.lock();
    release_assert(state_ == state::closing);
    out_stream_.reset();
    //under the closed state, buf_ is unabailable and no data need to be written in out_stream_.
    state_ = state::closed;
    reopen_or_close_cv_.notify_all();
  }
}
