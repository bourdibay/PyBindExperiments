#pragma once

#include "ScriptEngine.fwd.hpp"
#include <compare>
#include <string>
#include <vector>

struct Function
{
   std::strong_ordering operator<=>(const Function&) const;
   bool operator==(const Function&) const;

   std::string m_name;
   size_t m_arity;
   std::vector<Type> m_argTypeList; // first element is the return type
   OldSchoolFunction m_callback;
};

