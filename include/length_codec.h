#ifndef LENGTH_CODEC_H
#define LENGTH_CODEC_H

#include "codec.h"

namespace puck {
class LengthCodec : public Codec {
public:
  LengthCodec();

  void onMessage(TcpConnection *ptr) override;

  std::string encode(const char *ptr, size_t n) override;

  ~LengthCodec() = default;

private:
  using callback_type = Codec::callback_type;

  enum class state { waitingLength, waitingMessage };
  state state_;
  uint32_t length_;
};
}


#endif // MD5_CODEC_H
