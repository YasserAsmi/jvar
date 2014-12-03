cmake_minimum_required(VERSION 2.6)

project(jvar)

include_directories("include/")
link_directories("lib/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(LIBRARY_OUTPUT_PATH  "lib/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wno-unused-parameter -Wno-variadic-macros -std=c++0x")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Wextra -Wno-unused-parameter -Wno-variadic-macros -std=c++0x")

add_library(jvar STATIC src/str.cpp src/util.cpp src/arr.cpp src/var.cpp src/json.cpp)

add_executable(ex_basics example/basics.cpp)
target_link_libraries(ex_basics jvar -lrt)

add_executable(ex_arrays example/arrays.cpp)
target_link_libraries(ex_arrays jvar -lrt)

add_executable(ex_objs example/objs.cpp)
target_link_libraries(ex_objs jvar -lrt)

add_executable(ex_printv example/printv.cpp)
target_link_libraries(ex_printv jvar -lrt)

add_executable(ex_jsonparse example/jsonparse.cpp)
target_link_libraries(ex_jsonparse jvar -lrt)

add_executable(ex_func example/func.cpp)
target_link_libraries(ex_func jvar -lrt)

add_executable(ex_misc example/misc.cpp)
target_link_libraries(ex_misc jvar -lrt)
