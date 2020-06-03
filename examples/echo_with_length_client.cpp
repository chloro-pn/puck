#include "../include/client.h"
#include "../include/puck_signal.h"
#include "../include/log/pnlog.h"
#include "../include/length_codec.h"
#include <iostream>
#include <memory>

using namespace puck;

class myClientLength {
private:
  Client client_;
  std::shared_ptr<pnlog::CapTure> logger_;

  void getFromConsoleAndSend(TcpConnection* ptr) {
    char buf[128] = {0};
    std::cin.getline(buf,128);
    if(buf[0] == 'q') {
      ptr->shutdownWr();
    }
    else {
      ptr->send(buf);
    }
  }

public:
  myClientLength(std::string ip, uint16_t port):client_(ip, port),logger_(pnlog::backend->get_capture(0)) {
    client_.setOnConnection([this](TcpConnection* ptr)->void {
      if(ptr->isReadComplete() == true) {
        if(ptr->isWriteComplete() == false) {
          logger_->warning("endpoint close before shutdownwr.");
          ptr->forceClose();
        }
        return;
      }
      logger_->info("new connection.");
      ptr->setCodec(new LengthCodec());
      getFromConsoleAndSend(ptr);
    });

    client_.setOnMessage([this](TcpConnection* ptr)->void {
      std::string ret = ptr->getCodec()->getMessage();
      logger_->info(piece("get message : ", ret));
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
  myClientLength client("127.0.0.1", 50011);
  client.bind(&pool);
  client.connect();
  pool.loop();
  return 0;
}
