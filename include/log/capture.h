#pragma once
#include "type.h"
#include "str_appender.h"
#include "timer.h"
#include <ctime>
#include <string>
#include <memory>
#include <mutex>
#include <sys/time.h>

namespace pnlog { 
  class BackEnd;

  std::string get_file_name(const char* path);

  class CapTure {
  public:
    enum class Level { PN_TRACE, PN_DEBUG, PN_INFO, PN_WARNING, PN_ERROR, PN_FATAL };

  private:
    std::shared_ptr<BackEnd> back_;
    Level default_level_;
    size_type index_;
    enum class time_record { on, off };
    time_record tflag_;

  public:
    static constexpr size_type buf_size_ = 4096;

    void close();

    void setLevel(Level l);

    Level getLevel();

    void enable_time() {
      tflag_ = time_record::on;
    }

    void disable_time() {
      tflag_ = time_record::off;
    }

    CapTure(const CapTure&) = delete;
    CapTure(CapTure&&) = delete;
    CapTure& operator=(const CapTure&) = delete;
    CapTure& operator=(CapTure&&) = delete;

    void trace(const std::string& str) {
      if (default_level_ <= Level::PN_TRACE) {
        log(index_, "TRACE", str);
      }
    }

    void trace(const std::string& str, const char* filepath, int line) {
      if(default_level_ <= Level::PN_TRACE) {
        char buf[32] = {0};
        sprintf(buf, "%d", line);
        std::string result;
        result.append("[");
        result.append(get_file_name(filepath));
        result.append("] [");
        result.append(buf);
        result.append("] ");
        result.append(str);
        trace(result);
      }
    }

    void debug(const std::string& str) {
      if (default_level_ <= Level::PN_DEBUG) {
        log(index_, "DEBUG", str);
      }
    }

    void debug(const std::string& str, const char* filepath, int line) {
      if(default_level_ <= Level::PN_DEBUG) {
        char buf[32] = {0};
        sprintf(buf, "%d", line);
        std::string result;
        result.append("[");
        result.append(get_file_name(filepath));
        result.append("] [");
        result.append(buf);
        result.append("] ");
        result.append(str);
        debug(result);
      }
    }

    void info(const std::string& str) {
      if (default_level_ <= Level::PN_INFO) {
        log(index_, "INFO", str);
      }
    }

    void info(const std::string& str, const char* filepath, int line) {
      if(default_level_ <= Level::PN_INFO) {
        char buf[32] = {0};
        sprintf(buf, "%d", line);
        std::string result;
        result.append("[");
        result.append(get_file_name(filepath));
        result.append("] [");
        result.append(buf);
        result.append("] ");
        result.append(str);
        info(result);
      }
    }

    void warning(const std::string& str) {
      if (default_level_ <= Level::PN_WARNING) {
        log(index_, "WARNING", str);
      }
    }

    void warning(const std::string& str, const char* filepath, int line) {
      if(default_level_ <= Level::PN_WARNING) {
        char buf[32] = {0};
        sprintf(buf, "%d", line);
        std::string result;
        result.append("[");
        result.append(get_file_name(filepath));
        result.append("] [");
        result.append(buf);
        result.append("] ");
        result.append(str);
        warning(result);
      }
    }

    void error(const std::string& str) {
      if (default_level_ <= Level::PN_ERROR) {
        log(index_, "ERROR", str);
      }
    }

    void error(const std::string& str, const char* filepath, int line) {
      if(default_level_ <= Level::PN_ERROR) {
        char buf[32] = {0};
        sprintf(buf, "%d", line);
        std::string result;
        result.append("[");
        result.append(get_file_name(filepath));
        result.append("] [");
        result.append(buf);
        result.append("] ");
        result.append(str);
        error(result);
      }
    }

    void fatal(const std::string& str) {
      if (default_level_ <= Level::PN_FATAL) {
        log(index_, "FATAL", str);
      }
    }

    void fatal(const std::string& str, const char* filepath, int line) {
      if(default_level_ <= Level::PN_FATAL) {
        char buf[32] = {0};
        sprintf(buf, "%d", line);
        std::string result;
        result.append("[");
        result.append(get_file_name(filepath));
        result.append("] [");
        result.append(buf);
        result.append("] ");
        result.append(str);
        fatal(result);
      }
    }

    ~CapTure();

  private:
    friend class BackEnd;
    CapTure(std::shared_ptr<BackEnd> b, size_type index);

    void log(size_type index, const char* level, const std::string& str);
  };
}//namespace pnlog
