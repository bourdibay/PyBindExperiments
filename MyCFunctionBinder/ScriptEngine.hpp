#pragma once

/*
* Contains the list of C++ functions mapped to their corresponding labels.
* This class should also execute the functions, but because this is just a PoC,
* this is not done here.
*/
#include "ScriptEngine.fwd.hpp"
#include <list>
#include <string>
#include <vector>
#include "Function.hpp"

using Functions = std::list<Function>;

class ScriptEngine
{
public:
   Function& declareFunctionOldSchool(
      const std::string& name,
      const OldSchoolFunction&,
      size_t nbArguments,
      const std::vector<Type>& arguments);


   Functions m_functions;
};

    