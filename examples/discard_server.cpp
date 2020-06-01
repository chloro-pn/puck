#include "../include/tcp_connection.h"
#include "../include/tcp_server.h"
#include "../include/puck_signal.h"
#include "../include/log/pnlog.h"
#include <memory>

using namespace puck;

class discard_server {
private:
  TcpServer ts_;
  std::shared_ptr<pnlog::CapTure> logger_;

public:
  explicit discard_server(uint16_t port):ts_(port), logger_(pnlog::backend->get_capture(0)) {
    logger_->enable_time();
    ts_.setOnConnection([this](TcpConnection* con)->void {
      if(con->isReadComplete() == true) {
        con->shutdownWr();
      }
      else {
        logger_->trace(piece("echo connection ", con->iport(), " start."));
      }
    });

    ts_.setOnMessage([this](TcpConnection* con)->void {
      size_t n = con->size();
      con->abandon(con->size());
      logger_->trace(piece("discards ", n, " bytes."));
      con->send(piece("discards ", n, " bytes.\n").c_str());
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

  ~discard_server() {
    logger_->close();
  }
};

int main() {
  Signal::instance().ign(SIGPIPE);
  EventLoop pool(2);
  discard_server ds(12345);
  ds.bind(&pool);
  Signal::instance().handle(SIGINT, [&]()->void {
    pool.stop();
    exit(-1);
  });
  pool.start();
  return 0;
}
