/*
 * Copyright (c) 2018 Ally of Intelligence Technology Co., Ltd. All rights
 * reserved.
 *
 * Created by WuKun on 5/28/19.
 * Contact with:wk707060335@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http: *www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/noncopyable.hpp>

using boost::asio::signal_set;
using boost::asio::steady_timer;
using boost::asio::ip::tcp;

typedef std::shared_ptr<boost::asio::io_context> io_context_ptr;
typedef boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
    io_context_work;

/// A pool of io_context objects.
class io_context_pool : private boost::noncopyable {
 public:
  /// Construct the io_context pool.
  explicit io_context_pool() : next_io_context_(0) {
    //        const auto pool_size = std::thread::hardware_concurrency();
    const auto pool_size = 3;

    // Give all the io_contexts work to do so that their run() functions will
    // not exit until they are explicitly stopped.
    for (std::size_t i = 0; i < pool_size; ++i) {
      io_context_ptr io_context(new boost::asio::io_context);
      io_contexts_.push_back(io_context);
      work_.push_back(boost::asio::make_work_guard(*io_context));
    }
  }

  /// Run all io_context objects in the pool.
  void run() {
    // Create a pool of threads to run all of the io_contexts.
    std::vector<std::shared_ptr<std::thread>> threads;
    for (const auto& io_context : io_contexts_) {
      std::shared_ptr<std::thread> thread(new std::thread(
          std::bind(&boost::asio::io_context::run, io_context)));
      threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (const auto& thread : threads) thread->join();
  }

  /// Stop all io_context objects in the pool.
  void stop() {
    // Explicitly stop all io_contexts.
    for (const auto& io_context : io_contexts_) io_context->stop();
  }

  /// Get an io_context to use.
  boost::asio::io_context& get_io_context() {
    // Use a round-robin scheme to choose the next io_context to use.
    boost::asio::io_context& io_context = *io_contexts_[next_io_context_];
    ++next_io_context_;
    if (next_io_context_ == io_contexts_.size()) next_io_context_ = 0;
    return io_context;
  }

 private:
  /// The pool of io_contexts.
  std::vector<io_context_ptr> io_contexts_;

  /// The work that keeps the io_contexts running.
  std::vector<io_context_work> work_;

  /// The next io_context to use for a connection.
  std::size_t next_io_context_;
};

class session : public std::enable_shared_from_this<session> {
 public:
  explicit session(tcp::socket socket) : socket_(std::move(socket)) {}

  void start() { do_read(); }

 private:
  void do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            do_write(length);
          }
        });
  }

  void do_write(std::size_t length) {
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_, boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            do_read();
          }
        });
  }

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server {
 public:
  server(boost::asio::io_context& io_context, const tcp::endpoint& endpoint)
      : signals_(io_context),
        acceptor_(io_context),
        socket_(context_pool_.get_io_context()) {
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif  // defined(SIGQUIT)
    signals_.async_wait(std::bind(&server::handle_stop, this));
    signals_.async_wait(
        std::bind(&boost::asio::io_context::stop, std::ref(io_context)));

    // Open the acceptor with the option to reuse the address (i.e.
    // SO_REUSEADDR).
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(boost::asio::socket_base::max_listen_connections);

    do_accept();
  }

  void run() {
    // The io_context::run() call will block until all asynchronous operations
    // have finished. While the server is running, there is always at least one
    // asynchronous operation outstanding: the asynchronous accept call waiting
    // for new incoming connections.
    context_pool_.run();
  }

 private:
  void do_accept() {
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
      if (!ec) {
        std::make_shared<session>(std::move(socket_))->start();
      }

      do_accept();
    });

    // hand the socket to a io_context in pool
    // each io_context object run on their own thread
    socket_ = tcp::socket(context_pool_.get_io_context());
  }

  void handle_stop() {
    // The server is stopped by cancelling all outstanding asynchronous
    // operations. Once all operations have finished the io_context::run()
    // call will exit.
    acceptor_.close();
    context_pool_.stop();

    std::cout << "server is graceful closed." << std::endl;
  }

  /// The signal_set is used to register for process termination notifications.
  io_context_pool context_pool_;
  signal_set signals_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

int main(int argc, char* argv[]) {
  try {
    auto const address = boost::asio::ip::make_address("0.0.0.0");
    auto const port = 8888;

    boost::asio::io_context io_context;

    server s(io_context, tcp::endpoint{address, port});
    std::thread io_thread([&]() { s.run(); });

    io_context.run();
    io_thread.join();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}