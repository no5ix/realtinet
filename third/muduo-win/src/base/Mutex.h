#pragma once

#include <mutex>

namespace muduo
{

// Use as data member of a class, eg.
//
// class Foo
// {
//  public:
//   int size() const;
//
//  private:
//   mutable MutexLock mutex_;
//   std::vector<int> data_; // GUARDED BY mutex_
// };
#define MutexLock std::mutex

// Use as a stack variable, eg.
// int Foo::size() const
// {
//   MutexLockGuard lock(mutex_);
//   return data_.size();
// }
#define MutexLockGuard std::unique_lock<std::mutex>
}

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
//#define MutexLockGuard(x) error "Missing guard object name"