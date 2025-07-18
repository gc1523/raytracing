cmake_minimum_required(VERSION 3.16)
project(raytracing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your source files here
add_executable(raytracing
  main.cc
  # add other .cpp files as needed
)

# Optionally, add include directories
# target_include_directories(raytracing PRIVATE include)