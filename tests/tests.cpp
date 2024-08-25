#include <gtest/gtest.h>

#include "../aprsroute.hpp"

using namespace aprs::router;
using namespace aprs::router::detail;

TEST(segment, try_parse_segment)
{
    segment s1;
    s1.text = "WIDE";
    s1.n = 2;
    s1.N = 1;
    s1.mark = false;
    EXPECT_TRUE(to_string(s1) == "WIDE2-1");

    s1.mark = true;
    EXPECT_TRUE(to_string(s1) == "WIDE2-1*");

    // Test case: Wide segment with both digits present
    std::string path1 = "WIDE7-5";
    segment s2;
    EXPECT_TRUE(try_parse_segment(path1, s2));
    EXPECT_TRUE(s2.mark == false);
    EXPECT_TRUE(s2.n == 7);
    EXPECT_TRUE(s2.N == 5);
    EXPECT_TRUE(s2.text == "WIDE");
    EXPECT_TRUE(s2.type == segment_type::wide);

    // Test case: Wide segment with a mark but no digits after dash
    std::string path2 = "WIDE*";
    segment s3;
    EXPECT_TRUE(try_parse_segment(path2, s3));
    EXPECT_TRUE(s3.mark == true);
    EXPECT_TRUE(s3.n == 0);
    EXPECT_TRUE(s3.N == 0);
    EXPECT_TRUE(s3.text == "WIDE");
    EXPECT_TRUE(s3.type == segment_type::wide);

    // Test case: Wide segment with only leading digits
    std::string path3 = "WIDE5";
    segment s4;
    EXPECT_TRUE(try_parse_segment(path3, s4));
    EXPECT_TRUE(s4.mark == false);
    EXPECT_TRUE(s4.n == 5);
    EXPECT_TRUE(s4.N == 0);
    EXPECT_TRUE(s4.text == "WIDE");
    EXPECT_TRUE(s4.type == segment_type::wide);

    // Test case: Wide segment with leading digits and a mark
    std::string path4 = "WIDE5*";
    segment s5;
    EXPECT_TRUE(try_parse_segment(path4, s5));
    EXPECT_TRUE(s5.mark == true);
    EXPECT_TRUE(s5.n == 5);
    EXPECT_TRUE(s5.N == 0);
    EXPECT_TRUE(s5.text == "WIDE");
    EXPECT_TRUE(s5.type == segment_type::wide);

    // Test case: Q construct segment
    std::string path5 = "qAR";
    segment s6;
    EXPECT_TRUE(try_parse_segment(path5, s6));
    EXPECT_TRUE(s6.mark == false);
    EXPECT_TRUE(s6.n == 0);
    EXPECT_TRUE(s6.N == 0);
    EXPECT_TRUE(s6.text == "qAR");
    EXPECT_TRUE(s6.q == q_construct::qAR);
    EXPECT_TRUE(s6.type == segment_type::q);

    // Test case: Wide segment with both digits and a mark
    std::string path6 = "WIDE7-7*";
    segment s7;
    EXPECT_TRUE(try_parse_segment(path6, s7));
    EXPECT_TRUE(s7.mark == true);
    EXPECT_TRUE(s7.n == 7);
    EXPECT_TRUE(s7.N == 7);
    EXPECT_TRUE(s7.text == "WIDE");
    EXPECT_TRUE(s7.type == segment_type::wide);

    // Test case: Other segment with text and digits
    std::string path7 = "W7ION-10*";
    segment s8;
    EXPECT_TRUE(try_parse_segment(path7, s8));
    EXPECT_TRUE(s8.mark == true);
    EXPECT_TRUE(s8.n == 0);
    EXPECT_TRUE(s8.N == 0);
    EXPECT_TRUE(s8.text == "W7ION-10");
    EXPECT_TRUE(s8.type == segment_type::other);

    // Test case: Other segment without a mark and with no digits
    std::string path8 = "N0CALL";
    segment s9;
    EXPECT_TRUE(try_parse_segment(path8, s9));
    EXPECT_TRUE(s9.mark == false);
    EXPECT_TRUE(s9.n == 0);
    EXPECT_TRUE(s9.N == 0);
    EXPECT_TRUE(s9.text == "N0CALL");
    EXPECT_TRUE(s9.type == segment_type::other);

    // Additional test cases to cover edge cases
    std::string path9 = "WIDE-1"; // Should not be valid, missing leading digits
    segment s10;
    EXPECT_TRUE(try_parse_segment(path9, s10));
    EXPECT_TRUE(s10.mark == false);
    EXPECT_TRUE(s10.n == 0);
    EXPECT_TRUE(s10.N == 0);
    EXPECT_TRUE(s10.text == "WIDE-1"); // Entire text should be preserved if parsing fails
    EXPECT_TRUE(s10.type == segment_type::other);

    std::string path10 = "*WIDE"; // Leading mark with no valid segment
    segment s11;
    EXPECT_TRUE(try_parse_segment(path10, s11));
    EXPECT_TRUE(s11.mark == false);
    EXPECT_TRUE(s11.n == 0);
    EXPECT_TRUE(s11.N == 0);
    EXPECT_TRUE(s11.text == "*WIDE");
    EXPECT_TRUE(s11.type == segment_type::other);

    std::string path11 = "WIDE0-4";
    segment s12;
    EXPECT_TRUE(try_parse_segment(path11, s12));
    EXPECT_TRUE(s12.n == 0);
    EXPECT_TRUE(s12.N == 0);
    EXPECT_TRUE(s12.text == "WIDE0-4");
    EXPECT_TRUE(s12.type == segment_type::other);

    std::string path12 = "WIDE8-4";
    segment s13;
    EXPECT_TRUE(try_parse_segment(path12, s13));
    EXPECT_TRUE(s13.n == 0);
    EXPECT_TRUE(s13.N == 0);
    EXPECT_TRUE(s13.text == "WIDE8-4");
    EXPECT_TRUE(s13.type == segment_type::other);
}

TEST(packet, try_decode_packet)
{
    // Test case: Basic packet with single path
    std::string packet_string1 = "N0CALL>APRS,WIDE2-2:data";
    packet p1;
    try_decode_packet(packet_string1, p1);
    EXPECT_TRUE(p1.from == "N0CALL");
    EXPECT_TRUE(p1.to == "APRS");
    EXPECT_TRUE(p1.data == "data");
    EXPECT_TRUE(p1.path.size() == 1);
    EXPECT_TRUE(p1.path[0] == "WIDE2-2");
    EXPECT_TRUE(to_string(p1) == packet_string1);

    // Test case: Packet with multiple paths and a mark
    std::string packet_string2 = "N0CALL>APRS,OH7RDB,OH7RDC,WIDE2*:data";
    packet p2;
    try_decode_packet(packet_string2, p2);
    EXPECT_TRUE(p2.from == "N0CALL");
    EXPECT_TRUE(p2.to == "APRS");
    EXPECT_TRUE(p2.data == "data");
    EXPECT_TRUE(p2.path.size() == 3);
    EXPECT_TRUE(p2.path[0] == "OH7RDB");
    EXPECT_TRUE(p2.path[1] == "OH7RDC");
    EXPECT_TRUE(p2.path[2] == "WIDE2*");
    EXPECT_TRUE(to_string(p2) == packet_string2);

    // Test case: Packet with empty path
    std::string packet_string3 = "N0CALL>APRS::data";
    packet p3;
    try_decode_packet(packet_string3, p3);
    EXPECT_TRUE(p3.from == "N0CALL");
    EXPECT_TRUE(p3.to == "APRS");
    EXPECT_TRUE(p3.data == ":data");
    EXPECT_TRUE(p3.path.empty());
    EXPECT_TRUE(to_string(p3) == packet_string3);

    // Test case: Packet with missing data field
    // std::string packet_string4 = "N0CALL>APRS,WIDE2-2";
    // packet p4;
    // try_decode_packet(packet_string4, p4);
    // EXPECT_TRUE(p4.from == "N0CALL");
    // EXPECT_TRUE(p4.to == "APRS");
    // EXPECT_TRUE(p4.data.empty());  // Assuming empty data is acceptable
    // EXPECT_TRUE(p4.path.size() == 1);
    // EXPECT_TRUE(p4.path[0] == "WIDE2-2");
    // EXPECT_TRUE(to_string(p4) == packet_string4);

    // Test case: Packet with no path
    std::string packet_string5 = "N0CALL>APRS:data";
    packet p5;
    try_decode_packet(packet_string5, p5);
    EXPECT_TRUE(p5.from == "N0CALL");
    EXPECT_TRUE(p5.to == "APRS");
    EXPECT_TRUE(p5.data == "data");
    EXPECT_TRUE(p5.path.empty());
    EXPECT_TRUE(to_string(p5) == packet_string5);

    // Test case: Packet with invalid path format
    std::string packet_string6 = "N0CALL>APRS,INVALID_PATH_FORMAT:data";
    packet p6;
    try_decode_packet(packet_string6, p6);
    EXPECT_TRUE(p6.from == "N0CALL");
    EXPECT_TRUE(p6.to == "APRS");
    EXPECT_TRUE(p6.data == "data");
    EXPECT_TRUE(p6.path.size() == 1);
    EXPECT_TRUE(p6.path[0] == "INVALID_PATH_FORMAT");
    EXPECT_TRUE(to_string(p6) == packet_string6);

    // Test case: Packet with special characters in path
    std::string packet_string7 = "N0CALL>APRS,SPCL-@!,WIDE2*:data";
    packet p7;
    try_decode_packet(packet_string7, p7);
    EXPECT_TRUE(p7.from == "N0CALL");
    EXPECT_TRUE(p7.to == "APRS");
    EXPECT_TRUE(p7.data == "data");
    EXPECT_TRUE(p7.path.size() == 2);
    EXPECT_TRUE(p7.path[0] == "SPCL-@!");
    EXPECT_TRUE(p7.path[1] == "WIDE2*");
    EXPECT_TRUE(to_string(p7) == packet_string7);

    // Test case: Packet with only path and no data
    // std::string packet_string8 = "N0CALL>APRS,WIDE2-2,OH7RDB";
    // packet p8;
    // try_decode_packet(packet_string8, p8);
    // EXPECT_TRUE(p8.from == "N0CALL");
    // EXPECT_TRUE(p8.to == "APRS");
    // EXPECT_TRUE(p8.data.empty());  // Assuming empty data is acceptable
    // EXPECT_TRUE(p8.path.size() == 2);
    // EXPECT_TRUE(p8.path[0] == "WIDE2-2");
    // EXPECT_TRUE(p8.path[1] == "OH7RDB");
    // EXPECT_TRUE(to_string(p8) == packet_string8);

    // Test case: Packet with only source and destination
    std::string packet_string9 = "N0CALL>APRS:";
    packet p9;
    try_decode_packet(packet_string9, p9);
    EXPECT_TRUE(p9.from == "N0CALL");
    EXPECT_TRUE(p9.to == "APRS");
    EXPECT_TRUE(p9.data.empty());  // Assuming empty data is acceptable
    EXPECT_TRUE(p9.path.empty());
    EXPECT_TRUE(to_string(p9) == packet_string9);
}

TEST(router, try_route_packet)
{
    routing_result result;
    router_settings digi;

    // -------------------------------------------------------------
    // Explicitely route a packet through the router's callsign
    //
    // Input:  WB2OSZ>APRS,N2GH,W2UB:data
    // Output: WB2OSZ>APRS,N2GH*,W2UB:data
    // -------------------------------------------------------------

    digi.callsign = "N2GH";

    packet p = {"WB2OSZ", "APRS", {"N2GH", "W2UB"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(p) == to_string(result.original_packet));
    EXPECT_TRUE(to_string(result.routed_packet) == "WB2OSZ>APRS,N2GH*,W2UB:data");

    // -------------------------------------------------------------
    // Explicitely route a packet through the router's callsign
    //
    // Input:  WB2OSZ>APRS,N2GH*,W2UB:data
    // Output: WB2OSZ>APRS,N2GH,W2UB*:data
    // -------------------------------------------------------------

    digi.callsign = "W2UB";

    p = {"WB2OSZ", "APRS", {"N2GH*", "W2UB"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(p) == to_string(result.original_packet));
    EXPECT_TRUE(to_string(result.routed_packet) == "WB2OSZ>APRS,N2GH,W2UB*:data");

    // -------------------------------------------------------------
    // Explicitely route a packet that has to wait on the first digipeter to route first
    //
    // Input:  WB2OSZ>APRS,N2GH,W2UB:data
    // Output: WB2OSZ>APRS,N2GH,W2UB:data
    // -------------------------------------------------------------

    digi.callsign = "W2UB";

    p = {"WB2OSZ", "APRS", { "N2GH", "W2UB" }, "data" };
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == false);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::not_routed);
    EXPECT_TRUE(to_string(p) == to_string(result.original_packet));
    EXPECT_TRUE(to_string(result.routed_packet) == "WB2OSZ>APRS,N2GH,W2UB:data");

    // -------------------------------------------------------------
    // Simple decrease hop count through router
    //
    // Input:  WB2OSZ>XXXX,WIDE1-3:data
    // Output: WB2OSZ>XXXX,WW1ABC*,WIDE1-2:data
    // -------------------------------------------------------------

    digi.callsign = "WW1ABC";
    digi.aliases = { "WIDE1" };

    p = {"WB2OSZ", "XXXX", {"WIDE1-3"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(p) == to_string(result.original_packet));
    EXPECT_TRUE(to_string(result.routed_packet) == "WB2OSZ>XXXX,WW1ABC*,WIDE1-2:data");

    // -------------------------------------------------------------
    // Route the digipeated packet through 2nd digipeater
    //
    // Input:  WB2OSZ>XXXX,WW1ABC*,WIDE1-2:data
    // Output: WB2OSZ>XXXX,WW1ABC,WW2DEF*,WIDE1-1:data
    // -------------------------------------------------------------

    digi.callsign = "WW2DEF";
    digi.aliases = { "WIDE1" };

    p = {"WB2OSZ", "XXXX", { "WW1ABC*", "WIDE1-2"}, "data" };
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(p) == to_string(result.original_packet));
    EXPECT_TRUE(to_string(result.routed_packet) == "WB2OSZ>XXXX,WW1ABC,WW2DEF*,WIDE1-1:data");

    // -------------------------------------------------------------
    // Route the digipeated packet through 3rd digipeater
    //
    // Input:  WB2OSZ>XXXX,WW1ABC,WW2DEF*,WIDE1-1:data
    // Output: WB2OSZ>XXXX,WW1ABC,WW2DEF,W3GHI*:data
    // -------------------------------------------------------------

    digi.callsign = "W3GHI";
    digi.aliases = { "WIDE1" };

    p = {"WB2OSZ", "XXXX", { "WW1ABC", "WW2DEF*", "WIDE1-1"}, "data" };
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(p) == to_string(result.original_packet));
    EXPECT_TRUE(to_string(result.routed_packet) == "WB2OSZ>XXXX,WW1ABC,WW2DEF,W3GHI*:data");

    // -------------------------------------------------------------
    // Routing a packet that has exaused hops
    //
    // Input:  N0CALL>APRS,WIDE2-2,qAR,N0CALL:data
    // Output: N0CALL>APRS,W7ION-10*,WIDE2-1,qAR,N0CALL:data (no change)
    // -------------------------------------------------------------

    digi.aliases = { "WIDE2" };
    digi.callsign = "W7ION-10";

    p = {"N0CALL", "APRS", {"OH7RDB", "OH7RDC", "WIDE2*"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == false);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::not_routed);
    EXPECT_TRUE(to_string(p) == to_string(result.original_packet));
    EXPECT_TRUE(to_string(result.original_packet) == to_string(result.routed_packet));    

    // -------------------------------------------------------------
    // Routing a packet
    //
    // Input:  N0CALL>APRS,WIDE2-2,qAR,N0CALL:data
    // Output: N0CALL>APRS,W7ION-10*,WIDE2-1,qAR,N0CALL:data
    // -------------------------------------------------------------

    p = {"N0CALL", "APRS", {"WIDE2-2", "qAR", "N0CALL"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,W7ION-10*,WIDE2-1,qAR,N0CALL:data");

    // -------------------------------------------------------------
    // Routing same packet but with a different digipeater
    //
    // Input:  N0CALL>APRS,W7ION-10*,WIDE2-1,qAR,N0CALL:data
    // Output: N0CALL>APRS,W7ION-10,W7ION-5*,qAR,N0CALL:data
    // -------------------------------------------------------------
    
    digi.callsign = "W7ION-5";    
    try_route_packet(result.routed_packet, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,W7ION-10,W7ION-5*,qAR,N0CALL:data");

    // -------------------------------------------------------------
    // Packet is not routed as path is now decremented to WIDE2*
    // and there are no more remaining hops
    // -------------------------------------------------------------

    digi.callsign = "W7ION-1";
    routing_result result2;
    try_route_packet(result.routed_packet, digi, result2);
    EXPECT_TRUE(result2.routed == false);
    EXPECT_TRUE(to_string(result.routed_packet) == to_string(result2.routed_packet));

    // -------------------------------------------------------------
    // Package is not Routing as path is now decremented to WIDE2*
    // and there are no more remaining hops
    // -------------------------------------------------------------

    digi.callsign = "W7ION-10";
    try_route_packet(result.routed_packet, digi, result2);
    EXPECT_TRUE(result2.routed == false);
    EXPECT_TRUE(to_string(result.routed_packet) == to_string(result2.routed_packet));

    // -------------------------------------------------------------
    // Routing a packet that has been digipeated before
    //
    // Input:  N0CALL>APRS,OH7RDA,WIDE1*,WIDE2-1:data
    // Output: N0CALL>APRS,OH7RDA,WIDE1,OH7RDB*:data
    // -------------------------------------------------------------

    digi.callsign = "OH7RDB";
    digi.aliases = { "WIDE2", "WIDE1" };
    p = {"N0CALL", "APRS", {"OH7RDA", "WIDE1*", "WIDE2-1"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,OH7RDA,WIDE1,OH7RDB*:data");

    // -------------------------------------------------------------
    // Routing packet with no match in aliases
    //
    // Input:  N0CALL>APRS,WIDE3-3,OH7RDA:data
    // Output: N0CALL>APRS,WIDE3-3,OH7RDA:data (no change)
    // -------------------------------------------------------------

    digi.callsign = "OH7RDB";
    digi.aliases = {"WIDE1", "WIDE2"};
    p = {"N0CALL", "APRS", {"WIDE3-3", "OH7RDA"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == false);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::not_routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,WIDE3-3,OH7RDA:data");

    // -------------------------------------------------------------
    // Routing a packet with multiple aliases in the path
    //
    // Input:  N0CALL>APRS,WIDE1-1,WIDE2-2:data
    // Output: N0CALL>APRS,OH7RDB*,WIDE2-2:data
    // -------------------------------------------------------------

    digi.callsign = "OH7RDB";
    digi.aliases = {"WIDE1"};
    p = {"N0CALL", "APRS", {"WIDE1-1", "WIDE2-2"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,OH7RDB*,WIDE2-2:data");

    // -------------------------------------------------------------
    // Routing a packet that has an alias in the path
    //
    // Input:  N0CALL>APRS,WIDE1-1,OH7RDA:data
    // Output: N0CALL>APRS,OH7RDB*,OH7RDA:data
    // -------------------------------------------------------------

    digi.callsign = "OH7RDB";
    digi.aliases = { "WIDE1" };
    p = {"N0CALL", "APRS", {"WIDE1-1", "OH7RDA"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,OH7RDB*,OH7RDA:data");

    // -------------------------------------------------------------
    // Routing a packet that has an alias in the path but also the digipeater callsign
    //
    // Input:  N0CALL>APRS,WIDE1-1,OH7RDA:data
    // Output: N0CALL>APRS,WIDE1-1,OH7RDA*:data
    // -------------------------------------------------------------

    digi.callsign = "OH7RDA";
    digi.aliases = { "WIDE1" };
    p = {"N0CALL", "APRS", {"WIDE1-1", "OH7RDA"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,WIDE1-1,OH7RDA*:data"); // nice if N0CALL>APRS,OH7RDA*:data

    // -------------------------------------------------------------
    // Routing a packet that has an alias in the path but also the digipeater callsign
    //
    // Input:  N0CALL>APRS,WIDE1-2,OH7RDA:data
    // Output: N0CALL>APRS,OH7RDA*,WIDE1-1:data
    // -------------------------------------------------------------

    digi.callsign = "OH7RDA";
    digi.aliases = { "WIDE1" };
    p = {"N0CALL", "APRS", {"WIDE1-2", "OH7RDA"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,WIDE1-2,OH7RDA*:data"); // nice if N0CALL>APRS,OH7RDA*,WIDE1-2:data

    // -------------------------------------------------------------
    // Routing a packet that has an alias in the path but also the digipeater callsign
    //
    // Input:  N0CALL>APRS,OH7RDA,WIDE1-1:data
    // Output: N0CALL>APRS,OH7RDA*,WIDE1-1:data
    // -------------------------------------------------------------

    digi.callsign = "OH7RDA";
    digi.aliases = { "WIDE1" };
    p = {"N0CALL", "APRS", { "OH7RDA", "WIDE1-1"}, "data" };
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,OH7RDA*,WIDE1-1:data");

    // -------------------------------------------------------------
    // Routing a packet that been digipeated by the same digipeater
    //
    // Input:  N0CALL>APRS,OH7RDA*,WIDE1-1:data
    // Output: N0CALL>APRS,OH7RDA*,WIDE1-1:data (no change)
    // -------------------------------------------------------------

    digi.callsign = "OH7RDA";
    digi.aliases = { "WIDE1" };
    p = {"N0CALL", "APRS", { "OH7RDA*", "WIDE1-1" }, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == false);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::already_routed);
    EXPECT_TRUE(to_string(result.routed_packet) == to_string(p));

    // -------------------------------------------------------------
    // Attempting to route a packet through a digipeater that doesn't match any aliases
    //
    // Input:  N0CALL>APRS,WIDE2-2,OH7RDA:data
    // Output: N0CALL>APRS,WIDE2-2,OH7RDA:data (no change)
    // -------------------------------------------------------------

    digi.callsign = "OH7RDB";
    digi.aliases = {"WIDE1"};
    p = {"N0CALL", "APRS", {"WIDE2-2", "OH7RDA"}, "data"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == false);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::not_routed);
    EXPECT_TRUE(to_string(result.routed_packet) == to_string(p));   














    digi.callsign = "WB2OSZ-9";
    digi.aliases = {"TRACE3"};
    p = {"W1ABC", "TEST01", { "TRACE3-3" }, ""};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "W1ABC>TEST01,WB2OSZ-9*,TRACE3-2:");

    digi.callsign = "WB2OSZ-9";
    digi.aliases = {"TRACE3"};
    p = {"W1ABC", "TEST02", { "TRACE3-3" }, ""};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "W1ABC>TEST02,WB2OSZ-9*,TRACE3-2:");

    digi.callsign = "WB2OSZ-9";
    digi.aliases = {"TRACE3"};
    p = {"W1ABC", "TEST03", { "TRACE3-3" }, ""};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "W1ABC>TEST03,WB2OSZ-9*,TRACE3-2:");

    digi.callsign = "WB2OSZ-9";
    digi.aliases = {"TRACE3"};
    p = {"W1ABC", "TEST04", { "TRACE3-1" }, ""};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "W1ABC>TEST04,WB2OSZ-9*:");

    digi.callsign = "WB2OSZ-9";
    digi.aliases = {"WIDE3"};
    p = {"W1ABC", "TEST11", { "R1","R2","R3","R4","R5","R6*", "WIDE3-3" }, ""};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "W1ABC>TEST11,R1,R2,R3,R4,R5,R6,WB2OSZ-9*,WIDE3-2:");

    digi.callsign = "WB2OSZ-9";
    digi.aliases = {"WIDE3"};
    p = {"W1ABC", "TEST12", { "R1","R2","R3","R4","R5","R6","R7*", "WIDE3-3" }, ""};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "W1ABC>TEST12,R1,R2,R3,R4,R5,R6,R7*,WIDE3-2:"); // because only 9 addresses allowed

    digi.callsign = "WB2OSZ-9";
    digi.aliases = {"WIDE3"};
    p = {"W1ABC", "TEST13", { "R1","R2","R3","R4","R5","R6","R7*", "WIDE3-1" }, ""};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "W1ABC>TEST13,R1,R2,R3,R4,R5,R6,R7,WB2OSZ-9*:");

    digi.callsign = "WB2OSZ-9";
    digi.aliases = { "WIDE3", "WIDE2", "WIDE1" };
    p = {"K1CPD-1", "T2SR5R", { "RELAY*", "WIDE", "WIDE", "SGATE", "WIDE" }, ""};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == false);

    digi.callsign = "WB2OSZ-9";
    digi.aliases = {};
    p = {"W1XYZ", "TESTD", { "R1*" ,"WB2OSZ-9" }, "has explicit routing"};
    try_route_packet(p, digi, result);
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.success == true);
    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "W1XYZ>TESTD,R1,WB2OSZ-9*:has explicit routing");
}

TEST(router, simple_demo)
{
    using namespace aprs::router;

    routing_result result;
    router_settings digi;

    digi.callsign = "WW1ABC";
    digi.aliases = { "WIDE1" };

    // WB2OSZ>XXXX,WIDE1-3:data
    packet p = { "WB2OSZ", "XXXX", { "WIDE1-3" }, "data"};

    try_route_packet(p, digi, result);
    
    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(result.state == routing_state::routed);

    // WB2OSZ>XXXX,WW1ABC*,WIDE1-2:data
    EXPECT_TRUE(to_string(result.routed_packet) == "WB2OSZ>XXXX,WW1ABC*,WIDE1-2:data");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
