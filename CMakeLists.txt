cmake_minimum_required(VERSION 2.6)

project(jvar)

include_directories("${CMAKE_SOURCE_DIR}/include/")
link_directories("${CMAKE_SOURCE_DIR}/lib/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH  "${CMAKE_SOURCE_DIR}/lib/")

option(CXX11 "Enable C++11 support" OFF)
message(STATUS "CXX11 = " ${CXX11})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wno-unused-parameter -Wno-variadic-macros -Wno-ignored-qualifiers")

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
endif()

if(CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif(CXX11)
message(STATUS "CMAKE_CXX_FLAGS = " ${CMAKE_CXX_FLAGS})

option(AUTOADDPROP "Enable adding properties on assignment, without using addProperty" OFF)
message(STATUS "AUTOADDPROP = " ${AUTOADDPROP})
if(AUTOADDPROP)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAUTOADDPROP")
endif(AUTOADDPROP)

add_library(jvar STATIC src/str.cpp src/util.cpp src/arr.cpp src/var.cpp src/json.cpp)

add_executable(ex_basics example/basics.cpp)
target_link_libraries(ex_basics jvar )

add_executable(ex_arrays example/arrays.cpp)
target_link_libraries(ex_arrays jvar)

add_executable(ex_objs example/objs.cpp)
target_link_libraries(ex_objs jvar)

add_executable(ex_printv example/printv.cpp)
target_link_libraries(ex_printv jvar)

add_executable(ex_jsonparse example/jsonparse.cpp)
target_link_libraries(ex_jsonparse jvar)

add_executable(ex_func example/func.cpp)
target_link_libraries(ex_func jvar)

add_executable(ex_misc example/misc.cpp)
target_link_libraries(ex_misc jvar)

install (TARGETS jvar
	     ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)

install (DIRECTORY include/ DESTINATION include/jvar)
