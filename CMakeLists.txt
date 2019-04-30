cmake_minimum_required (VERSION 3.12.1)
project (SeamCarver)
enable_testing()
include(CTest)
set(CMAKE_CXX_FLAGS -std="c++17")

find_package(Boost 1.55 REQUIRED COMPONENTS unit_test_framework)
include_directories (tests ${Boost_INCLUDE_DIRS})
add_definitions (-DBOOST_TEST_DYN_LINK)

add_executable(seamcarve seam_carver.cpp)