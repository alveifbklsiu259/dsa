cmake_minimum_required(VERSION 3.10)

project(DSA)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# find_package(fmt CONFIG REQUIRED)
add_executable(DSA main.cpp ./DataStructure/LinkedList/Exception.cpp)


# target_link_libraries(HelloWorld PRIVATE fmt::fmt)