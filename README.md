# libaprsroute

C++ header only standalone APRS routing library

## Examples

Decoding a packet:

``` cpp
packet p;
try_decode_packet("N0CALL>APRS,WIDE2-2:data", p);

assert(p.from == "N0CALL");
assert(p.to == "APRS");
assert(p.data == "data");
assert(p.path.size() == 1);
assert(p.path[0] == "WIDE2-2");
```

Routing a packet:

``` cpp
router_settings digi { "DIGI", { "WIDE1" } };
routing_result result;

packet p = { "N0CALL", "APRS", { "WIDE1-3" }, "data"}; // N0CALL>APRS,WIDE1-3:data

try_route_packet(p, digi, result);

assert(result.state == routing_state::routed);
assert(to_string(result.routed_packet) == "N0CALL>APRS,DIGI*,WIDE1-2:data"); // N0CALL>APRS,DIGI*,WIDE1-2:data
```

Other examples can be found in the tests directory.

## Goals

- **Modularity**. This library is reusable, composable and standalone. It is written *without any coupling* to command line parsing code, configuration parsing code, memory management infrastructure, logging, or thread management, as it's typical will current digipeaters. This makes it easy to integrate in any project in an unintrusive way. This library has no dependencies, only relying on the C++ standard library.
- **Diagnostics**. This library generates extensive diagnostics as part of its routing API, which allow for clean detailed diagnostics and logging, without actual dependency on stdout oa logging library.
- **Testability**. This library has a comprehensive set of tests to ensure quality and correctness. The library has been written with testability in mind.
- **Documentation**. The documentation in the code, combined with the comprehensive tests and test assets, can be used for learning and understanding how the APRS digipeaters work.
- **Cross Platform**. This library works on Linux, Windows or OSX systems. And has been tested with an MSVC and GCC compiler.

## Testing

The ***tests*** directory contain a comprehensive sets of tests for the library. A test asset file (***routes.json***) contains various routing scenarios, and is used to drive the testing. This test asset is in a structured format, can be read by humans and programs, and can be used to study APRS routing, or used for standalone digipeater testing.

## Development

The test project can be opened in Visual Studio or VSCode. Install the dependencies listed in install_dependencies.sh, which include a compiler, the CMake build system, and a native build system like make. Install the CMake and C++ extensions in VSCode, or the Native Desktop workload inside Visual Studio (*if using Visual Studio*).