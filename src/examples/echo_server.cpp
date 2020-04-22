#include "../../include/tcp_connection.h"
#include "../../include/tcp_server.h"
#include "../../include/console.h"
#include "../../include/puck_signal.h"
#include <memory>
#include <chrono>

using namespace puck;

class echo_server {
private:
  TcpServer ts_;

public:
  explicit echo_server(uint16_t port):ts_(port) {

    ts_.setOnConnection([](TcpConnection* con)->void {
      if(con->isReadComplete() == true) {
        con->shutdownWr();
      }
      else {
        logger()->trace("echo connection start.");
      }
    });

    ts_.setOnMessage([](TcpConnection* con)->void {
      logger()->trace(" message get!");
      con->send(con->data(), con->size());
      con->abandon(con->size());
    });
/*
    ts_.setOnClose([](TcpConnection* con)->void {
      if(con->getState() == TcpConnection::connState::succ_close) {
        //logger()->trace("succ close!");
      }
      else {
        logger()->warning(piece("close state : ", con->getStateStr()));
      }
    });
*/
    //ts_.openHeartBeat(2000);
  }

  void bind(EventLoop* loop) {
    ts_.bind(loop);
  }
};

int main() {
  pnlog::BackEnd::options op;
  op.asyn = true;
  pnlog::backend->open(op, 1, new pnlog::FileOutStream("log.txt"));

  Signal::instance().ign(SIGPIPE);
  logger()->enable_time();

  EventLoop pool(2);
  echo_server ds(12345);
  ds.bind(&pool);
  Signal::instance().handle(SIGINT, [&]()->void {
    pool.stop();
    exit(-1);
  });
  pool.start();
  return 0;
}
