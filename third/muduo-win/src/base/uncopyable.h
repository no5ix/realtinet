#ifndef CALM_BASE_UNCOPYABLE_H_
#define CALM_BASE_UNCOPYABLE_H_
namespace muduo {
	namespace detail
	{
		class uncopyable
		{
		protected:
			uncopyable() {}
			~uncopyable() {}
		private:
			uncopyable(const uncopyable&);
			uncopyable& operator=(const uncopyable&);
		};

		class noncopyable
		{
		protected:
			noncopyable() {}
			~noncopyable() {}
		private:
			noncopyable(const noncopyable&);
			noncopyable& operator=(const noncopyable&);
		};
	}

	typedef detail::uncopyable uncopyable;
	typedef detail::noncopyable noncopyable;

}
#endif //CALM_BASE_UNCOPYABLE_H_