#include "../../include/client.h"
#include "../../include/puck_signal.h"
#include "../../include/log/pnlog.h"
#include <cstdio>
#include <memory>

using namespace puck;

class myClient {
private:
  Client client_;
  std::shared_ptr<pnlog::CapTure> logger_;

  void getFromConsoleAndSend(TcpConnection* ptr) {
    char buf[128] = {0};
    gets(buf);
    if(buf[0] == 'q') {
      ptr->shutdownWr();
    }
    else {
      ptr->send(buf);
    }
  }

public:
  myClient(std::string ip, uint16_t port):client_(ip, port),logger_(pnlog::backend->get_capture(0)) {
    client_.setOnConnection([this](TcpConnection* ptr)->void {
      if(ptr->isReadComplete() == true) {
        if(ptr->isWriteComplete() == false) {
          logger_->warning("endpoint close before shutdownwr.");
          ptr->forceClose();
        }
        return;
      }
      logger_->info("new connection.");
      getFromConsoleAndSend(ptr);
    });

    client_.setOnMessage([this](TcpConnection* ptr)->void {
      std::string ret;
      ret.append(ptr->data(), ptr->size());
      logger_->info(piece("get message : ", ret));
      ptr->abandon(ptr->size());
      getFromConsoleAndSend(ptr);
    });

    client_.setOnClose([](TcpConnection* ptr)->void {
      logger()->info(piece("close , state : ", ptr->getStateStr()));
    });
  }

  void bind(Poller* pool) {
    client_.bind(pool);
  }

  void connect() {
    client_.connect(3000);
  }
};

int main() {
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
