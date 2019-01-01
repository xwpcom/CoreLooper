cmake_minimum_required(VERSION 2.6)

project(corelooper CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CXX_FLAGS
 #-g
 # -DVALGRIND
 # -DMUDUO_STD_STRING
 #-DCHECK_PTHREAD_RETURN_VALUE
 #-D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 #-Werror
 -Wconversion
 #-Wno-unused-parameter
 #-Wold-style-cast
 #https://stackoverflow.com/questions/1628768/why-does-an-overridden-function-in-the-derived-class-hide-other-overloads-of-the
 -Wno-overloaded-virtual
 -Wno-missing-field-initializers
 -Wpointer-arith
 #-Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
 # -std=c++11
 -rdynamic
 )
if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -D_DEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Os -s -O2 -D NDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(CORELOOPER_DIR ${PROJECT_SOURCE_DIR}/src)
SET(INCLUDE_ROOT ${PROJECT_SOURCE_DIR}/include)
SET(INCLUDE_CORE ${INCLUDE_ROOT}/core)
INCLUDE_DIRECTORIES(${INCLUDE_ROOT})
INCLUDE_DIRECTORIES(${INCLUDE_CORE})
INCLUDE_DIRECTORIES(${INCLUDE_CORE}/base)
INCLUDE_DIRECTORIES(${INCLUDE_CORE}/file)
INCLUDE_DIRECTORIES(${INCLUDE_CORE}/looper)
INCLUDE_DIRECTORIES(${INCLUDE_CORE}/net)
INCLUDE_DIRECTORIES(${INCLUDE_CORE}/string)
INCLUDE_DIRECTORIES(${INCLUDE_CORE}/thread)
INCLUDE_DIRECTORIES(${INCLUDE_CORE}/xml)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#find_package(Boost REQUIRED)
#find_package(Protobuf)
#find_package(CURL)
#find_package(ZLIB)

include_directories(${PROJECT_SOURCE_DIR})
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(src)

if(CMAKE_BUILD_EXAMPLES)
  add_subdirectory(demo)
endif()
