#ifndef REALTINET_CORE_TCP_SERVER_HPP
#define REALTINET_CORE_TCP_SERVER_HPP

#include <memory>
#include <string>
#include <vector>

#include <asio.hpp>
#include <common/noncopyable.hpp>
#include "io_context_hub.hpp"

namespace realtinet {

class TcpServer {
 private:
  IoContextHub _io_ctx_hub;
  signal_set _signals;
  tcp::acceptor _acceptor;
  tcp::socket _socket;
  std::size_t _port;
  std::string _ip;

 public:
  TcpServer(asio::io_context& io_context, const tcp::endpoint& endpoint)
      : _signals(io_context),
        _acceptor(io_context),
        _socket(_io_ctx_hub.GetIoContext()) {
    _signals.add(SIGINT);
    _signals.add(SIGTERM);

#if defined(SIGQUIT)
    _signals.add(SIGQUIT);
#endif  // defined(SIGQUIT)

    _signals.async_wait(std::bind(&TcpServer::HandleStop, this));
    _signals.async_wait(
        std::bind(&asio::io_context::stop, std::ref(io_context)))

        do_accept();
  }

  void Bind(std::string ip, std::size_t port) {
    if (_ip == ip && _port == port) {
      return;
    };
    _ip = ip;
    _port = ip;

    //     asio::ip::tcp::resolver temp_resolver(_io_ctx_hub.GetIoContext());
    auto const address = asio::ip::make_address(ip);
    auto temp_endpoint = tcp::endpoint(address, port);
    _acceptor.open(temp_endpoint.protocol());
    _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.bind(temp_endpoint);
  }

  void Listen() {
    _acceptor.listen(/*asio::socket_base::max_listen_connections*/);
  }

  void run() { _io_ctx_hub.run(); }

 private:
  void do_accept() {
    _acceptor.async_accept(_socket, [this](asio::error_code ec) {
      if (!ec) {
        std::make_shared<session>(std::move(_socket))->start();
      }

      do_accept();
    });

    _socket = tcp::socket(_io_ctx_hub.get_io_context());
  }

  void HandleStop() {
    _acceptor.close();
    _io_ctx_hub.stop();

    std::cout << "TcpServer is graceful closed." << std::endl;
  }
};

}  // namespace realtinet

#endif  // REALTINET_CORE_TCP_SERVER_HPP
