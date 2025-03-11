#include "use_in_tu.h"

#include <cassert>

using namespace aprs::router;
using namespace aprs::router::detail;

int main()
{
    router_settings digi { "DIGI", { "WIDE1" } };
    routing_result result;

    packet p = { "N0CALL", "APRS", { "WIDE1-3" }, "data" }; // N0CALL>APRS,WIDE1-3:data

    try_route_packet(p, digi, result);

    assert(result.state == routing_state::routed);
    assert(to_string(result.routed_packet) == "N0CALL>APRS,DIGI*,WIDE1-2:data");

    func();

    return 0;
}