// #include <iostream>
//
// using std::cout;
// using std::endl;
//
//
// ////
// //// server.cpp
// //// ~~~~~~~~~~
// ////
// //// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot
// com)
// ////
// //// Distributed under the Boost Software License, Version 1.0. (See
// accompanying
// //// file LICENSE_1_0.txt or copy at http://www.std.org/LICENSE_1_0.txt)
// ////
// //
// //#include <ctime>
// //#include <iostream>
// //#include <string>
// ////#include <std/array.hpp>
// ////#include <std/bind.hpp>
// ////#include <std/shared_ptr.hpp>
// ////#include <std/enable_shared_from_this.hpp>
// //#include <array>
// //#include <memory>
// //#include <asio.hpp>
// //
// //using asio::ip::tcp;
// //using asio::ip::udp;
// //
// //std::string make_daytime_string()
// //{
// //	using namespace std; // For time_t, time and ctime;
// //	time_t now = time(0);
// //	return ctime(&now);
// //}
// //
// //class tcp_connection
// //	: public std::enable_shared_from_this<tcp_connection>
// //{
// //public:
// //	typedef std::shared_ptr<tcp_connection> pointer;
// //
// //	static pointer create(asio::io_context& io_context)
// //	{
// //		return pointer(new tcp_connection(io_context));
// //	}
// //
// //	tcp::socket& socket()
// //	{
// //		return socket_;
// //	}
// //
// //	void start()
// //	{
// //		message_ = make_daytime_string();
// //
// //		asio::async_write(socket_, asio::buffer(message_),
// //			std::bind(&tcp_connection::handle_write,
// shared_from_this()));
// //	}
// //
// //private:
// //	tcp_connection(asio::io_context& io_context)
// //		: socket_(io_context)
// //	{
// //	}
// //
// //	void handle_write()
// //	{
// //	}
// //
// //	tcp::socket socket_;
// //	std::string message_;
// //};
// //
// //class tcp_server
// //{
// //public:
// //	tcp_server(asio::io_context& io_context)
// //		: acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
// //	{
// //		start_accept();
// //	}
// //
// //private:
// //	void start_accept()
// //	{
// //		tcp_connection::pointer new_connection =
// //
// tcp_connection::create(acceptor_.get_executor().context());
// //
// //		acceptor_.async_accept(new_connection->socket(),
// //			std::bind(&tcp_server::handle_accept, this,
// new_connection,
// //				std::placeholders::_1));
// //	}
// //
// //	void handle_accept(tcp_connection::pointer new_connection,
// //		const asio::error_code& error)
// //	{
// //		if (!error)
// //		{
// //			new_connection->start();
// //		}
// //
// //		start_accept();
// //	}
// //
// //	tcp::acceptor acceptor_;
// //};
// //
// //class udp_server
// //{
// //public:
// //	udp_server(asio::io_context& io_context)
// //		: socket_(io_context, udp::endpoint(udp::v4(), 13))
// //	{
// //		start_receive();
// //	}
// //
// //private:
// //	void start_receive()
// //	{
// //		socket_.async_receive_from(
// //			asio::buffer(recv_buffer_), remote_endpoint_,
// //			std::bind(&udp_server::handle_receive, this,
// //				std::placeholders::_1));
// //	}
// //
// //	void handle_receive(const asio::error_code& error)
// //	{
// //		if (!error)
// //		{
// //			std::shared_ptr<std::string> message(
// //				new std::string(make_daytime_string()));
// //
// //			socket_.async_send_to(asio::buffer(*message),
// remote_endpoint_,
// //				std::bind(&udp_server::handle_send, this,
// message));
// //
// //			start_receive();
// //		}
// //	}
// //
// //	void handle_send(std::shared_ptr<std::string> /*message*/)
// //	{
// //	}
// //
// //	udp::socket socket_;
// //	udp::endpoint remote_endpoint_;
// //	std::array<char, 1> recv_buffer_;
// //};
// //
// //int main()
// //{
// //	try
// //	{
// //		asio::io_context io_context;
// //		tcp_server server1(io_context);
// //		udp_server server2(io_context);
// //		io_context.run();
// //	}
// //	catch (std::exception& e)
// //	{
// //		std::cerr << e.what() << std::endl;
// //	}
// //
// //	return 0;
// //}
//
//
//
//
// //#include <chrono>
// //#include <iostream>
// //#include <string>
// ////#ifdef _WIN32
// ////#include <Windows.h>
// ////#define snprintf sprintf_s
// ////#endif
// ////#include "Logging.h"
// //
// //int64_t CurrentTimeMillis()
// //{
// //	auto now = std::chrono::system_clock::now();
// //	return
// std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
// //}
// //
// //enum
// //{
// //	WHEEL_BUCKETS = 4,
// //	TVN_BITS = 6,                   // time vector level shift bits
// //	TVR_BITS = 8,                   // timer vector shift bits
// //	TVN_SIZE = (1 << TVN_BITS),     // wheel slots of level vector
// //	TVR_SIZE = (1 << TVR_BITS),     // wheel slots of vector
// //	TVN_MASK = (TVN_SIZE - 1),      //
// //	TVR_MASK = (TVR_SIZE - 1),      //
// //
// //	MAX_TVAL = ((uint64_t)((1ULL << (TVR_BITS + 4 * TVN_BITS)) - 1)),
// //};
// //
// //class testcls
// //{
// //public:
// //	int gg;
// //};
// //
// //int main()
// //{
// //	{
// //		auto tt = testcls();
// //		testcls* tt_tpr = &tt;
// //		tt_tpr->gg = 0;
// //
// //		int testint = 1;
// //		int testttint = 2;
// //		const int* ptrr = &testint;
// //		ptrr = &testttint;
// //		//*ptrr = 3;
// //
// //
// //
// //
// //		std::string str1 = "uud";
// //		std::string str2 = "uud5";
// //
// //		std::cout << "int(str1 == str2 ): " << int(str1 == str2) <<
// std::endl;
// //
// //		uint8_t test_num = 254;
// //		uint8_t test_numss = 0;
// //		std::cout << +test_numss << std::endl;
// //
// //		int add_cnt = 0;
// //		for(uint8_t i = 0; i >= 0; --i){
// //			if(++add_cnt > 7)
// //				break;
// //			std::cout << +i << std::endl;
// //		}
// //		//for (uint8_t i = 254; i < TVR_SIZE; i++)
// //		//{
// //		//	if (++add_cnt > 7)
// //		//		break;
// //		//	std::cout << i << std::endl;
// //		//}
// //		std::cout << test_num + 2 << std::endl;
// //		std::cout << test_num + 3 << std::endl;
// //		std::cout << test_num + 4 << std::endl;
// //		std::cout << "test num finished\n";
// //
// //		int64_t now = CurrentTimeMillis();
// //		int64_t current_ = 1;
// //
// //		std::cout << "now: " << now << std::endl;
// //		std::cout << (int)(now - current_) << std::endl;
// //		int64_t current = 909909099;
// //		std::cout << std::string("time go backwards") +
// std::to_string(current).c_str() << std::endl;
// //		getchar();
// //		return 0;
// //	}
// //}
//
//
// //template <class I, class T>
// //void func_impl(I iter, T t){
// //	T tmp; // 这里就是迭代器所指物的类别
// //				 // ... 功能实现
// //	//return tmp;
// //}
// //
// //template <class T>
// //void func(T t){
// //	// !!!Wrong code
// //	 func_impl(t, *t); // forward the task to func_impl
// //}
// //
// //int main(){
// //	int i = 10;
// //	//cout <<
// //	func(&i);
// //		//<< endl; // !!! Can’t pass compile
// //}
//
// //template <class I>
// //inline
// //void func(I iter){
// //	func_impl(iter, *iter); // 传入iter和iter所指的值，class自动推导
// //}
// //
// //template <class I, class T>
// //void func_impl(I iter, T t){
// //	T tmp; // 这里就是迭代器所指物的类别
// //				 // ... 功能实现
// //}
// //
// //int main(){
// //	int i;
// //	func(&i);
// //}
//
//
// //template <class T>
// //struct MyIter{
// //	typedef T value_type; // 内嵌型别声明
// //	T* ptr;
// //	MyIter(T* p = 0) : ptr(p){}
// //	T& operator*() const{ return *ptr; }
// //};
// //
// //template <class I>
// //typename I::value_type func(I ite){
// //	std::cout << "class version" << std::endl;
// //	return *ite;
// //}
// //
// ////template <class I>
// ////(typename *I) func(I ite){
// ////	std::cout << "class version" << std::endl;
// ////	return *ite;
// ////}
// //
// //template <typename T>
// //T func(T* it){
// //	//std::cout <<
// //	return *it;
// //}
// //
// //int main(){
// //	// ...
// //	MyIter<int> ite(new int(8));
// //	cout << func(ite) << endl;    // 输出8
// //
// //	int *p = new int(5);
// //	cout << func(p) << endl; // error
// //
// //	getchar();
// //}
//
//
// //template <class T>
// //struct MyIter{
// //	typedef T value_type; // 内嵌型别声明
// //	T* ptr;
// //	MyIter(T* p = 0) : ptr(p){}
// //	T& operator*() const{ return *ptr; }
// //};
// //
// //// class type
// //template <class T>
// //struct iterator_traits{
// //	typedef typename T::value_type value_type;
// //};
// //
// //// 偏特化1
// //template <class T>
// //struct iterator_traits<T*>{
// //	typedef T value_type;
// //};
// //
// //// 偏特化2
// //template <class T>
// //struct iterator_traits<const T*>{
// //	typedef T value_type;
// //};
// //
// //template <class I>
// ////
// 首先询问iterator_traits<I>::value_type,如果传递的I为指针,则进入特化版本,iterator_traits直接回答;如果传递进来的I为class
// type,就去询问T::value_type.
// //typename iterator_traits<I>::value_type func(I ite){
// //	std::cout << "normal version" << std::endl;
// //	return *ite;
// //}
// //
// //int main(){
// //	// ...
// //	MyIter<int> ite(new int(8));
// //	std::cout << func(ite) << std::endl;
// //	int *p = new int(52);
// //	std::cout << func(p) << std::endl;
// //	const int k = 3;
// //	std::cout << func(&k) << std::endl;
// //	getchar();
// //}
//
//
// //#include <iostream>
// //using namespace std;
//
// /*先定义一些tag*/
// struct A{};
// struct B : A{}; // 继承的好处就是，当函数需要参数为A，
// 								//
// 而你传入的参数为B的时候，可以往上一直找到适合的对象
//
// 								/*假设有一个未知类*/
// template <class AorB>
// struct unknown_class{
// 	typedef AorB return_type;
// };
//
// /*特性萃取器*/
// template <class unknown_class>
// struct unknown_class_traits{
// 	typedef typename unknown_class::return_type return_type;
// };
//
// /*特性萃取器 —— 针对原生指针*/
// template <class T>
// struct unknown_class_traits<T*>{
// 	typedef T return_type;
// };
//
// /*特性萃取其 —— 针对指向常数*/
// template <class T>
// struct unknown_class_traits<const T*>{
// 	typedef const T return_type;
// };
//
//
// /*决定使用哪一个类型*/
// template <class unknown_class>
// inline typename unknown_class_traits<unknown_class>::return_type
// return_type(unknown_class){
// 	typedef typename unknown_class_traits<unknown_class>::return_type RT;
// 	return RT();
// }
//
// template <class unknown_class>
// inline typename unknown_class_traits<unknown_class>::return_type
// __func(unknown_class, A){
// 	cout << "use A flag" << endl;
// 	return A();
// }
//
// template <class unknown_class>
// inline typename unknown_class_traits<unknown_class>::return_type
// __func(unknown_class, B){
// 	cout << "use B flag" << endl;
// 	return B();
// }
//
// template <class unknown_class, class T>
// T
// __func(unknown_class, T){
// 	cout << "use origin ptr" << endl;
// 	return T();
// }
//
// template <class unknown_class>
// inline typename unknown_class_traits<unknown_class>::return_type
// func(unknown_class u){
// 	typedef typename unknown_class_traits<unknown_class>::return_type
// return_type; 	return __func(u, return_type());
// }
//
// int main(){
// 	{
// 		char buffer[512];   //chunk of memory内存池
// 		int *p1, *p2, *p3;
// 		//常规new:
// 		p1 = new int[10];
// 		//定位new:
// 		p2 = new (buffer) int[10];
// 		for (int i = 0; i < 10; ++i)
// 			p1[i] = p2[i] = 20 - i;
// 		cout << "p1 = " << p1 << endl;             //常规new指向的地址
// 		cout << "buffer = " <<(void *)buffer << endl; //内存池地址
// 		cout << "p2 = " << p2 << endl;             //定位new指向的地址
// 		cout << "p2[0] = " << p2[0] << endl;
// 		p3 = new (buffer) int[2];
// 		p3[0] = 1;
// 		p3[1] = 2;
// 		cout << "p3 = " << p3 << endl;
// 		cout << "p2[0] = " << p2[0] << endl;
// 		cout << "p2[1] = " << p2[1] << endl;
// 		cout << "p2[2] = " << p2[2] << endl;
// 		cout << "p2[3] = " << p2[3] << endl;
// 		getchar(); return 0;
// 	}
//
// 	{
// 		unknown_class<B> b;
// 		unknown_class<A> a;
// 		//unknown_class<int> i;
// 		int value = 1;
// 		int *p = &value;
//
// 		A v1 = func(a);
// 		B v2 = func(b);
// 		int v3 = func(p);
//
// 		char ch = getchar();
// 		return 0;
// 	}
//
// 	{
//
// 	}
// }

#include <asio.hpp>
// #include <shared_ptr.hpp>
#include <memory>
// #include <thread.hpp>
#include <mutex>
// #include <bind.hpp>
#include <iostream>

std::mutex global_stream_lock;

void WorkerThread(std::shared_ptr<asio::io_context> io_context) {
  global_stream_lock.lock();
  std::cout << "[" << std::this_thread::get_id() << "] Thread Start"
            << std::endl;
  global_stream_lock.unlock();

  io_context->run();

  global_stream_lock.lock();
  std::cout << "[" << std::this_thread::get_id() << "] Thread Finish"
            << std::endl;
  global_stream_lock.unlock();
}

void PrintNum(int x) {
  std::cout << "[" << std::this_thread::get_id() << "] x: " << x << std::endl;
}

class noncopyable {
 protected:
  noncopyable(int i): testint(i) { std::cout << i << " call noncopy cons\n"; };
  ~noncopyable() { std::cout << "call noncopy descons\n"; };

  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
  int testint;
};

class testnoncopy : private noncopyable {
 public:
  testnoncopy() : noncopyable(88) {
    std::cout << "call testnoncopy cons\n";
  }
  void testfunc1() { this->testint; }
};

class testnoncopyChild : public testnoncopy {
 public:
  testnoncopyChild()  {
    { std::cout << "call testnoncopyChild cons\n"; };
  }

  void testfunc2() {
    //     this->testint;
//     noncopyable();
  }
};

void test_join(int late){
	std::this_thread::sleep_for(std::chrono::seconds(late));
  std::cout << late << "fffff?\n";
}

int main(int argc, char* argv[]) {
  {
    // 	std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread th1([]() { test_join(4); });
    std::thread th2([]() { test_join(1); });
    std::cout << "bbbbb  ddone?\n";
    th2.join();
    std::cout << "ddone?\n";
    th1.join();
    std::cout << "ddoneww?\n";
    getchar();
    return 0;
  }

  {
    std::vector<int> v_int;
    //     v_int.resize(4);
    //     std::cout << v_int[2] << std::endl;

//     testnoncopy tn;
//     tn.testfunc1();
    testnoncopyChild tnc;
    tnc.testfunc2();
    std::cout << sizeof(tnc) << std::endl;
    getchar();
    return 0;
  }

  unsigned int n = std::thread::hardware_concurrency();
  std::cout << n << " concurrent threads are supported.\n";

  std::shared_ptr<asio::io_context> io_context(new asio::io_context);
  std::shared_ptr<asio::io_context::work> work(
      new asio::io_context::work(*io_context));
  asio::io_context::strand strand(*io_context);

  global_stream_lock.lock();
  std::cout << "[" << std::this_thread::get_id()
            << "] The program will exit when all  work has finished."
            << std::endl;
  global_stream_lock.unlock();

  // 	std::thread_group worker_threads;
  // 	for(int x = 0; x < 4; ++x) {
  // 		worker_threads.create_thread(std::bind(&WorkerThread,
  // io_context));
  // //一个service多个线程
  // 	}

  std::thread th1(std::bind(&WorkerThread, io_context));
  std::thread th2(std::bind(&WorkerThread, io_context));
  std::thread th3(std::bind(&WorkerThread, io_context));
  std::thread th4(std::bind(&WorkerThread, io_context));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  io_context->post(strand.wrap(std::bind(&PrintNum, 1)));
  io_context->post(strand.wrap(std::bind(&PrintNum, 2)));

  // 	std::this_thread::sleep_for(std::chrono::milliseconds(100));
  io_context->post(strand.wrap(std::bind(&PrintNum, 3)));
  io_context->post(strand.wrap(std::bind(&PrintNum, 4)));

  // 	std::this_thread::sleep_for(std::chrono::milliseconds(100));
  io_context->post(strand.wrap(std::bind(&PrintNum, 5)));
  io_context->post(strand.wrap(std::bind(&PrintNum, 6)));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  strand.dispatch(std::bind(&PrintNum, 100));
  strand.post(std::bind(&PrintNum, 101));
  strand.dispatch(std::bind(&PrintNum, 102));
  strand.post(std::bind(&PrintNum, 103));
  strand.dispatch(std::bind(&PrintNum, 104));
  strand.post(std::bind(&PrintNum, 105));
  strand.dispatch(std::bind(&PrintNum, 106));

  work.reset();

  // 	worker_threads.join_all();
  th1.join();
  th2.join();
  th3.join();
  th4.join();

  getchar();
  return 0;
}