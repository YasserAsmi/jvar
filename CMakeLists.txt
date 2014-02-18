cmake_minimum_required(VERSION 2.6)

project(edge)

include_directories("include/")
link_directories("lib/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(LIBRARY_OUTPUT_PATH  "lib/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wno-unused-parameter -Wno-variadic-macros")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Og -Wall -Wextra -Wno-unused-parameter -Wno-variadic-macros")

add_library(jvar STATIC base/str.cpp base/util.cpp base/arr.cpp base/var.cpp base/json.cpp)
add_library(edgedb STATIC db/sql.cpp db/dm.cpp)

add_executable(basetest test/basetest.cpp)
target_link_libraries(basetest jvar -lrt)

add_executable(parsetest test/parsetest.cpp)
target_link_libraries(parsetest jvar  -lrt)

add_executable(sqltest test/sqltest.cpp)
target_link_libraries(sqltest edgedb jvar -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -ldl)

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
