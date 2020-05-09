#ifndef _IO_CTX_HANDLER
#define _IO_CTX_HANDLER

#include <memory>
#include <vector>

#include <asio.hpp>
#include <common/noncopyable.hpp>

class IoCtxHub() : private common::noncopyable {
 private:
  std::vector<std::thread> _threads;
  std::vector<asio::io_context> _io_contexts;
  std::vector<asio::executor_work_guard<asio::io_context::executor_type>>
      _ioc_work_guards;

  std::size_t _thread_size;
  std::shared_ptr<std::thread> _cur_thread;

 public:
  void IoCtxHandler() : thread_size_(std::thread::hardware_concurrency()) {}

  void set_thread_size(thread_size) { _thread_size = thread_size; }

  void Start() {
    _cur_thread.reset([this]() { _Start(); })
  }

  void Stop() {
    _ioc_work_guards.clear();
    for (auto &ioc : _io_contexts) ioc.stop();
    _cur_thread->join();
  }

 private:
  void _Start() {
    _io_contexts.reserve(_thread_size);
    _io_contexts.resize(_thread_size);

    _threads.reserve(_thread_size);
    _ioc_work_guards.reserve(_thread_size);

    for (std::size_t i = 0; i < _thread_size; ++i) {
      _ioc_work_guards.emplace_back(_io_contexts[i]);
      _threads.emplace_back([this, i]() { _io_contexts[i].run(); })
    }

    for (aut &th : _threads) th.join();
  }
}

#endif  // _IO_CTX_HANDLER
