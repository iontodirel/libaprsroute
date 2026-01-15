#include "use_in_tu.h"

TEST(router, try_route_packet_in_tu)
{
using namespace aprs::router;
using namespace aprs::router::detail;

    router_settings digi { "DIGI", {}, { "WIDE1" } };
    routing_result result;

    packet p = { "N0CALL", "APRS", { "WIDE1-3" }, "data" }; // N0CALL>APRS,WIDE1-3:data

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,DIGI*,WIDE1-2:data");
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}