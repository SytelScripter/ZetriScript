# Set the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(ZetriScript VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable target
add_executable(ZetriScript parser.cpp) # for testing
