# libaprsroute

C++ header only standalone APRS routing library.

This library can be used to implement a digipeater.

Automatic Packet Reporting System (APRS) is a system for real-time digital data communication over ham radio.

See more info at http://www.aprs.org/ and https://groups.io/g/APRS.

## Examples

### Decoding a packet:

``` cpp
packet p;
try_decode_packet("N0CALL>APRS,WIDE2-2:data", p);

assert(p.from == "N0CALL");
assert(p.to == "APRS");
assert(p.data == "data");
assert(p.path.size() == 1);
assert(p.path[0] == "WIDE2-2");
```

### Decoding a packet using constructors:

``` cpp
packet p = "N0CALL>APRS,WIDE2-2:data";
assert(p == "N0CALL>APRS,WIDE2-2:data");
```

### Routing a packet:

``` cpp
router_settings digi { "DIGI", { "WIDE1" } };
routing_result result;

packet p = { "N0CALL", "APRS", { "WIDE1-3" }, "data"}; // N0CALL>APRS,WIDE1-3:data

try_route_packet(p, digi, result);

assert(result.state == routing_state::routed);
assert(to_string(result.routed_packet) == "N0CALL>APRS,DIGI*,WIDE1-2:data"); // N0CALL>APRS,DIGI*,WIDE1-2:data
```

### Routing diagnostics:

``` cpp
router_settings digi { "DIGI", { "WIDE1", "WIDE2" }, routing_option::none, true };
routing_result result;

packet p = { "N0CALL", "APRS", { "WIDE1-2" }, "data"};

try_route_packet(p, digi, result);

// N0CALL>APRS,WIDE1-2:data
//             ~~~~~~~
//             12 19 - Packet address decremented
//
// N0CALL>APRS,DIGI,WIDE1-1:data
//             ~~~~
//             12 16 - Packet address inserted
//
// N0CALL>APRS,DIGI,WIDE1-1:data
//             ~~~~
//             12 16 - Packet address marked as 'set'

assert(result.actions.size() == 3);

assert(result.actions[0].address == "WIDE1-1");
assert(result.actions[0].target == applies_to::path);
assert(result.actions[0].type == routing_action::decrement);
assert(result.actions[0].start == 12);
assert(result.actions[0].end == 19);
assert(result.actions[0].index == 0);

assert(result.actions[1].address == "DIGI");
assert(result.actions[1].target == applies_to::path);
assert(result.actions[1].type == routing_action::insert);
assert(result.actions[1].start == 12);
assert(result.actions[1].end == 16);
assert(result.actions[1].index == 0);

assert(result.actions[2].address == "DIGI");
assert(result.actions[2].target == applies_to::path);
assert(result.actions[2].type == routing_action::set);
assert(result.actions[2].start == 12);
assert(result.actions[2].end == 16);
assert(result.actions[2].index == 0);
```

### Print routing diagnostics using to_string:

``` cpp
router_settings digi { "DIGI", { "WIDE1", "WIDE2" }, routing_option::none, /*enable_diagnostics*/ true };
routing_result result;

packet p = { "N0CALL", "APRS", { "WIDE1-2" }, "data"};

try_route_packet(p, digi, result);

std::string diag_string = to_string(result);

std::cout << diag_string << std::endl;

// Prints the following to stdout:

/*
   Packet address decremented:
   
   N0CALL>APRS,WIDE1-1:data
               ~~~~~~~
   
   Packet address inserted:
   
   N0CALL>APRS,DIGI,WIDE1-1:data
               ~~~~
   
   Packet address marked as 'set':
   
   N0CALL>APRS,DIGI*,WIDE1-1:data
               ~~~~~
*/
```

### Print routing diagnostics using format:

``` cpp
routing_result result;

routing_diagnostic_display diag = format(result);

for (auto& l : diag.entries)
{
    fmt::print(fg(fmt::color::blue_violet), "note: ");
    fmt::print("{}\n", l.message);
    fmt::print(fg(fmt::color::gray), "{:4}{}\n", "", l.packet_string);
    fmt::print(fg(fmt::color::lime_green), "{:4}{}\n", "", l.highlight_string);
}
```

Other examples can be found in the tests directory.

## Features

- Packet decoding
- Address parsing, encoding and decomposition
- Explicit routing
  - Support for preemptive routing: front, truncate and drop
- n-N routing
  - Supports any arbitrary alias, ex: FOOBAR2-2 can be used
  - Supports completed alias substitution
  - Trap excessive hops
- Diagnostics: every routing action is surfaced as a diagnostic
  - Routed packets can be fully restored using the routing actions alone
  - Intelligent post-routing decisions can be made using the routing actions
  - Routing actions can be printed as a ready string using `to_string` or customized using `format`

## Goals

- **Modularity**. This library is reusable, composable and standalone. It is written *without any coupling* to command line parsing code, configuration parsing code, memory management infrastructure, logging, or thread management. This makes it easy to integrate in any project in an unintrusive way. This library has no dependencies, only relying on the C++ standard library.
- **Diagnostics**. This library generates extensive diagnostics as part of its core routing API, which allow for clean detailed diagnostics and logging, without an actual dependency on stdout or a dependency on a logging library.
- **Testability**. This library has a comprehensive set of tests to ensure quality and correctness. The library has been written with testability in mind.
- **Documentation**. The documentation in the code, combined with a comprehensive set of tests, can be used for learning and understanding of how the APRS network routing works.
- **Cross Platform**. The library works on Linux, Windows or OSX systems. And has been tested with an MSVC, Clang and GCC compiler.

## Testing

The ***tests*** directory contain a comprehensive sets of tests. `tests/tests.cpp` contains a comprehensive sets of tests, written using the Google Test framework.

A test asset file `routes.json` contains various routing scenarios. This test asset is stored in a structured format, which can be read by humans or programs, and can be used to study APRS routing, or used for standalone digipeater testing.

Use the tests as examples of how to use the library.

## Development

The test project can be opened in Visual Studio or VSCode. And it will work out of the box if the dependencies are installed.

Install the CMake and C++ extensions in VSCode, or the Native Desktop workload inside Visual Studio (*if using Visual Studio*).

On Linux systems, install the dependencies listed in `install_dependencies.sh`, which include a compiler, the CMake build system, and a native build system like make. Example for Debian systems: `apt-get install -y gcc g++ clang make cmake ninja-build`.

## Integration with CMake

As this is a header only library, you can simple download the header and use it:

`file(DOWNLOAD
    https://raw.githubusercontent.com/iontodirel/libaprsroute/main/aprsroute.hpp
    ${CMAKE_SOURCE_DIR}/external/aprsroute.hpp)`

Include the header in your source file:

`#include "external/aprsroute.hpp"`
