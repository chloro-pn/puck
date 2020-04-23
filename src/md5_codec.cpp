#include "../include/md5_codec.h"
#include "../include/tcp_connection.h"
#include "../include/sockets.h"
#include "../third_party/md5.h"
#include <cassert>

namespace puck {
Md5Codec::Md5Codec():state_(state::waitingLength),length_(0) {

}

void Md5Codec::onMessage(TcpConnection *ptr) {
  if(state_ == state::waitingLength) {
    if(ptr->size() < sizeof(length_)) {
      return;
    }
    else {
      memcpy(&length_, ptr->data(), sizeof(length_));
      ptr->abandon(sizeof(length_));
      length_ = sockets::networkToHost(length_);
      if(length_ < 32 || length_ > 8096) {
        setCodecError(ptr);
        return;
      }
      state_ = state::waitingMessage;
    }
  }
  if(state_ == state::waitingMessage) {
    if(ptr->size() < length_) {
      return;
    }
    else {
      message_.clear();
      message_.append(ptr->data(), length_);
      ptr->abandon(length_);
      length_ -= 32;
      std::string md5 = message_.substr(length_, 32);
      message_ = message_.substr(0, length_);
      if(MD5(message_).toStr() != md5) {
        logger()->error(piece("md5 check error : ", md5, " - ", MD5(message_).toStr()));
        setCodecError(ptr);
        return;
      }
      length_ = 0;
      state_ = state::waitingLength;
      on_message_(ptr);
    }
  }
}

std::string Md5Codec::encode(const char *ptr, size_t n) {
  std::string len;
  uint32_t length = n + 32;
  len.resize(sizeof(length));
  length = sockets::hostToNetwork(length);
  memcpy(&(*len.begin()), &length, sizeof(length));
  std::string message;
  message.append(ptr, n);
  std::string result;
  result.append(len);
  result.append(message);
  result.append(MD5(message).toStr());
  return result;
}
}
