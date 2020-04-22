#include "../../include/client.h"
#include "../../include/puck_signal.h"
#include <cstdio>

using namespace puck;

class myClient {
private:
  Client client_;

public:
  myClient(std::string ip, uint16_t port):client_(ip, port) {
    client_.setOnConnection([](TcpConnection* ptr)->void {
      if(ptr->isReadComplete() == true) {
        if(ptr->isWriteComplete() == false) {
          logger()->warning("endpoint close before shutdownwr.");
          ptr->forceClose();
        }
      }
      logger()->info("new connection.");
      ptr->send("begin");
    });
    client_.setOnMessage([](TcpConnection* ptr)->void {
      logger()->info("get message.");
      logger()->info(ptr->getCodec()->getMessage());
      char buf[128] = {0};
      gets(buf);
      if(buf[0] == 'q') {
        ptr->shutdownWr();
      }
      else {
        ptr->send(buf);
      }
    });
    client_.setOnClose([](TcpConnection* ptr)->void {
      logger()->info(piece("close , state : ", ptr->getStateStr()));
    });
  }

  void bind(Poller* pool) {
    client_.bind(pool);
  }

  void connect() {
    client_.connect();
  }
};

int main1() {
  Signal::instance().ign(SIGPIPE);
  Poller pool;
  Signal::instance().handle(SIGINT, [&]()->void {
    pool.stop();
    logger()->info("quit");
    exit(-1);
  });
  myClient client("127.0.0.1", 12345);
  client.bind(&pool);
  client.connect();
  pool.loop();
  return 0;
}
