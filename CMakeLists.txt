cmake_minimum_required(VERSION 3.16)

project(DSA LANGUAGES CXX)
include(FetchContent)

FetchContent_Declare(
  gsl
  GIT_REPOSITORY https://github.com/microsoft/GSL.git
  GIT_TAG v4.2.0)
FetchContent_MakeAvailable(gsl)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(DSA main.cpp ./DataStructure/LinkedList/Exception.cpp)
target_link_libraries(DSA PRIVATE Microsoft.GSL::GSL)
