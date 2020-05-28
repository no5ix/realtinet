// Copyright (C) 2018 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once


#include <stdint.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

//#include "Clock.h"
#include <assert.h>
#include <chrono>


// timer queue implemented with hashed hierarchical wheel.
//
// inspired by linux kernel, see links below
// https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git/tree/kernel/timer.c?h=v3.2.98
//
// We model timers as the number of ticks until the next
// due event. We allow 32-bits of space to track this
// due interval, and break that into 4 regions of 8 bits.
// Each region indexes into a bucket of 256 lists.
//
// complexity:
//      AddTimer   CancelTimer   PerTick
//       O(1)       O(1)          O(1)
//


// callback on timed-out
typedef std::function<void()> TimerCallback;

enum{
	WHEEL_BUCKETS = 4,
	TVN_BITS = 6,                   // time vector level shift bits
	TVR_BITS = 8,                   // timer vector shift bits
	TVN_SIZE = (1 << TVN_BITS),     // wheel slots of level vector
	TVR_SIZE = (1 << TVR_BITS),     // wheel slots of vector
	TVN_MASK = (TVN_SIZE - 1),      //
	TVR_MASK = (TVR_SIZE - 1),      //

	MAX_TVAL = ((uint64_t)((1ULL << (TVR_BITS + 4 * TVN_BITS)) - 1)),
};

int64_t CurrentTimeUnits(){
	//auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
}


class WheelTimer{
public:
	struct TimerNode{
		bool canceled = false;  // do lazy cancellation
		int id = -1;
		int64_t expire = -1;     // jiffies
		TimerCallback cb;
		int32_t repeat_num = 1;
		int64_t interval = -1;
		std::string key = "";
	};

	typedef std::vector<TimerNode*> TimerList;

	const int FREE_LIST_CAPACITY = 1024;

public:
	WheelTimer(int64_t now = 0);
	~WheelTimer();

	WheelTimer(const WheelTimer&) = delete;
	WheelTimer& operator=(const WheelTimer&) = delete;

	int Schedule(const std::string& key, float delay_time_unit, TimerCallback cb,
		int32_t repeat_num = 1, float first_delay_time_unit = 0, bool replace = true);
	int Schedule(float delay_time_unit, TimerCallback cb, const std::string& key = "",
		int32_t repeat_num = 1, float first_delay_time_unit = 0, bool replace = true);

	//int Schedule(uint32_t delay, TimerCallback cb);


	int Cancel(int id);
	int Cancel(const std::string& key);

	int Has(int id);
	int Has(const std::string& key);

	int Update(int64_t now = 0);

	int Size() const{ return size_; }

	void clearAll();

private:
	int nextCounter();
	int tick();
	void addTimerNode(TimerNode* node);
	int execute(int index);
	bool cascade(int bucket, int index);
	void clearList(TimerList& list);
	TimerNode* allocNode();
	void freeNode(TimerNode*);

private:
	int size_ = 0;
	int64_t current_ = 0;
	int64_t jiffies_ = 0;
	TimerList near_[TVR_SIZE];
	TimerList buckets_[WHEEL_BUCKETS][TVN_SIZE];
	std::unordered_map<int, TimerNode*> ref_;       // make O(1) searching
	std::unordered_map<std::string, TimerList> key2timer_list_map_;
	std::unordered_map<std::string, TimerNode*> key2timer_map_;
	TimerList free_list_;
	int counter_ = 0;   // next timer id
	int64_t next_tick_jiffy_ = 0;
};


class TimerHub{
protected:
	TimerHub(){};
public:
	static std::unique_ptr<TimerHub>& GetInstance(){
		static auto th = std::unique_ptr<TimerHub>(new TimerHub());
		return th;
	}

	TimerHub(const TimerHub&) = delete;
	TimerHub& operator=(const TimerHub&) = delete;

	void Add(WheelTimer* timer){ timer_hub_.push_back(timer); }
	void Tick(int64_t now = 0){
		for(const auto& timer : timer_hub_)
			timer->Update(now);
	}

private:
	std::vector<WheelTimer*> timer_hub_;

};



WheelTimer::WheelTimer(int64_t now)
//: current_(CurrentTimeUnits())
{
	if(now == 0)
		now = CurrentTimeUnits();
	current_ = now;
	ref_.rehash(64);            // reserve a little space
	key2timer_list_map_.rehash(64);            // reserve a little space
	key2timer_map_.rehash(64);            // reserve a little space
	free_list_.reserve(FREE_LIST_CAPACITY);

	TimerHub::GetInstance()->Add(this);
	//printf("wheel start at %lld\n", current_);
}


WheelTimer::~WheelTimer(){
	clearAll();
}

void WheelTimer::clearList(TimerList& list){
	for(auto ptr : list){
		delete ptr;
	}
	list.clear();
}

void WheelTimer::clearAll(){
	for(uint16_t i = 0; i < TVR_SIZE; ++i){
		clearList(near_[i]);
	}
	for(uint8_t i = 0; i < WHEEL_BUCKETS; ++i){
		for(uint8_t j = 0; j < TVN_SIZE; ++j){
			clearList(buckets_[i][j]);
		}
	}
	ref_.clear();
	clearList(free_list_);
	//for(auto node : free_list_){
		//delete node;
	//}
}

WheelTimer::TimerNode* WheelTimer::allocNode(){
	TimerNode* node = nullptr;
	if(free_list_.size() > 0){
		node = free_list_.back();
		free_list_.pop_back();
	} else{
		node = new TimerNode;
	}
	return node;
}


void WheelTimer::freeNode(TimerNode* node){
	if(free_list_.size() < free_list_.capacity()){
		free_list_.push_back(node);
	} else{
		delete node;
	}
}

void WheelTimer::addTimerNode(TimerNode* node){
	int64_t expires = node->expire;
	uint64_t idx = (uint64_t)(expires - jiffies_);
	TimerList* list = nullptr;
	if(idx < TVR_SIZE) // [0, 0x100)
	{
		int i = expires & TVR_MASK;  // 因为只关心后8位(即TVR_BITS=8)
		list = &near_[i];
		if (expires < next_tick_jiffy_) next_tick_jiffy_ = expires - i;
	} else if(idx < (1 << (TVR_BITS + TVN_BITS))) // [0x100, 0x4000)
	{
		// 因为不关心后8位(即TVR_BITS=8)的数, 所以直接 expires >> TVR_BITS 了
		// 又因为 TimerList buckets_[WHEEL_BUCKETS][TVN_SIZE] 的第二维为 TVN_SIZE, 所以要 & TVN_MASK
		int i = (expires >> TVR_BITS) & TVN_MASK;
		list = &buckets_[0][i];
	} else if(idx < (1 << (TVR_BITS + 2 * TVN_BITS))) // [0x4000, 0x100000)
	{
		int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
		list = &buckets_[1][i];
	} else if(idx < (1 << (TVR_BITS + 3 * TVN_BITS))) // [0x100000, 0x4000000)
	{
		int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
		list = &buckets_[2][i];
	} else if((int64_t)idx < 0){
		// Can happen if you add a timer with expires == jiffies,
		// or you set a timer to go off in the past
		int i = jiffies_ & TVR_MASK;
		list = &near_[i];
	} else{
		// If the timeout is larger than MAX_TVAL on 64-bit
		// architectures then we use the maximum timeout
		if(idx > MAX_TVAL){
			idx = MAX_TVAL;
			expires = idx + jiffies_;
		}
		int i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
		list = &buckets_[3][i];
	}
	// add to linked list
	list->push_back(node);
}

int WheelTimer::Schedule(const std::string& key, float delay_time_unit, TimerCallback cb,
	int32_t repeat_num, float first_delay_time_unit, bool replace){

	TimerNode* node = allocNode();
	node->canceled = false;
	node->cb = cb;
	node->key = key;
	if(repeat_num > 1 || repeat_num == -1)
		node->expire = jiffies_ + static_cast<int64_t>(first_delay_time_unit);
	else
		node->expire = jiffies_ + static_cast<int64_t>(delay_time_unit);

	node->id = nextCounter();
	node->interval = static_cast<int64_t>(delay_time_unit);
	node->repeat_num = (repeat_num == 0) ? 1 : repeat_num;


	addTimerNode(node);
	ref_[node->id] = node;

	if(replace){
		Cancel(key);
		key2timer_map_[key] = node;
	} else
		key2timer_list_map_[key].push_back(node);

	size_++;
	//printf("wheel node %d scheduled at %lld to %lld #%lld\n", node->id, current_, current_ + time_units, node->expire);
	return node->id;
}

int WheelTimer::Schedule(float delay_time_unit, TimerCallback cb, const std::string& key,
	int32_t repeat_num, float first_delay_time_unit, bool replace){
	return Schedule(key, delay_time_unit, cb, repeat_num, first_delay_time_unit, replace);
}

// Do lazy cancellation, so we can effectively use vector as container of timer nodes
int WheelTimer::Cancel(int id){
	TimerNode* node = ref_[id];
	if(node != nullptr){
		if(node->canceled)
			return 0;
		node->canceled = true;
		--size_;
		return 1;
	}
	return 0;
}

// Do lazy cancellation, so we can effectively use vector as container of timer nodes
int WheelTimer::Cancel(const std::string& key){
	int cancel_sum = 0;
	const auto& time_list_iter = key2timer_list_map_.find(key);
	if(time_list_iter != key2timer_list_map_.end()){
		for(const auto& node : time_list_iter->second){
			if(node->canceled)
				continue;
			node->canceled = true;
			--size_;
			++cancel_sum;
		}
	}
	auto& node = key2timer_map_[key];
	if(node){
		if(!node->canceled){
			node->canceled = true;
			--size_;
			++cancel_sum;
		}
	}
	return cancel_sum;
}

// Do lazy cancellation, so we can effectively use vector as container of timer nodes
int WheelTimer::Has(int id){
	TimerNode* node = ref_[id];
	//if(node){
	return node && !node->canceled;
		//if(node->canceled)
			//return F;
		//return 1;
	//}
	//return false;
}

// Do lazy cancellation, so we can effectively use vector as container of timer nodes
int WheelTimer::Has(const std::string& key){
	//int cancel_sum = 0;
	const auto& time_list_iter = key2timer_list_map_.find(key);
	if(time_list_iter != key2timer_list_map_.end()){
		for(const auto& node : time_list_iter->second){
			if(!node->canceled)
				return true;
				//continue;
			//node->canceled = true;
			//--size_;
			//++cancel_sum;
		}
	}
	auto& node = key2timer_map_[key];
	return node && !node->canceled;

	//if(node){
	//	return node.canceled
	//		if(!node->canceled){
	//			node->canceled = true;
	//			--size_;
	//			++cancel_sum;
	//		}
	//}
	//return false;
}

// cascade all the timers at bucket of index up one level
bool WheelTimer::cascade(int bucket, int index){
	// swap list
	TimerList list;
	buckets_[bucket][index].swap(list);

	for(auto& node : list){
		if(node->id > 0){
			addTimerNode(node);  // 把各个定时器往前推, 比如条件达成就挪到this->near_里去
		}
	}
	// 如INDEX(N), 当N=0, 因为进入本函数之前, jiffies_ & TVR_MASK 是为 0 的,
	// 说明 jiffies_ 8位以前的高位绝对有不为0的位,
	// jiffies右移8位然后跟TVN_MASK(即63, 即二进制111111, 六位)做且操作之后的结果 index == 0 ,
	// 则说明jiffies大于N=0的这个bucket区间了, 还需要调整下一个区间(即 N+1 这个bucket区间),
	// 就跟水表一样, 小表转一圈需要调整中表, 中表转一圈则要调整大表
	return index == 0;
}

// #define INDEX(N) (   ( jiffies_ >> (8 +  (N) * 6)  )        & 63)
#define INDEX(N) ((jiffies_ >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)

// cascades all vectors and executes all expired timer
int WheelTimer::tick(){
	//int fired = execute();
	int fired = 0;
	int index = jiffies_ & TVR_MASK;
	if(index == 0) // cascade timers
	{
		if(cascade(0, INDEX(0)) &&
			cascade(1, INDEX(1)) &&
			cascade(2, INDEX(2))
			)
			cascade(3, INDEX(3));
	}
#undef INDEX

	jiffies_++;
	fired += execute(index);
	return fired;
}

int WheelTimer::execute(int index){
	int fired = 0;
	// 因为都已经在 addTimerNode 里安插好了, 所以这里 jiffies_ & TVR_MASK 出来的index是几, 那就直接从near_里取出来执行就完事了
	//int index = jiffies_ & TVR_MASK;
	TimerList expired;
	near_[index].swap(expired); // swap list
	for(auto node : expired){
		if(!node->canceled && node->cb){
			//printf("wheel node %d triggered at %lld of jiffies %lld\n", node->id, current_, jiffies_);
			node->cb();
			size_--;
			fired++;
		}
		if(--node->repeat_num == 0){
			ref_.erase(node->id);
			freeNode(node);
			size_--;
		} else{
			node->expire = jiffies_ + node->interval;
			addTimerNode(node);
		}
	}
	return fired;
}

int WheelTimer::Update(int64_t now){
	if(now == 0){
		now = CurrentTimeUnits();
	}
	if(now < current_){
		assert(false && "(time go backwards) OR (init with now ts but update without now ts or vice versa)");
		current_ = now;
		return -1;
	} else if(now > current_){
		if(now < next_tick_jiffy_) {
			++jiffies_;
			return 0;
		}
		int ticks = (int)(now - current_);
		//int64_t ticks = now - current_;
		current_ = now;
		int fired = 0;
		for(int i = 0; i < ticks; i++){
			//printf("tick of jiffies %lld at %lld, %d/%d\n", jiffies_, current_, i, ticks);
			fired += tick();
		}
		return fired;
	}
	return 0;
}

// we assume you won't have too much timer in the same time
int WheelTimer::nextCounter(){
	return ++counter_;
}