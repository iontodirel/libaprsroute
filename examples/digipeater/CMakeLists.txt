cmake_minimum_required (VERSION 3.25)

project ("digipeater")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

set(BOOST_INCLUDE_LIBRARIES asio thread beast circular_buffer interprocess process)
set(BOOST_ENABLE_CMAKE ON)
set(BOOST_USE_STATIC_LIBS ON)

FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz
  URL_MD5 d55d43218e81ca3d0fc14436b7665bf1
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(boost)

FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG master)
FetchContent_MakeAvailable(fmt)

add_executable(digipeater "main.cpp" "digipeater.cpp" "common.cpp" "log.cpp")
target_link_libraries(digipeater nlohmann_json::nlohmann_json Boost::asio Boost::thread Boost::beast Boost::process Boost::circular_buffer Boost::interprocess fmt::fmt)

set_property(TARGET digipeater PROPERTY CXX_STANDARD 23)

file(DOWNLOAD https://raw.githubusercontent.com/iontodirel/libaprsroute/main/aprsroute.hpp ${CMAKE_SOURCE_DIR}/external/aprsroute.hpp)
