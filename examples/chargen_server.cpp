#include "../include/tcp_connection.h"
#include "../include/tcp_server.h"
#include "../include/puck_signal.h"
#include "../include/log/pnlog.h"
#include <memory>

using namespace puck;

class chargen_server {
private:
  TcpServer ts_;
  std::shared_ptr<pnlog::CapTure> logger_;

public:
  explicit chargen_server(uint16_t port):ts_(port), logger_(pnlog::backend->get_capture(0)) {
    logger_->enable_time();
    ts_.setOnConnection([this](TcpConnection* con)->void {
      if(con->isReadComplete() == true) {
        logger_->info("read complete.");
        con->shutdownWr();
      }
      else {
        logger_->trace(piece("echo connection ", con->iport(), " start."));
        std::string ret;
        ret.resize(1024, 'a');
        con->send(ret.data(), 1024);
      }
    });

    ts_.setOnMessage([this](TcpConnection* con)->void {
      con->abandon(con->size());
    });

    ts_.setOnWriteComplete([this](TcpConnection* con)->void {
      std::string ret;
      ret.resize(1024, 'a');
      con->send(ret.data(), 1024);
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

  ~chargen_server() {
    logger_->close();
  }
};

int main() {
  Signal::instance().ign(SIGPIPE);
  EventLoop pool(2);
  chargen_server ds(12345);
  ds.bind(&pool);
  Signal::instance().handle(SIGINT, [&]()->void {
    pool.stop();
    exit(-1);
  });
  pool.start();
  return 0;
}
