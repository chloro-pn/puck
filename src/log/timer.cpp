#include "../../include/log/timer.h"
#include <string>
#include <cstdio>

//learn from muduo.timestamp --> https://github.com/chenshuo/muduo

namespace pnlog {
std::string timer::now() {
  thread_local time_t tlast_seconds_ {0};
  thread_local char tbuf_[64] = {0};
  timeval time;
  gettimeofday(&time,nullptr);
  time_t seconds = time.tv_sec;

  if (tlast_seconds_ != seconds) {
    tlast_seconds_ = seconds;
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    snprintf(tbuf_, sizeof(tbuf_), "%4d%02d%02d-%02d:%02d:%02d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec
           );
  }

  int microseconds = static_cast<int>(time.tv_usec);
  snprintf(tbuf_ + 17,sizeof(tbuf_) - 18,".%06d", microseconds);
  return tbuf_;
}
}

