#ifndef CALM_BASE_TIMESTAMP_H_
#define CALM_BASE_TIMESTAMP_H_

#include <copyable.h>
#include <types.h>


namespace muduo
{
	class Timestamp :muduo::copyable
	{
	public:
		Timestamp()
			:microSecondsSinceEpoch_(0)
		{}
		explicit Timestamp(int64_t microSecondsSinceEpochArg)
			: microSecondsSinceEpoch_(microSecondsSinceEpochArg)
		{}
		void swap(Timestamp& that)
		{
			std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
		}
		std::string toString() const;
		std::string toFormattedString(bool showMicroseconds =true) const;
		bool valid() const{ return microSecondsSinceEpoch_ > 0; }

		int64_t microSecondsSinceEpoch() const
		{
			return microSecondsSinceEpoch_;
		}
		time_t secondsSinceEpoch()const
		{
			return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
		}
		static Timestamp now();
		static Timestamp invalid()
		{
			return Timestamp();
		}
		static Timestamp fromTime(time_t t)
		{
			fromTime(t, 0);
		}
		static Timestamp fromTime(time_t t, int microseconds)
		{
			return Timestamp(static_cast<int64_t>(t)*kMicroSecondsPerSecond + microseconds);
		}
		static const int kMicroSecondsPerSecond = 1000 * 1000;
		static const int kMicroSecondsPerMillisecond = 1000;
		
	private:
		int64_t microSecondsSinceEpoch_;
	};// end class Timestamp 


inline bool operator==(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}
inline bool operator!=(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() != rhs.microSecondsSinceEpoch();
}
inline bool operator<(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}
inline bool operator>(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch();
}
inline bool operator<=(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() <= rhs.microSecondsSinceEpoch();
}
inline bool operator>=(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() >= rhs.microSecondsSinceEpoch();
}
//get difference of two timestamp,result in seconds
inline double timeDifferenceSeconds(Timestamp high, Timestamp low)
{
	int64_t diff = static_cast<int64_t>(high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch());
	return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}
//get difference of two timestamp,result in ms
inline int timeDifferenceMs(Timestamp high, Timestamp low)
{
	int64_t diff = static_cast<int64_t>(high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch());
	return static_cast<int>(diff / Timestamp::kMicroSecondsPerMillisecond);
}
//add seconds to given timestamp
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
	int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
	return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}// end namespace muduo
#endif //CALM_BASE_TIMESTAMP_H_