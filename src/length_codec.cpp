#include "../include/length_codec.h"
#include "../include/tcp_connection.h"
#include "../include/sockets.h"
#include <cassert>

namespace puck {
LengthCodec::LengthCodec():state_(state::waitingLength),length_(0) {

}

void LengthCodec::onMessage(TcpConnection *ptr) {
  if(state_ == state::waitingLength) {
    if(ptr->size() < sizeof(length_)) {
      return;
    }
    else {
      memcpy(&length_, ptr->data(), sizeof(length_));
      ptr->abandon(sizeof(length_));
      length_ = sockets::networkToHost(length_);
      if(length_ > 8096) {
        what_ = piece("length error : ", length_);
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
      length_ = 0;
      state_ = state::waitingLength;
      on_message_(ptr);
    }
  }
}

std::string LengthCodec::encode(const char *ptr, size_t n) {
  std::string len;
  uint32_t length = n;
  len.resize(sizeof(length));
  length = sockets::hostToNetwork(length);
  memcpy(&(*len.begin()), &length, sizeof(length));
  std::string message;
  message.append(ptr, n);
  std::string result;
  result.append(len);
  result.append(message);
  return result;
}
}
