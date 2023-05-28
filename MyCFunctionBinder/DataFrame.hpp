#pragma once

#include "Value.hpp"
#include <vector>

using Arguments = std::vector<Value>;

struct DataFrame
{
   Arguments m_argList;
   Value m_result;
};