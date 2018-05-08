cmake_minimum_required(VERSION 2.6) 

project(Vana)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)

use_cxx11()

set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

#### Finding libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.37.0 REQUIRED COMPONENTS filesystem regex system thread)

find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})

find_package(MySQL++ REQUIRED)
include_directories(${MYSQL++_INCLUDE_DIR})

find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

find_package(Crypto++ REQUIRED)
include_directories(${CRYPTO++_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src/Common)

add_subdirectory(src)
