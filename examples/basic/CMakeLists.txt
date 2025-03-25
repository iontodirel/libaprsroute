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

project ("example")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

add_executable(example "main.cpp")
target_link_libraries(example)
set_property(TARGET example PROPERTY CXX_STANDARD 20)

file(DOWNLOAD https://raw.githubusercontent.com/iontodirel/libaprsroute/main/aprsroute.hpp ${CMAKE_SOURCE_DIR}/external/aprsroute.hpp)
