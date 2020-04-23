#ifndef CODEC_H
#define CODEC_H

#include <functional>

namespace puck {
class TcpConnection;

class Codec {
protected:
  using callback_type = std::function<void(TcpConnection*)>;

  callback_type on_message_;
  std::string what_;
  std::string message_;

public:
  Codec();

  void setOnMessage(callback_type ct);

  const std::string& getMessage() const {
    return message_;
  }

  virtual void onMessage(TcpConnection* ptr) = 0;

  std::string what() const ;

  void setCodecError(TcpConnection* ptr);

  virtual std::string encode(const char* ptr, size_t n) = 0;

  virtual ~Codec() = default;
};
}

#endif // CODEC_H
