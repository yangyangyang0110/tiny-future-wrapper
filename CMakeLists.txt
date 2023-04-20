cmake_minimum_required(VERSION 3.23)
project(tiny-future)

set(CMAKE_CXX_STANDARD 14)

option(ENABLE_TEST "enable test" ON)


set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_ROOT ${PROJECT_ROOT}/src)

### Find Package.

find_package(Boost)


set(benchmark_ROOT /home/ubuntu/3rdparty/google_benchmark)
find_package(benchmark REQUIRED)
#add_subdirectory(benchmark)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_ROOT}/cmake)
if (ENABLE_TEST)
	enable_testing()
	include(gtest)
endif ()


add_subdirectory(src)
