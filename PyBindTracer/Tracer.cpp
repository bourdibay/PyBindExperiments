
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <iostream>
#include <string>
#include "Tracer.hpp"

/*
* Here is what we cannot trace:
*    - we cannot get the return value of a C/C++ function
*    - we cannot know the parameter values of a C/C++ function
*/

// This function has been conveniently copied from https://github.com/sumerc/yappi/blob/master/yappi/_yappi.c
static std::string getModuleName(PyCFunctionObject* cfn)
{
	PyObject* name = nullptr;

	// The __module__ attribute, can be anything
	PyObject* obj = cfn->m_module;

	if (!obj)
	{
		name = PyUnicode_FromString("__builtin__");
	}
	else if (PyUnicode_Check(obj))
	{
		Py_INCREF(obj);
		name = obj;
	}
	else if (PyModule_Check(obj))
	{
		const char* s = PyModule_GetName(obj);
		if (!s)
		{
			return "";
		}
		name = PyUnicode_FromString(s);
	}
	else
	{
		// Something else - str(obj)
		name = PyObject_Str(obj);
	}

	return PyUnicode_AsUTF8(name);
}

static int profileFunctionCalls(PyObject* obj, PyFrameObject* frame, int what, PyObject* arg)
{
	auto* tracerInformation = static_cast<TracerInformation*>(PyCapsule_GetPointer(obj, nullptr));

	PyCodeObject* code = PyFrame_GetCode(frame);
	PyObject* locals = PyFrame_GetLocals(frame);

	if (!tracerInformation
		|| !PyCode_Check(code)
		|| reinterpret_cast<PyObject*>(code) == Py_None
		|| strcmp(PyUnicode_AsUTF8(code->co_filename), tracerInformation->m_module.c_str()) != 0)
	{
		return 0;
	}

	std::string messageType;
	std::string messageContent;

	if (what == PyTrace_CALL)
	{
		messageType = "[Python function call]";

		const char* functionName = PyUnicode_AsUTF8(code->co_name);
		messageContent += functionName ? functionName : "";
	}
	else if (what == PyTrace_RETURN)
	{
		messageType = "[Python function return]";

		const char* functionName = PyUnicode_AsUTF8(code->co_name);
		messageContent += functionName ? functionName : "";

		const char* reprReturnObject = PyUnicode_AsUTF8(PyObject_Repr(arg));
		messageContent += " return the value ";
		messageContent += reprReturnObject ? reprReturnObject : "";
	}
	else if (what == PyTrace_C_CALL)
	{
		messageType = "[Python C function call]";
		PyCFunctionObject* cfunc = (PyCFunctionObject*)arg;

		const char* functionName = cfunc->m_ml->ml_name;
		std::string moduleName = getModuleName(cfunc);
		messageContent += functionName ? functionName : "";
		messageContent += " from module ";
		messageContent += moduleName;
	}
	else if (what == PyTrace_C_RETURN)
	{
		messageType = "[Python C function return]";
	}
	else if (what == PyTrace_EXCEPTION)
	{
		messageType = "[Python Exception]";
	}

	std::cout << messageType << ": " << messageContent << std::endl;

	return 0;
}

static int traceLineCalls(PyObject* obj,
	PyFrameObject* frame,
	int what,
	PyObject*)
{
	auto* tracerInformation = static_cast<TracerInformation*>(PyCapsule_GetPointer(obj, nullptr));

	PyCodeObject* code = PyFrame_GetCode(frame);
	PyObject* locals = PyFrame_GetLocals(frame);

	if (!tracerInformation
		|| !PyCode_Check(code)
		|| reinterpret_cast<PyObject*>(code) == Py_None
		|| strcmp(PyUnicode_AsUTF8(code->co_filename), tracerInformation->m_module.c_str()) != 0)
	{
		return 0;
	}

	std::string messageType;
	std::string messageContent;

	if (what == PyTrace_LINE)
	{
		messageType = "[Python Line executed]";
		const int lineNumber = PyFrame_GetLineNumber(frame);
		messageContent += "line ";
		messageContent += std::to_string(lineNumber);
	}

	std::cout << messageType << ": " << messageContent << std::endl;

	return 0;
}

void setTracers(PyObject* tracerInformation)
{
	PyEval_SetProfile(&profileFunctionCalls, tracerInformation);
	PyEval_SetTrace(&traceLineCalls, tracerInformation);
}

void removeTracers()
{
	PyEval_SetProfile(nullptr, nullptr);
	PyEval_SetTrace(nullptr, nullptr);
}
