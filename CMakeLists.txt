cmake_minimum_required(VERSION 3.10)
project(tixc)

file(GLOB_RECURSE SRC "tix.c" "src/*.c")
add_executable(tixc ${SRC})
target_include_directories(tixc PUBLIC "src/")

project(tix)
set(CMAKE_BUILD_TYPE Debug)
file(GLOB TSRC "rt/*.c")
add_library(tix ${TSRC})
target_link_libraries(tix PUBLIC c)

