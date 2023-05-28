cmake_minimum_required(VERSION 3.10)
project(PybindExperiments CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(MyCFunctionBinder)
add_subdirectory(PyBindTracer)
