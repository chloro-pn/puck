#include "../../include/log/capture.h"
#include "../../include/log/back_end.h"
#include "../../include/log/convert.h"
#include "../../include/log/char_array_wrapper.h"
#include <string>


namespace pnlog {
  constexpr size_type CapTure::buf_size_;

  std::string get_file_name(const char* path) {
    const char* ptr = path;
    size_t index = 0;
    size_t begin = index;
    while(*(ptr + begin) != '\0') {
      if(*(ptr + begin) == '/') {
        index = begin;
      }
      ++begin;
    }
    return std::string(static_cast<const char*>(ptr + index + 1));
  }

  CapTure::CapTure(std::shared_ptr<BackEnd> b, size_type ind)
      :back_(b), default_level_(Level::PN_TRACE), index_(ind), tflag_(time_record::off) {

  }

  void CapTure::close() {
    back_->close(index_);
  }

  void CapTure::log(size_type index, const char* level, const std::string& str) {
    CharArrayWrapper tmp(-1);
    if (back_->rangecheck(index) == false) {
      fprintf(stderr,"index out of range !");
      std::abort();
    }
    if(tflag_ == time_record::on) {
      tmp.get()->append("[");
      tmp.get()->append(timer::instance().now());
      tmp.get()->append("] ");
    }
    tmp.get()->append("[");
    tmp.get()->append(level);
    tmp.get()->append("] ");
    tmp.get()->append(str.c_str());
    tmp.get()->append("\n");
    if (tmp.get()->error() == true) {
      back_->abort("log is too long!\n");
    }
    back_->write(index, tmp.get()->getBuf(), tmp.get()->getSize());
    if (*level == 'F') { // fatal.
      back_->abort(piece("log fatal from file : ",__FILE__, ", line : ", __LINE__));
    }
  }

  void CapTure::setLevel(Level l) {
    default_level_ = l;
  }

  CapTure::Level CapTure::getLevel() {
    return default_level_;
  }

  CapTure::~CapTure() {

  }
}//namespace pnlog
