# libaprsroute

C++ header only APRS routing library

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
assert(to_string(result.routed_packet) == "N0CALL>APRS,DIGI*,WIDE1-2:data");
```

Other examples can be found in the tests directory.

## Goals

- **Modularity**. This library is reusable, composable and standalone. It is written without any coupling to command line parsing code, configuration parsing code, memory management infrastructure, logging, or thread management, as it's typical will all current digipeaters. This makes it easy to integrate in any project in an unintrusive way.
- **Diagnostics**. This library generates extensive diagnostics as part of its routing API, which allow for clean detailed diagnostics and logging.
- **Testability**. This library has a comprehensive set of tests to guarantee correctness. The library has been written in a way that makes it's APIs easily testable.

## Testing

The ***tests*** directory contain a comprehensive sets of tests for the routing APIs. A test asset ***routes.json*** contain various routing scenarios, and is used to drive the testing.