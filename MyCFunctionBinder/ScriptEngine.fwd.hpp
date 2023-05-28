#pragma once

#include <functional>

class ScriptEngine;
struct DataFrame;

enum class Type
{
   Unknown = -1,
   String = 0,
   Integer,
   Numeric,
   Boolean,
   Pointer
};

using OldSchoolFunction = std::function<int(ScriptEngine*, DataFrame*)>;

