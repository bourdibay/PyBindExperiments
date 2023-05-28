
#pragma once

#include <string>
#include <pytypedefs.h>

struct TracerInformation
{
	std::string m_module;
};

void setTracers(PyObject * TracerInformation);
void removeTracers();
