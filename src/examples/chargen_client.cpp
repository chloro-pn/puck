#include "../../include/client.h"
#include "../../include/puck_signal.h"
#include "../../include/log/pnlog.h"
#include <cstdio>
#include <memory>

using namespace puck;

class chargenClient {
private:
  Client client_;
  std::shared_ptr<pnlog::CapTure> logger_;
  size_t sum_;

public:
  chargenClient(std::string ip, uint16_t port):client_(ip, port),
                                               logger_(pnlog::backend->get_capture(0)),
                                               sum_(0) {
    client_.setOnConnection([this](TcpConnection* ptr)->void {
      if(ptr->isReadComplete() == true) {
        if(ptr->isWriteComplete() == false) {
          logger_->warning("read complete before shutdown.");
          ptr->forceClose();
        }
        else {
          double mb = double(sum_) / (1024 * 1024);
          logger_->info(piece("totally received ", mb, " Mb."));
        }
        return;
      }
      logger_->info("new connection.");

      ptr->runAfter(3000, [ptr]()->bool {
                      ptr->shutdownWr();
                      return false;
                    });

    });

    client_.setOnMessage([this](TcpConnection* ptr)->void {
      sum_ += ptr->size();
      ptr->abandon(ptr->size());
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
  chargenClient client("127.0.0.1", 12345);
  client.bind(&pool);
  client.connect();
  pool.loop();
  return 0;
}
