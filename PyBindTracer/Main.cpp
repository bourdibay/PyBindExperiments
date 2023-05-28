
#include <pybind11/embed.h>
#include <iostream>
#include <string>
#include "Tracer.hpp"

/*
* Create basic Python class and module
*/
struct PyInt
{
	int get() { return 1; }
};

static PyInt createPyInt()
{
	return PyInt();
}

PYBIND11_EMBEDDED_MODULE(PyInt, m)
{
	auto date = pybind11::class_<PyInt>(m, "PyInt");
	date.def("get", &PyInt::get);
	m.def("CreatePyInt", &createPyInt);
}

PYBIND11_EMBEDDED_MODULE(PyFunctions, m)
{
	m.def("add", [](int i, int j) {
		std::cout << "[ADD]: " << i << " + " << j << std::endl;
		return i + j;
		});

	m.def("printString", [](std::string str) {
		std::cout << "[PRINT_STRING]: " << str << std::endl;
		return str;
		});
}

/*
* Python source code to trace
*/
static const std::string pythonSourceCode
= R"(
import PyFunctions
import PyInt

i = PyInt.CreatePyInt()
value = i.get()
print(i)

def my_local_fonction(unused_arg):
   print("Call to my_local_fonction with parameter " + str(unused_arg))
   return False

my_var = "Hello"
an_int = 99
ret = PyFunctions.add(1, 2)
another_int = an_int + 7

l = [c for c in my_var if c >= 'l']

PyFunctions.printString("Hey")

print("I am calling my_local_fonction")
my_local_fonction("my_param")
print("I am calling PyFunctions.add")
res = PyFunctions.add(1, 2)

print("The result of the addition is " + str(res))

# Test python built-in functions
fd = open("hello.txt", "w+")
fd.close()
)";

int main()
{
	pybind11::scoped_interpreter guard{};

	const char* filename = "MyFile";
	TracerInformation tracerInformation;
	tracerInformation.m_module = filename;

	PyObject * tracerInformationCapsule = PyCapsule_New(&tracerInformation, nullptr, nullptr);
	setTracers(tracerInformationCapsule);

	try
	{
		pybind11::object globals = pybind11::globals();
		pybind11::object locals = globals;

		pybind11::detail::ensure_builtins_in_globals(globals);

		std::string buffer = "# -*- coding: utf-8 -*-\n" + pythonSourceCode;

		auto bytecode = pybind11::reinterpret_steal<pybind11::object>(
			Py_CompileString(buffer.c_str(), filename, Py_file_input));

		if (!bytecode)
		{
			throw pybind11::error_already_set();
		}

		pybind11::object result
			= pybind11::reinterpret_steal<pybind11::object>(
				PyEval_EvalCode(bytecode.ptr(), globals.ptr(), locals.ptr()));

		if (!result)
		{
			throw pybind11::error_already_set();
		}

	}
	catch (const pybind11::error_already_set & e)
	{
		std::cerr << e.what() << std::endl;
	}

	removeTracers();

	Py_DECREF(tracerInformationCapsule);

	return 0;
}
