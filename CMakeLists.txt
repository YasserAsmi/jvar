cmake_minimum_required(VERSION 2.6)

project(jvar)

include_directories("include/")
link_directories("lib/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
set(LIBRARY_OUTPUT_PATH  "lib/")

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wno-unused-parameter -Wno-variadic-macros -Wno-ignored-qualifiers")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Wextra -Wno-unused-parameter -Wno-variadic-macros -Wno-ignored-qualifiers")
endif(CMAKE_COMPILER_IS_GNUCXX)

option(AUTOADDPROP "Enable adding properties on assignment, without using addProperty" OFF)
message(STATUS "AUTOADDPROP = " ${AUTOADDPROP})
if(AUTOADDPROP)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAUTOADDPROP")
endif(AUTOADDPROP)

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

install (TARGETS jvar 
	     ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (DIRECTORY include/ DESTINATION include/jvar)
