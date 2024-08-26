# **************************************************************** #
# libaprsroute - APRS header only routing library                  # 
# Version 0.1.0                                                    #
# https://github.com/iontodirel/libaprsroute                       #
# Copyright (c) 2024 Ion Todirel                                   #
# **************************************************************** #

cmake_minimum_required (VERSION 3.8)

project ("aprsroute_tests")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

if(MSVC)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

include(FetchContent)

FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(aprsroute_tests "tests.cpp" "../aprsroute.hpp")
target_link_libraries(aprsroute_tests GTest::gtest_main gtest gtest_main)

set_property(TARGET aprsroute_tests PROPERTY CXX_STANDARD 20)

include(GoogleTest)

gtest_discover_tests(aprsroute_tests)
