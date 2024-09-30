# **************************************************************** #
# libaprsroute - APRS header only routing library                  # 
# Version 0.1.0                                                    #
# https://github.com/iontodirel/libaprsroute                       #
# Copyright (c) 2024 Ion Todirel                                   #
# **************************************************************** #
#
# CMakeLists.txt
#
# MIT License
#
# Copyright (c) 2024 Ion Todirel

cmake_minimum_required (VERSION 3.8)

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

FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG master)
FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
  FetchContent_Populate(fmt)
  add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

enable_testing()

set(INPUT_TEST_FILE ${CMAKE_SOURCE_DIR}/routes.json)

add_executable(aprsroute_tests "tests.cpp" "../aprsroute.hpp")
target_link_libraries(aprsroute_tests GTest::gtest_main gtest gtest_main nlohmann_json::nlohmann_json fmt::fmt)
add_definitions(-DINPUT_TEST_FILE="${INPUT_TEST_FILE}")

set_property(TARGET aprsroute_tests PROPERTY CXX_STANDARD 20)

if(MSVC)
    target_compile_options(aprsroute_tests PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(aprsroute_tests PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

include(GoogleTest)

gtest_discover_tests(aprsroute_tests)
