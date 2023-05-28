cmake_minimum_required(VERSION 3.10)
project(PyBindTracer CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(PythonLibs REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../pybind11/include)

add_executable(PyBindTracer
    Main.cpp
    Tracer.cpp
    Tracer.hpp
)

target_link_libraries(PyBindTracer ${PYTHON_LIBRARIES})
