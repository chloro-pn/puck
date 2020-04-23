#include "../../include/tcp_connection.h"
#include "../../include/tcp_server.h"
#include "../../include/puck_signal.h"
#include "../../include/log/pnlog.h"
#include "../../include/md5_codec.h"
#include <memory>
#include <chrono>

using namespace puck;

class echoServerMd5 {
private:
  TcpServer ts_;
  std::shared_ptr<pnlog::CapTure> logger_;

public:
  explicit echoServerMd5(uint16_t port):ts_(port), logger_(pnlog::backend->get_capture(0)) {
    logger_->enable_time();
    ts_.setOnConnection([this](TcpConnection* con)->void {
      if(con->isReadComplete() == true) {
        con->shutdownWr();
      }
      else {
        logger_->trace(piece("echo connection ", con->iport(), " start."));
        con->setCodec(new Md5Codec());
      }
    });

    ts_.setOnMessage([this](TcpConnection* con)->void {
      std::string str = con->getCodec()->getMessage();
      logger_->trace(piece("message get : ", str));
      con->send(str.data(), str.size());
    });

    ts_.setOnClose([this](TcpConnection* con)->void {
      if(con->getState() == TcpConnection::connState::succ_close) {
        logger_->trace("succ close!");
      }
      else {
        logger_->warning(piece("close state : ", con->getStateStr()));
      }
    });
  }

  void bind(EventLoop* loop) {
    ts_.bind(loop);
  }

  ~echoServerMd5() {
    logger_->close();
  }
};

int main() {
  Signal::instance().ign(SIGPIPE);
  EventLoop pool(2);
  echoServerMd5 ds(12345);
  ds.bind(&pool);
  Signal::instance().handle(SIGINT, [&]()->void {
    pool.stop();
    exit(-1);
  });
  pool.start();
  return 0;
}
