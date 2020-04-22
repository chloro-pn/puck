#include "../include/codec.h"
#include "../include/tcp_connection.h"

namespace puck {
Codec::Codec() {

}

void Codec::setOnMessage(callback_type ct) {
  on_message_ = ct;
}

void Codec::onMessage(TcpConnection* ptr) {
  //message_.clear();
  //message_.append(ptr->data(), ptr->size());
  //ptr->abandon(ptr->size());
  on_message_(ptr);
}

void Codec::setCodecError(TcpConnection* ptr) {
  ptr->setState(TcpConnection::connState::codec_error);
}

std::string Codec::what() const {
  return what_;
}

std::string Codec::encode(const char* ptr, size_t n) {
  std::string result;
  result.append(ptr, n);
  return result;
}
}
