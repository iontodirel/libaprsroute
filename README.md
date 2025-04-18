# libaprsroute

C++ header only standalone APRS routing library.

This library can be used to implement a digipeater.

Automatic Packet Reporting System (APRS) is a system for real-time digital data communication over ham radio.

See more info at http://www.aprs.org/ and https://groups.io/g/APRS.

![CMake Build Status](https://github.com/iontodirel/libaprsroute/actions/workflows/cmake.yml/badge.svg)
![CMake Clang Build Status](https://github.com/iontodirel/libaprsroute/actions/workflows/cmake-clang.yml/badge.svg)
![CodeQL Analysis Status](https://github.com/iontodirel/libaprsroute/actions/workflows/codeql-analysis.yml/badge.svg)

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
router_settings digi { "DIGI", {}, { "WIDE1" } };
routing_result result;

packet p = "N0CALL>APRS,WIDE1-3:data";

try_route_packet(p, digi, result);

assert(result.state == routing_state::routed);
assert(to_string(result.routed_packet) == "N0CALL>APRS,DIGI*,WIDE1-2:data");
```

### Routing diagnostics:

``` cpp
router_settings digi { "DIGI", {}, { "WIDE1", "WIDE2" }, routing_option::none, true };
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
router_settings digi { "DIGI", {}, { "WIDE1", "WIDE2" }, routing_option::none, /*enable_diagnostics*/ true };
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

for (auto& e : diag.entries)
{
    fmt::print(fg(fmt::color::blue_violet), "note: ");
    fmt::print("{}\n", e.message);
    fmt::print(fg(fmt::color::gray), "{:4}{}\n", "", e.packet_string);
    fmt::print(fg(fmt::color::lime_green), "{:4}{}\n", "", e.highlight_string);
}
```

### Address parsing:

#### Parsing a WIDE generic address

``` cpp
address s;
std::string path = "WIDE2-1*";

try_parse_address(path, s);

assert(s.mark == true); // *
assert(s.n == 2); // 2-1
assert(s.N == 1); // 2-1
assert(s.text == "WIDE");
assert(s.kind == address_kind::wide);
```

#### Parsing a callsign

``` cpp
address s;
std::string path = "N0CALL-10*";

try_parse_address(path, s);

assert(s.mark == true); // *
assert(s.ssid == 10); // 10
assert(s.text == "N0CALL");
```

Other examples can be found in the `tests` and `examples` directories.

## Features

- Packet decoding
- Address parsing, encoding and decomposition
  - Can parse WIDE generic addresses, and decode n, N, or the used flag
  - Can parse callsigns, and decode ssid, or the used flag
- Explicit routing
  - Supports all preemptive forms of routing: `front`, `truncate`, `drop` and `mark`
    - Preemptive `front` routing reorders the packet path
  - Supports any arbitrary aliases, ex: WIDE2-2 can be used for explicit routing, as an address "WIDE2" with callsign "2"
  - Supports optional router address substitution
- n-N routing
  - Supports any arbitrary generic addresses, ex: FOOBAR2-2 can be used
  - Supports optional used address substitution
  - Trap excessive hops
  - Configure hop limits per generic address, ex: WIDE2-2 - _2 maximum hops_, WIDE2 - _unlimited (7) number of hops_
- Diagnostics: every routing action is surfaced as a diagnostic
  - Routed packets can be fully restored using the routing actions alone
  - Intelligent post-routing decisions can be made using the routing actions
  - Routing actions can be printed as a ready string using `to_string`, or customized with `format`
- With or without packet type support, with no coupling on a packet data type
- No external dependencies, only uses a small set of the C++ standard library (C++20)

## Goals

- **Modularity**. This library is reusable, composable and standalone. It is written *without any coupling* to command line parsing code, configuration parsing code, memory management infrastructure, logging, or thread management. This makes it easy to integrate in any project in an unintrusive way. This library has no dependencies, only relying on the C++ standard library.
- **Diagnostics**. This library generates extensive diagnostics as part of its core routing API, which allow for clean detailed diagnostics and logging, without an actual dependency on stdout or a dependency on a logging library.
- **Testability**. This library has a comprehensive set of tests to ensure quality and correctness. The library has been written with testability in mind.
- **Documentation**. Democratize APRS. The documentation in the code, combined with a comprehensive set of tests, can be used for learning and understanding of how the APRS network routing works.
- **Cross Platform**. The library works on Linux, Windows, OSX and embedded platforms. And has been tested with an MSVC, Clang and GCC compiler.

## Testing

The ***tests*** directory contain a comprehensive sets of tests. `tests/tests.cpp` contains tests written using the Google Test framework.

A test asset file `routes.json` contains various routing scenarios. This test asset is stored in a structured format, which can be read by humans or programs, and can be used to study APRS routing, or used for standalone digipeater testing.

Use the tests as examples of how to use the library.

## Development

The test project can be opened in Visual Studio or VSCode. And it will work out of the box if the dependencies are installed.

Install the CMake and C++ extensions in VSCode, or the Native Desktop workload inside Visual Studio (*if using Visual Studio*).

On Linux systems, install the dependencies listed in `install_dependencies.sh`, which include a compiler, the CMake build system, and a native build system like make. Example for Debian systems: `apt-get install -y gcc g++ clang make cmake ninja-build`.

### AX25 and modularity

The library routes APRS packets directly, to maintain maximum flexibility and modularity. To do this, a routing result maintains both the original APRS packet, as well as the routed (transformed) APRS packet. An APRS packet can trivially be transformed to an AX25 or FX25 frame, after the routing has been completed.

The library does not interface directly with AX25 or HDLC frames. AX25 is a way to represent an APRS packet, but it is not the only way. APRS packets can also be stored, sent or received as FX25 frames, they could be sent in direct textual form via TCP sockets, or encapsulated in other protocols.

The library's focus on APRS packes for the routing, allows a developer to implement various transport mechanisms as needed, with no coupling on a particular frame type or transport. This abstracted layering, allows the library to work seamlessly in any environment, from embedded systems interfacing with hardware TNCs, to software TNCs that provide forward error correction, to local connected applications communicating over computer protocols like TCP, to internet connected applications using APRS-IS. We are not living in the 80s anymore, there is no need to directly modify H flags, using naive incomplete algorithms, with minimum computational resources.

By maintaining protocol independence, the library can be used in conjunction with existing AX.25 implementations, modern FX.25 systems with forward error correction, or even entirely new transport mechanisms that may emerge in the amateur radio community.

### Performance

The primary design goals of this project is modularity, usability and maintainability. However, the library tries its best to leverage zero overhead abstractions, like `string_view` as much as possible, to minimize the number of allocations and maximize throughput. The library clearly separates allocations and data structures, in a way
that makes it easy to minimize them. As I work on this project, I will continue to improve the library performance on constrained embedded devices.

All vector allocations can be configured with an optionally supplied allocator. All string allocations are effectively eliminated thanks to the small string optimization. This effectively eliminates all heap usage, when the router is configured with a stack allocator.

#### Performance analysis

This analysis contains routing performance for a typical but worse case scenario.

This is the packet used for testing: N0CALL-10>CALL-5,CALLA-10*,CALLB-5*,CALLC-15*,WIDE1*,WIDE2-1:data

The router's address is "DIGI". And the router's path is "WIDE1-2" and "WIDE2-3".

Diagnostics was disabled, as it is not a critical feature. The analysis measures the try_route_packet function performance. Routing memory shows total memory used, alongside total allocations (_do not multiplicate the two_). Routing time is calculated as an average over 10M routed packets, on desktop platforms; and 1K packets on embedded platforms.

Typically by default, the retail/release optimizations enable no heap memory usage even without a stack allocator. But using a stack allocator, guarantees that **no heap memory** is used by the router. For stack tests, the router is configured with a 4K reusable buffer allocated as a static C array on the stack. All string allocations are optimized away with the built-in small string optimization within std::string. All std::vector allocations are on the stack.

The tests were configured with `APRSROUTE_USE_STACK_ALLOCATOR2` in the stress harness, this configuration uses a non-PMR stack allocator.

| Platform     | Hardware                     | Throughput  | Routing time  | Routing memory (heap)      |
|--------------|------------------------------|-------------|---------------|----------------------------|
| Windows MSVC | Intel i9-14900HX, 97GB RAM   | 1.4M pkts/s | 0.68 μs       | 0 bytes, 0 allocations     |
| WSL GCC      | Intel i9-14900HX, 97GB RAM   | 1.9M pkts/s | 0.53 μs       | 0 bytes, 0 allocations     |
| WSL Clang    | Intel i9-14900HX, 97GB RAM   | 1.7M pkts/s | 0.58 μs       | 0 bytes, 0 allocations     |
| Linux GCC    | Intel Celeron N5095, 8GB RAM | 693K pkts/s | 1.44 μs       | 0 bytes, 0 allocations     |
| Linux Clang  | Intel Celeron N5095, 8GB RAM | 677K pkts/s | 1.48 μs       | 0 bytes, 0 allocations     |
| RISCV GCC    | ESP32 C6, 512KB RAM          | 6K pkts/s   | 165 μs        | 0 bytes, 0 allocations     |
| ARM GCC      | Pico 2 W, 520KB RAM          | 4.1K pkts/s | 242 μs        | 0 bytes, 0 allocations     |
| ARM GCC      | Teensy 4.1, 1024KB RAM       | 44K pkts/s  | 22 μs         | 0 bytes, 0 allocations     |

### Integration with CMake

As this is a header only library, you can simple download the header and use it:

`file(DOWNLOAD
    https://raw.githubusercontent.com/iontodirel/libaprsroute/main/aprsroute.hpp
    ${CMAKE_SOURCE_DIR}/external/aprsroute.hpp)`

Include the header in your source file:

`#include "external/aprsroute.hpp"`

### Integration with other build systems

As this is a header only library, you can simple download the header and use it.

## Examples

There are multiple examples provided in the `examples` directory. The examples showcase how to use the library in C, C++, C#, Java, JavaScript, Python, on Windows, Linux, OSX, Raspberry Pico and ESP32 platforms.

A barebone digipeater implementation is also provided.

- The `android_basic` example shows how to integrate the library in an Android application. The sample was generated using Android Studio.
- The `basic` example shows how to host the library in a standalone C++ CMake project.
- The `python_basic` example showcases a demo of the library from Python.
- The `dot_net_basic` example shows how to use the library from a .NET C# project. It contains a solution and two projects generated from Visual Studio.
- The `digipeater` example, contains a barebone implementation of a digipeater. This can be used to implement a digipeater. Note, the CMake projects contains references to boost and nlohmann/json, but aren't actually used, they are for implementation convenience and can be removed. I plan to implement a digipeater as a standalone project, using this `digipeater` sample as a starting point.
- The `pico_basic` example demonstrates simple usage of the library in an embedded Raspberry Pi Pico 2 project. The project was generated with the official Raspberry Pi Pico extension in VSCode with the default C++17 as the language standard.
- The `esp32_basic` example demonstrates simple usage of the library in an embedded ESP32 C6 project. The project was generated with the official ESP-IDF extension in VSCode with C++20 as the language standard.
- The `node_basic` example showcases a demo of the library from Node.js.