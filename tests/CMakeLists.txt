# **************************************************************** #
# libaprsroute - APRS header only routing library                  # 
# Version 0.1.0                                                    #
# https://github.com/iontodirel/libaprsroute                       #
# Copyright (c) 2024 - 2025 Ion Todirel                            #
# **************************************************************** #
#
# CMakeLists.txt
# 
# MIT License
# 
# Copyright (c) 2024 - 2025 Ion Todirel
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required (VERSION 3.25)

project ("aprsroute_tests")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

if(MSVC)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

if(UNIX)
    add_compile_definitions(IS_LINUX_MAC)
endif()

include(FetchContent)

FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG master)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(etl GIT_REPOSITORY https://github.com/ETLCPP/etl GIT_TAG 20.43.4)
FetchContent_MakeAvailable(etl)

enable_testing()

set(INPUT_TEST_FILE ${CMAKE_SOURCE_DIR}/routes.json)

add_executable(aprsroute_tests "tests.cpp" "../aprsroute.hpp")
target_link_libraries(aprsroute_tests GTest::gtest_main gtest gtest_main nlohmann_json::nlohmann_json fmt::fmt)
set_property(TARGET aprsroute_tests PROPERTY CXX_STANDARD 20)

add_executable(aprsroute_stress_test "stress_test.cpp" "../aprsroute.hpp")
target_link_libraries(aprsroute_stress_test)
set_property(TARGET aprsroute_stress_test PROPERTY CXX_STANDARD 20)

add_executable(aprsroute_external_packet_test "external_packet_test.cpp" "../aprsroute.hpp")
target_link_libraries(aprsroute_external_packet_test GTest::gtest_main gtest gtest_main)
set_property(TARGET aprsroute_external_packet_test PROPERTY CXX_STANDARD 20)

add_executable(aprsroute_external_custom_packet_test "external_custom_packet_test.cpp" "../aprsroute.hpp")
target_link_libraries(aprsroute_external_custom_packet_test GTest::gtest_main gtest gtest_main)
set_property(TARGET aprsroute_external_custom_packet_test PROPERTY CXX_STANDARD 20)

add_executable (aprsroute_use_in_tu "use_in_tu.h" "use_in_tu.cpp" "use_in_other_tu.cpp" "compile_in_tu.cpp")
set_property(TARGET aprsroute_use_in_tu PROPERTY CXX_STANDARD 20)

add_executable(aprsroute_without_pmr_build_test "without_pmr.cpp" "../aprsroute.hpp")
set_property(TARGET aprsroute_without_pmr_build_test PROPERTY CXX_STANDARD 20)

add_executable(aprsroute_with_pmr_build_test "with_pmr.cpp" "../aprsroute.hpp")
set_property(TARGET aprsroute_with_pmr_build_test PROPERTY CXX_STANDARD 20)

add_executable(aprsroute_set_cpp17_std "set_cpp17_std.cpp" "../aprsroute.hpp")
set_property(TARGET aprsroute_set_cpp17_std PROPERTY CXX_STANDARD 17)

add_executable(aprsroute_with_etl_vector_test "with_etl_vector.cpp" "../aprsroute.hpp")
set_property(TARGET aprsroute_with_etl_vector_test PROPERTY CXX_STANDARD 20)
target_link_libraries(aprsroute_with_etl_vector_test PRIVATE etl::etl)

add_executable(aprsroute_with_etl_string_test "with_etl_string.cpp" "../aprsroute.hpp")
set_property(TARGET aprsroute_with_etl_string_test PROPERTY CXX_STANDARD 20)
target_link_libraries(aprsroute_with_etl_string_test PRIVATE etl::etl)

add_definitions(-DINPUT_TEST_FILE="${INPUT_TEST_FILE}")

if(MSVC)
    target_compile_options(aprsroute_tests PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(aprsroute_tests PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

include(GoogleTest)

gtest_discover_tests(aprsroute_tests)
gtest_discover_tests(aprsroute_external_packet_test)
gtest_discover_tests(aprsroute_external_custom_packet_test)

