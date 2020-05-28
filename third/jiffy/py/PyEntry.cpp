#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include <iostream>

#include "WheelTimer.h"

namespace py = pybind11;
template <typename... Args>
using overload_cast_ = pybind11::detail::overload_cast_impl<Args...>;


static auto& gTimerHub = TimerHub::GetInstance();


void Tick(int64_t now){ gTimerHub->Tick(now); }


int add(int i, int j){
	return i + j;
}

//class Pet
//{
//public:
//	Pet() : name("") {}
//	virtual void setName(const std::string &name_ = "") = 0;
//	virtual const std::string &getName() const = 0;
//
//	Pet(const Pet&) = delete;
//	Pet& operator=(const Pet&) = delete;
//
//	virtual ~Pet() {}
//	std::string name;
//};

struct Pet{
	Pet(const std::string &name, int age) : name(name), age(age){}

	void set(int age_, const std::string& mmp){ age = age_; }
	void set(const std::string &name_, uint32_t mmp){ name = name_; }

	std::string name;
	int age;
};

class Dog{
public:
	Dog(){}
	//~Dog() { std::cout << 'dog: bye" << std::endl; }
	~Dog(){ add(1, 2); }
	void setName(const std::string &name_ = ""){ name = name_; }
	const std::string &getName() const{ return name; }

//	std::string bark(uint32_t delay, const std::string& key, int32_t repeat_num, uint32_t first_delay)  const
//	{
//		std::string new_str = "";
//		//if (key == "")
//		//	new_str = "woof!";
//
//		//new_str = key + " " + std::to_string(repeat_num) + " " + std::to_string(first_delay);
//		return new_str; 
//	}
//
//	std::string bark(const std::string& key, uint32_t delay, int32_t repeat_num, uint32_t first_delay) const
//	{
//		std::string new_str = "";
//		//if (key == "")
//		//	new_str = "woof!";
//
//		//new_str = key + " " + std::to_string(repeat_num) + " " + std::to_string(first_delay);
//		return new_str;
//	}
//
//	std::string name;
//
//};


	std::string bark(uint32_t delay, const std::string& key = "", int32_t repeat_num = 1, uint32_t first_delay = 0) const{
		std::string new_str = "";
		if(key == "")
			new_str = "woof!";

		new_str = key + " " + std::to_string(repeat_num) + " " + std::to_string(first_delay);
		return new_str;
	}

	std::string bark(const std::string& key, uint32_t delay, int32_t repeat_num = 1, uint32_t first_delay = 0) const{
		std::string new_str = "";
		if(key == "")
			new_str = "woof!";

		new_str = key + " " + std::to_string(repeat_num) + " " + std::to_string(first_delay);
		return new_str;
	}

	std::string name;

};


PYBIND11_MODULE(_elapse, m){
	//py::class_<Pet>(m, "Pett")
	//	.def(py::init<const std::string &>())
	//	.def("setName", &Pet::setName)
	//	.def_readwrite("name", &Pet::name)
	//	.def("getName", &Pet::getName);

	py::class_<Pet>(m, "Pet")
		.def(py::init<const std::string &, int>())
		.def("set", overload_cast_<int, const std::string &>()(&Pet::set), "Set the pet's age")
		.def("set", overload_cast_<const std::string &, uint32_t>()(&Pet::set), "Set the pet's name");

	py::class_<Dog>(m, "Dogg")
		.def(py::init<>())
		.def("setName", &Dog::setName, py::arg("name") = "")
		//.def("bark", &Dog::bark, py::arg("key_name")="uui", py::arg("delay"), py::arg("repeat_cnt")=11, py::arg("first_delay")=100)
		.def("bark", overload_cast_<uint32_t, const std::string&, int32_t, uint32_t>()(&Dog::bark, py::const_),
			py::arg("delay"), py::arg("key_name") = "uui", py::arg("repeat_cnt") = 11, py::arg("first_delay") = 100, "Set the pesst's name")

		.def("bark", overload_cast_<const std::string&, uint32_t, int32_t, uint32_t>()(&Dog::bark, py::const_),
			py::arg("key_name") = "uui", py::arg("delay"), py::arg("repeat_cnt") = 11, py::arg("first_delay") = 100, "Set the pet's name")

		.def_readwrite("name", &Dog::name)
		.def("getName", &Dog::getName);


	py::class_<WheelTimer>(m, "CallbackManager")
		.def(py::init<int64_t>(), py::arg("now") = 0)
		//.def("Schedule", &WheelTimer::Schedule)

		.def("callback",
			overload_cast_<float, TimerCallback, const std::string&, int32_t, float, bool>()(&WheelTimer::Schedule),
			py::arg("delay"), py::arg("func"), py::arg("key") = "",
			py::arg("repeat_num") = 1, py::arg("first_delay") = 0, py::arg("replace") = true,
			"Set a timer")

		.def("callback",
			overload_cast_<const std::string&, float, TimerCallback, int32_t, float, bool>()(&WheelTimer::Schedule),
			py::arg("key") = "", py::arg("delay"), py::arg("func"),
			py::arg("repeat_num") = 1, py::arg("first_delay") = 0, py::arg("replace") = true,
			"Set a timer")

		.def("callback_anony",
			overload_cast_<float, TimerCallback, const std::string&, int32_t, float, bool>()(&WheelTimer::Schedule),
			py::arg("delay"), py::arg("func"), py::arg("key") = "",
			py::arg("repeat_num") = 1, py::arg("first_delay") = 0, py::arg("replace") = true,
			"Set a timer")

		.def("Size", &WheelTimer::Size)

		.def("cancel", overload_cast_<const std::string&>()(&WheelTimer::Cancel), "Cancel a timer by key")
		.def("cancel", overload_cast_<int>()(&WheelTimer::Cancel), "Cancel a timer by timer id")

		.def("has", overload_cast_<const std::string&>()(&WheelTimer::Has), "Check Has a timer by key")
		.def("has", overload_cast_<int>()(&WheelTimer::Has), "Check Has a timer by timer id")

		.def("cancel_all", &WheelTimer::clearAll)
		.def("destroy", &WheelTimer::clearAll)
		.def("Update", &WheelTimer::Update, py::arg("now") = 0);



	m.doc() = R"pbdoc(
        Pybind11 jiffy_py plugin
        -----------------------

        .. currentmodule:: jiffy_py

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

	m.def("pop_expires", &Tick, py::arg("now") = 0, R"pbdoc(
        removes all expired handles and return them, according to the given time.
    )pbdoc");

	m.def("add", &add, R"pbdoc(
        Add two numbers

        Some other explanation about the add function.
    )pbdoc");

	m.def("subtract", [](int i, int j){ return i - j; }, R"pbdoc(
        Subtract two numbers

        Some other explanation about the subtract function.
    )pbdoc");


	// exporting variables
	m.attr("the_answer") = 42;
	py::object world = py::cast("World");
	m.attr("what") = world;

#ifdef VERSION_INFO
	m.attr("__version__") = VERSION_INFO;
#else
	m.attr("__version__") = "dev";
#endif
}