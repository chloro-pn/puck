#include "../../include/tcp_connection.h"
#include "../../include/tcp_server.h"
#include "../../include/puck_signal.h"
#include "../../include/log/pnlog.h"
#include <memory>
#include <set>
#include <mutex>

using namespace puck;

class chat_server {
private:
  TcpServer ts_;
  std::shared_ptr<pnlog::CapTure> logger_;
  std::set<TcpConnection*> conns_;
  std::mutex mut_;

  void addSession(TcpConnection* ptr) {
    std::unique_lock<std::mutex> mut(mut_);
    conns_.insert(ptr);
  }
  
  void eraseSession(TcpConnection* ptr) {
    std::unique_lock<std::mutex> mut(mut_);
    conns_.erase(ptr);
  }
  
  void broadcast(TcpConnection* ptr) {
    std::string message = ptr->iport();
    message.append(" : ");
    message.append(ptr->data(), ptr->size());
    ptr->abandon(ptr->size());

    std::unique_lock<std::mutex> mut(mut_);
    for(auto& each : conns_) {
      logger_->info(piece("send ", message, " to ", each->iport()));
      each->send(message.data(), message.size());
      Poller* loop = each->getLoop();
      each->getLoop()->push_func([loop, each]()->void {
        loop->change(each);
      });
      loop->wake_up();
    }
  }
  
public:
  explicit chat_server(uint16_t port):ts_(port), logger_(pnlog::backend->get_capture(0)) {
    logger_->enable_time();
    ts_.setOnConnection([this](TcpConnection* con)->void {
      if(con->isReadComplete() == true) {
        con->shutdownWr();
      }
      else {
        logger_->trace(piece("echo connection ", con->iport(), " start."));
        addSession(con);
      }
    });

    ts_.setOnMessage([this](TcpConnection* con)->void {
      broadcast(con);
    });

    ts_.setOnClose([this](TcpConnection* con)->void {
      if(con->getState() == TcpConnection::connState::succ_close) {
        logger_->trace("succ close!");
      }
      else {
        logger_->warning(piece("close state : ", con->getStateStr()));
      }
      eraseSession(con);
    });
  }

  void bind(EventLoop* loop) {
    ts_.bind(loop);
  }

  ~chat_server() {
    logger_->close();
  }
};

int main() {
  Signal::instance().ign(SIGPIPE);
  EventLoop pool(2);
  chat_server ds(12345);
  ds.bind(&pool);
  Signal::instance().handle(SIGINT, [&]()->void {
    pool.stop();
    exit(-1);
  });
  pool.start();
  return 0;
}
