#pragma once

#include <vector>

#include <asio.hpp>
#include <common/noncopyable.hpp>

class IoCtxHandler() : common::noncopyable {
 private:
  std::vector<std::thread> threads_;
  std::vector<asio::io_context> io_contexts_;
  std::vector<asio::executor_work_guard<asio::io_context::executor_type>>
      ioc_work_guards_;

  std::size_t thread_size_;
  std::thread 

 public:
  void IoCtxHandler() : thread_size_(std::thread::hardware_concurrency()) {}

  void set_thread_size(thread_size) { thread_size_ = thread_size; }

  void start() {

  }
}
