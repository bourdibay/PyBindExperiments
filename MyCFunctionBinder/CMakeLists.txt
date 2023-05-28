cmake_minimum_required(VERSION 3.10)
project(MyCFunctionBinder CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(MyCFunctionBinder
  DataFrame.hpp
  Function.cpp
  Function.hpp
  Main.cpp
  ScriptEngine.cpp
  ScriptEngine.fwd.hpp
  ScriptEngine.hpp
  ScriptFunctionsBinding.hpp
  Value.cpp
  Value.hpp
)