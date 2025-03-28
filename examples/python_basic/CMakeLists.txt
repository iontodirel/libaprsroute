cmake_minimum_required (VERSION 3.25)

project ("libroute")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

add_library(route SHARED "main.cpp")
target_link_libraries(route)
set_property(TARGET route PROPERTY CXX_STANDARD 20)

file(DOWNLOAD https://raw.githubusercontent.com/iontodirel/libaprsroute/main/aprsroute.hpp ${CMAKE_SOURCE_DIR}/external/aprsroute.hpp)

set_target_properties(route PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
