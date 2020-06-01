#include "../include/tcp_connection.h"
#include "../include/tcp_server.h"
#include "../include/puck_signal.h"
#include "../include/log/pnlog.h"
#include <ctime>
#include <memory>

using namespace puck;

class daytime_server {
private:
  TcpServer ts_;
  std::shared_ptr<pnlog::CapTure> logger_;

public:
  explicit daytime_server(uint16_t port):ts_(port), logger_(pnlog::backend->get_capture(0)) {
    logger_->enable_time();
    ts_.setOnConnection([this](TcpConnection* con)->void {
      if(con->isReadComplete() == true) {
        if(con->isWriteComplete() == false) {
          logger_->warning("endpoint close before shutdownwr.");
          con->forceClose();
        }
      }
      else {
        logger_->trace(piece("echo connection ", con->iport(), " start."));
        std::string t = now();
        con->send(t.data(), t.size());
        con->shutdownWr();
      }
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

  std::string now() const {
    time_t t;
    t = time(nullptr);
    std::string ret(ctime(&t));
    return ret;
  }

  void bind(EventLoop* loop) {
    ts_.bind(loop);
  }

  ~daytime_server() {
    logger_->close();
  }
};

int main() {
  Signal::instance().ign(SIGPIPE);
  EventLoop pool(2);
  daytime_server ds(12345);
  ds.bind(&pool);
  Signal::instance().handle(SIGINT, [&]()->void {
    pool.stop();
    exit(-1);
  });
  pool.start();
  return 0;
}
