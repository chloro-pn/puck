#include "../include/codec.h"
#include "../include/tcp_connection.h"

namespace puck {
Codec::Codec() {

}

void Codec::setOnMessage(callback_type ct) {
  on_message_ = ct;
}

void Codec::setCodecError(TcpConnection* ptr) {
  ptr->setState(TcpConnection::connState::codec_error);
}

std::string Codec::what() const {
  return what_;
}
}
