#include <count_down_latch.h>

using namespace muduo;
CountDownLatch::CountDownLatch(int count)
	:mutex_(),
	 condition_(),
	 count_(count)
{}

void CountDownLatch::countDown()
{
	std::unique_lock<std::mutex> lck(mutex_);
	count_--;
	if (count_ == 0)
		condition_.notify_all();
}
int CountDownLatch::getCount() const
{
	std::unique_lock<std::mutex> lck(mutex_);
	return count_;
}
void CountDownLatch::wait()
{
	std::unique_lock<std::mutex> lck(mutex_);
	while (count_ > 0)
		condition_.wait(lck);
}