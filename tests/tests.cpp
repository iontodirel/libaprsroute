// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 Ion Todirel                                   //
// **************************************************************** //
//
// tests.cpp
//
// MIT License
//
// Copyright (c) 2024 Ion Todirel
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <gtest/gtest.h>
#ifndef APRS_ROUTE_DISABLE_AUTO_TESTING
#include <nlohmann/json.hpp>
#endif

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <locale>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4267 4127)
#endif

#include <fmt/format.h>
#include <fmt/color.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "../aprsroute.hpp"

#if defined(IS_LINUX_MAC) && !defined(APRS_ROUTE_DISABLE_AUTO_TESTING)
#include <signal.h>
#endif

using namespace aprs::router;
using namespace aprs::router::detail;

#define APRS_ROUTE_DISABLE_PACKET_LOOP_TEST

routing_result test_packet_routing_iteration(const packet& p, router_settings digi, std::vector<std::string> addresses,  std::vector<size_t> digipeated_indices, int count);

TEST(address, to_string)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    address s;
    s.text = "WIDE";
    s.n = 2;
    s.N = 1;
    s.mark = false;
    EXPECT_TRUE(to_string(s) == "WIDE2-1");

    s.mark = true;
    EXPECT_TRUE(to_string(s) == "WIDE2-1*");

    s.mark = true;
    s.N = 0;
    EXPECT_TRUE(to_string(s) == "WIDE2*");

    s.N = 0;
    s.n = 0;
    EXPECT_TRUE(to_string(s) == "WIDE*");
#else
    EXPECT_TRUE(true);
#endif
}

TEST(packet, to_string)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    packet p = { "FROM", "TO", { "WIDE1-1", "WIDE2-1" }, "data"};
    EXPECT_TRUE(to_string(p) == "FROM>TO,WIDE1-1,WIDE2-1:data");

    p = { "FROM", "TO", { "CALL*", "WIDE1", "WIDE2-1" }, "data"};
    EXPECT_TRUE(to_string(p) == "FROM>TO,CALL*,WIDE1,WIDE2-1:data");
#else
    EXPECT_TRUE(true);
#endif
}

TEST(address, try_parse_address)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    address s;
    std::string path;

    // -------------------------------------------------------------
    // Wide segment with both digits present
    // -------------------------------------------------------------

    path = "WIDE7-5";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 7);
    EXPECT_TRUE(s.N == 5);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.kind == address_kind::wide);

    // -------------------------------------------------------------
    // Wide segment with a marker (*) but no digits after dash
    // -------------------------------------------------------------

    path = "WIDE*";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == true);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.kind == address_kind::wide);

    // -------------------------------------------------------------
    // Wide segment with only leading digits
    // -------------------------------------------------------------

    path = "WIDE5";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 5);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.kind == address_kind::wide);

    // -------------------------------------------------------------
    // Wide segment with leading digits and a mark
    // -------------------------------------------------------------

    path = "WIDE5*";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == true);
    EXPECT_TRUE(s.n == 5);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.kind == address_kind::wide);

    // -------------------------------------------------------------
    // Q construct segment
    // -------------------------------------------------------------

    path = "qAR";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "qAR");
    EXPECT_TRUE(s.q == q_construct::qAR);
    EXPECT_TRUE(s.kind == address_kind::q);

    // -------------------------------------------------------------
    // Wide segment with both digits and a mark
    // -------------------------------------------------------------

    path = "WIDE7-7*";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == true);
    EXPECT_TRUE(s.n == 7);
    EXPECT_TRUE(s.N == 7);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.kind == address_kind::wide);

    // -------------------------------------------------------------
    // Other segment with text and digits
    // -------------------------------------------------------------

    path = "W7ION-10*";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == true);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.ssid == 10);
    EXPECT_TRUE(s.text == "W7ION-10");
    EXPECT_TRUE(s.kind == address_kind::other);

    // -------------------------------------------------------------
    // Segment with invalid callsign SSID
    // -------------------------------------------------------------

    path = "W7ION-1d";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.ssid == 0);
    EXPECT_TRUE(s.text == "W7ION-1d");
    EXPECT_TRUE(s.kind == address_kind::other);

    // -------------------------------------------------------------
    // Other segment without a mark (*) and with no n-N digits
    // -------------------------------------------------------------

    path = "N0CALL";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "N0CALL");
    EXPECT_TRUE(s.kind == address_kind::other);

    // -------------------------------------------------------------
    // Additional test cases to cover edge cases
    // -------------------------------------------------------------

    path = "WIDE-1"; // Should not be valid, missing leading digits
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE-1"); // Entire text should be preserved if parsing n-N fails
    EXPECT_TRUE(s.kind == address_kind::other);

    path = "*WIDE"; // Leading mark with no valid segment
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "*WIDE");
    EXPECT_TRUE(s.kind == address_kind::other);

    path = "WIDE0-4"; // 0 is not valid for the leading digit (1-7 is the valid range)
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE0-4");
    EXPECT_TRUE(s.kind == address_kind::other);

    path = "WIDE8-4"; // 8 is not valid for the leading digit (1-7 is the valid range)
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE8-4");
    EXPECT_TRUE(s.kind == address_kind::other);

    path = "WIDE2-0";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE2-0");
    EXPECT_TRUE(s.kind == address_kind::other);

    path = "WIDE4-2-0";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE4-2-0");
    EXPECT_TRUE(s.kind == address_kind::other);

    path = "WIDE4-10";
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE4-10");
    EXPECT_TRUE(s.kind == address_kind::other);

    path = "WIDE14-1"; // Not really valid, but we don't care that it parses to 4-1
    EXPECT_TRUE(try_parse_address(path, s));
    EXPECT_TRUE(s.n == 4);
    EXPECT_TRUE(s.N == 1);
    EXPECT_TRUE(s.text == "WIDE1");
    EXPECT_TRUE(s.kind == address_kind::other);
#else
    EXPECT_TRUE(true);
#endif
}

TEST(packet, try_decode_packet)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    std::string packet_string;
    packet p;

    // Test case: Basic packet with single path
    packet_string = "N0CALL>APRS,WIDE2-2:data";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == "data");
    EXPECT_TRUE(p.path.size() == 1);
    EXPECT_TRUE(p.path[0] == "WIDE2-2");
    EXPECT_TRUE(to_string(p) == packet_string);

    // Test case: Packet with multiple paths and a mark
    packet_string = "N0CALL>APRS,CALLA,CALLB,WIDE2*:data";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == "data");
    EXPECT_TRUE(p.path.size() == 3);
    EXPECT_TRUE(p.path[0] == "CALLA");
    EXPECT_TRUE(p.path[1] == "CALLB");
    EXPECT_TRUE(p.path[2] == "WIDE2*");
    EXPECT_TRUE(to_string(p) == packet_string);

    // Test case: Packet with empty path
    packet_string = "N0CALL>APRS::data";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == ":data");
    EXPECT_TRUE(p.path.empty());
    EXPECT_TRUE(to_string(p) == packet_string);

    // Test case: Packet with missing data field
    packet_string = "N0CALL>APRS,WIDE2-2";
    EXPECT_FALSE(try_decode_packet(packet_string, p));

    // Test case: Packet with no path
    packet_string = "N0CALL>APRS:data";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == "data");
    EXPECT_TRUE(p.path.empty());
    EXPECT_TRUE(to_string(p) == packet_string);

    // Test case: Packet with invalid path format
    packet_string = "N0CALL>APRS,INVALID_PATH_FORMAT:data";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == "data");
    EXPECT_TRUE(p.path.size() == 1);
    EXPECT_TRUE(p.path[0] == "INVALID_PATH_FORMAT");
    EXPECT_TRUE(to_string(p) == packet_string);

    // Test case: Packet with special characters in path
    packet_string = "N0CALL>APRS,SPCL-@!,WIDE2*:data";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == "data");
    EXPECT_TRUE(p.path.size() == 2);
    EXPECT_TRUE(p.path[0] == "SPCL-@!");
    EXPECT_TRUE(p.path[1] == "WIDE2*");
    EXPECT_TRUE(to_string(p) == packet_string);

    // Test case: Packet with only source and destination
    packet_string = "N0CALL>APRS:";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data.empty());
    EXPECT_TRUE(p.path.empty());
    EXPECT_TRUE(to_string(p) == packet_string);

    // Test case: Packet with source, destination, and data, but no path
    packet_string = "N0CALL>APRS:data";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == "data");
    EXPECT_TRUE(p.path.empty());
    EXPECT_TRUE(to_string(p) == packet_string);

    // Test case: Packet with only data
    packet_string = "N0CALL:data";
    EXPECT_FALSE(try_decode_packet(packet_string, p));

    // Test case: with only source and destination
    packet_string = "N0CALL>APRS";
    EXPECT_FALSE(try_decode_packet(packet_string, p));

    // Test case: with only source and destination, and one empty path
    packet_string = "N0CALL>APRS,";
    EXPECT_FALSE(try_decode_packet(packet_string, p));

    // Test case: with only source and empty destination
    packet_string = "N0CALL>";
    EXPECT_FALSE(try_decode_packet(packet_string, p));

    // Test case: with only source
    packet_string = "N0CALL";
    EXPECT_FALSE(try_decode_packet(packet_string, p));

    // Test case: with some empty addresses
    packet_string = "N0CALL>APRS,,CALLA,,CALLB:data";
    EXPECT_TRUE(try_decode_packet(packet_string, p));
    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == "data");
    EXPECT_TRUE(p.path.size() == 4);
    EXPECT_TRUE(p.path[0] == "");
    EXPECT_TRUE(p.path[1] == "CALLA");
    EXPECT_TRUE(p.path[2] == "");
    EXPECT_TRUE(p.path[3] == "CALLB");
    EXPECT_TRUE(to_string(p) == packet_string);
#else
    EXPECT_TRUE(true);
#endif
}

TEST(packet, try_decode_packet_ctor)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    packet p = "N0CALL>APRS,WIDE2-2:data";

    EXPECT_TRUE(p.from == "N0CALL");
    EXPECT_TRUE(p.to == "APRS");
    EXPECT_TRUE(p.data == "data");
    EXPECT_TRUE(p.path.size() == 1);
    EXPECT_TRUE(p.path[0] == "WIDE2-2");

    EXPECT_TRUE(p == "N0CALL>APRS,WIDE2-2:data");
#else
    EXPECT_TRUE(true);
#endif
}

TEST(packet, equality)
{
    // Test: Equal packets
    packet p1{"N0CALL", "APRS", {"CALLA", "CALLB"}, "data"};
    packet p2{"N0CALL", "APRS", {"CALLA", "CALLB"}, "data"};
    EXPECT_TRUE(p1 == p2);

    // Test: Different 'from' field
    packet p3{"OTHER", "APRS", {"CALLA", "CALLB"}, "data"};
    EXPECT_FALSE(p1 == p3);

    // Test: Different 'to' field
    packet p4{"N0CALL", "OTHER", {"CALLA", "CALLB"}, "data"};
    EXPECT_FALSE(p1 == p4);

    // Test: Different path size
    packet p5{"N0CALL", "APRS", {"CALLA"}, "data"};
    EXPECT_FALSE(p1 == p5);

    // Test: Different path contents
    packet p6{"N0CALL", "APRS", {"CALLA", "CALLC"}, "data"};
    EXPECT_FALSE(p1 == p6);

    // Test: Different data
    packet p7{"N0CALL", "APRS", {"CALLA", "CALLB"}, "other_data"};
    EXPECT_FALSE(p1 == p7);

    // Test: All fields empty
    packet p8{"", "", {}, ""};
    packet p9{"", "", {}, ""};
    EXPECT_TRUE(p8 == p9);
}

TEST(router, try_route_packet_explicit_loop)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    routing_result result;
    router_settings digi;

    // -------------------------------------------------------------
    // Routing a packet through multiple routers
    //
    // Input:  N0CALL>APRS,DIGIA,DIGIB,DIGIC,DIGID,DIGIE,DIGIF,DIGIG,DIGIH:data
    // Output: N0CALL>APRS,DIGIA,DIGIB,DIGIC,DIGID,DIGIE,DIGIF,DIGIG,DIGIH*:data
    // -------------------------------------------------------------

    digi.path = {};

    packet p = {"N0CALL", "APRS", {"DIGIA","DIGIB","DIGIC","DIGID","DIGIE","DIGIF","DIGIG","DIGIH"}, "data"};
    result.routed_packet = p;

    std::vector<size_t> digipeated_indices = { 0, 1, 2, 3, 4, 5, 6, 7 };

    result = test_packet_routing_iteration(p, digi, p.path, digipeated_indices, 8);

    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,DIGIA,DIGIB,DIGIC,DIGID,DIGIE,DIGIF,DIGIG,DIGIH*:data");
#else
    EXPECT_TRUE(true);
#endif
}

TEST(router, try_route_packet_n_N_loop)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    routing_result result;
    router_settings digi;

    // -------------------------------------------------------------
    // Routing a packet through multiple N-hops and multiple routers
    //
    // Input:  N0CALL>APRS,WIDE1-2,CALL,WIDE2-2,ROUTE,WIDE3-2:data
    // Output: N0CALL>APRS,DIGI1,DIGI2,CALL,DIGI3,DIGI4,ROUTE,DIGI5,DIGI6*:data
    // -------------------------------------------------------------

    digi.path = {"WIDE1", "WIDE2", "WIDE3"};
    digi.options = routing_option::substitute_complete_hops;

    packet p = {"N0CALL", "APRS", {"WIDE1-2", "CALL", "WIDE2-2", "ROUTE", "WIDE3-2"}, "data"};
    result.routed_packet = p;

    std::vector<size_t> digipeated_indices = { 0, 1, 3, 4, 6, 7 };
    std::vector<std::string> digipeater_addresses = {"DIGI1","DIGI2","DIGI3","DIGI4","DIGI5","DIGI6"};

    result = test_packet_routing_iteration(p, digi, digipeater_addresses, digipeated_indices, 6);

    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,DIGI1,DIGI2,CALL,DIGI3,DIGI4,ROUTE,DIGI5,DIGI6*:data");

    // -------------------------------------------------------------
    // Routing a packet through multiple N-hops and multiple routers
    //
    // Input:  N0CALL>APRS,WIDE1-1,WIDE2-7:data
    // Output: N0CALL>APRS,DIGI1,DIGI2,CALL,DIGI3,DIGI4,ROUTE,DIGI5,DIGI6*:data
    // -------------------------------------------------------------

    digi.path = {"WIDE1","WIDE2"};
    digi.options = routing_option::substitute_complete_hops;

    p = {"N0CALL", "APRS", {"WIDE1-1", "WIDE2-7"}, "data"};
    result.routed_packet = p;

    digipeated_indices = { 0, 1, 2, 3, 4, 5, 6, 7 };
    digipeater_addresses = {"DIGI1","DIGI2","DIGI3","DIGI4","DIGI5","DIGI6","DIGI7","DIGI8"};

    result = test_packet_routing_iteration(p, digi, digipeater_addresses, digipeated_indices, 8);

    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,DIGI1,DIGI2,DIGI3,DIGI4,DIGI5,DIGI6,DIGI7,DIGI8*:data");
#else
    EXPECT_TRUE(true);
#endif
}

TEST(routing_option, enum_has_flag)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    routing_option op = routing_option::preempt_front | routing_option::substitute_complete_hops;

    EXPECT_TRUE(enum_has_flag(op, routing_option::preempt_front));
    EXPECT_TRUE(enum_has_flag(op, routing_option::substitute_complete_hops));
    EXPECT_TRUE(enum_has_flag(op, routing_option::route_self) == false);
#else
    EXPECT_TRUE(true);
#endif
}

TEST(router, simple_demo)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    using namespace aprs::router;

    router_settings digi { "DIGI", { "WIDE1" } };
    routing_result result;

    packet p = { "N0CALL", "APRS", { "WIDE1-3" }, "data" }; // N0CALL>APRS,WIDE1-3:data

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.state == routing_state::routed);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,DIGI*,WIDE1-2:data"); // N0CALL>APRS,DIGI*,WIDE1-2:data
#else
    EXPECT_TRUE(true);
#endif
}

TEST(router, try_route_packet_loop)
{
#ifndef APRS_ROUTE_DISABLE_PACKET_LOOP_TEST
    auto start = std::chrono::high_resolution_clock::now();

    // Typically about 500K packets / second (Intel Celeron N5095, 8GB RAM)

    for (int i = 0; i < 1000000; i++)
    {
        routing_result result;
        router_settings digi;

        digi.address = "DIGI";
        digi.path = {"WIDE4"};

        packet p = {"N0CALL", "APRS", {"CALL", "WIDE1-3", "TEMP", "WIDE2-3", "ROUTE", "WIDE3-3", "ROUTER", "WIDE4-3"}, "The quick brown fox jumps over the lazy dog."};

        try_route_packet(p, digi, result);

        EXPECT_TRUE(result.routed == true);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Benchmark: " << elapsed.count() << " ms" << std::endl;
#else
    EXPECT_TRUE(true);
#endif
}

routing_result test_packet_routing_iteration(const packet& p, router_settings digi, std::vector<std::string> addresses,  std::vector<size_t> digipeated_indices, int count)
{
    routing_result result;

    result.routed_packet = p;

    for (int i = 1; i <= count; i++)
    {
        digi.address = addresses[i - 1];

        try_route_packet(result.routed_packet, digi, result);
        EXPECT_TRUE(result.routed == true);
        EXPECT_TRUE(result.success == true);
        EXPECT_TRUE(result.state == routing_state::routed);

        EXPECT_TRUE(result.routed_packet.path[digipeated_indices[i - 1]] == (digi.address + "*"));

        printf("%s\n", to_string(result.routed_packet).c_str());

        for (size_t j = 0; j < result.routed_packet.path.size(); j++)
        {
            if (digipeated_indices[i - 1] != j)
            {
                EXPECT_TRUE(result.routed_packet.path[j].back() != '*');
            }
        }
    }

    return result;
}

#ifndef APRS_ROUTE_DISABLE_AUTO_TESTING

struct route_test
{
    bool mark = false;
    std::string id;
    bool debug = false;
    std::string comment;
    std::string address;
    std::string path;
    std::string original_packet;
    std::string routed_packet;
    bool routed = false;
    std::string options;
};

void test_diagnostics_reconstruct_packet_by_index(const routing_result& r);
void test_diagnostics_reconstruct_packet_by_start_end(const routing_result& r);

std::string to_lower(const std::string &str)
{
    std::locale loc;
    std::string s;
    s.resize(str.size());
    for (size_t i = 0; i < str.size(); i++)
        s[i] = std::tolower(str[i], loc);
    return s;
}

bool try_parse_bool(const std::string &s, bool &b)
{
    std::string s_lower = to_lower(s);
    if (s_lower == "true")
        b = true;
    else if (s_lower == "false")
        b = false;
    else
        return false;
    return true;
}

std::vector<route_test> load_routing_tests(const std::string& test_file)
{
    if (!std::filesystem::exists(test_file))
    {
        return {};
    }

    std::ifstream i;
    try
    {
        i.open(test_file);
    }
    catch (std::ios_base::failure&)
    {
        return {};
    }

    if (!i.is_open())
    {
        return {};
    }

    nlohmann::json j;

    try
    {
        j = nlohmann::json::parse(i, nullptr, true, /*ignore comments*/ true);
    }
    catch (nlohmann::json::parse_error&)
    {
        return {};
    }
    catch (nlohmann::json::type_error&)
    {
        return {};
    }

    std::vector<route_test> route_tests;

    if (j.contains("routes"))
    {
        nlohmann::json routes = j["routes"];
        for (const auto& route : routes)
        {
            if (route.contains("original_packet") &&
                route.contains("routed_packet") && route.contains("routed"))
            {
                route_test test;

                test.original_packet = route["original_packet"].get<std::string>();
                test.routed_packet = route["routed_packet"].get<std::string>();

                try_parse_bool(route["routed"].get<std::string>(), test.routed);

                if (route.contains("address"))
                {
                    test.address = route["address"].get<std::string>();
                }

                if (route.contains("path"))
                {
                    test.path = route["path"].get<std::string>();
                }

                if (route.contains("options"))
                {
                    test.options = route["options"].get<std::string>();
                }

                if (route.contains("comment"))
                {
                    test.comment = route["comment"].get<std::string>();
                }

                if (route.contains("debug"))
                {
                    try_parse_bool(route["debug"].get<std::string>(), test.debug);
                }

                if (route.contains("mark"))
                {
                    try_parse_bool(route["mark"].get<std::string>(), test.mark);
                }

                if (route.contains("id"))
                {
                    test.id = route["id"].get<std::string>();
                }

                route_tests.push_back(test);
            }
        }
    }

    return route_tests;
}

std::vector<std::string> split_comma_separated_values(const std::string& str)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        result.push_back(item);
    }

    return result;
}

bool try_get_routing_test_set(route_test test, packet& p, router_settings& settings)
{
    try_decode_packet(test.original_packet, p);

    settings.address = test.address;
    settings.path = split_comma_separated_values(test.path);

    settings.options = routing_option::none;

    std::vector<std::string> options = split_comma_separated_values(test.options);
    for (size_t i = 0; i < options.size(); i++)
    {
        routing_option option = routing_option::none;
        if (try_parse_routing_option(options[i], option))
        {
            settings.options = settings.options | option;
        }
    }

    return true;
}

void debugger_break()
{
#ifdef _MSC_VER
        __debugbreak();
#else
        raise(SIGTRAP);
#endif
}

void run_test(const route_test& test, const packet& p, const router_settings& settings)
{
    routing_result result;

    bool result_bool = try_route_packet(p, settings, result);

    EXPECT_TRUE(result_bool == test.routed);
    EXPECT_TRUE(result.routed == test.routed);
    EXPECT_TRUE(result.success == true);

    if (test.routed)
    {
        EXPECT_TRUE(result.state == routing_state::routed);
    }
    else
    {
        EXPECT_TRUE(result.state != routing_state::routed);
    }

    if (!test.routed_packet.empty())
    {
        bool routed_packet_result = to_string(result.routed_packet) == test.routed_packet;
        EXPECT_TRUE(routed_packet_result);
        if (!routed_packet_result)
        {
            printf("test failed\n");
            if (test.debug == true)
            {
                debugger_break();
            }
            // route again for debugging purposes
            try_route_packet(p, settings, result);
        }
        EXPECT_TRUE(to_string(result.original_packet) == test.original_packet);
    }

    if (!test.routed)
    {
        EXPECT_TRUE(to_string(result.original_packet) == to_string(result.routed_packet));
    }

    std::string diag_string = aprs::router::to_string(result);

    printf("%s", diag_string.c_str());

    test_diagnostics_reconstruct_packet_by_index(result);
    test_diagnostics_reconstruct_packet_by_start_end(result);
}

void test_diagnostics_reconstruct_packet_by_index(const routing_result& result)
{
    using namespace aprs::router;
    using namespace aprs::router::detail;

    if (result.state != routing_state::routed)
    {
        return;
    }

    EXPECT_TRUE(result.actions.size() > 0);

    packet routed_packet = result.original_packet;

    for (const auto& a : result.actions)
    {
        if (a.type == routing_action::remove)
        {
            routed_packet.path.erase(routed_packet.path.begin() + a.index);
        }
        else if (a.type == routing_action::insert)
        {
            routed_packet.path.insert(routed_packet.path.begin() + a.index, a.address);
        }
        else if (a.type == routing_action::set)
        {
            routed_packet.path[a.index].append("*");
        }
        else if (a.type == routing_action::unset || a.type == routing_action::replace ||
            a.type == routing_action::decrement)
        {
            routed_packet.path[a.index] = a.address;
        }
        else
        {
            EXPECT_TRUE(false);
        }
    }

    bool result_bool = result.routed_packet == routed_packet;

    EXPECT_TRUE(result_bool);

    if (!result_bool)
    {
        // handy breakpoint location
        printf("test failed\n");
    }
}

void test_diagnostics_reconstruct_packet_by_start_end(const routing_result& result)
{
    using namespace aprs::router;
    using namespace aprs::router::detail;

    if (result.state != routing_state::routed)
    {
        return;
    }

    EXPECT_TRUE(result.actions.size() > 0);

    std::string routed_packet = to_string(result.original_packet);

    for (const auto& a : result.actions)
    {
        size_t start = a.start;
        size_t end = a.end;
        size_t count = a.end - a.start;

        if (a.type == routing_action::remove)
        {
            if (routed_packet[end + 1] == ':' || a.index == 0)
            {
                count++;
            }
            else if (a.index != 0)
            {
                start--;
                count++;
            }
            routed_packet.erase(start, count);
        }
        else if (a.type == routing_action::insert)
        {
            routed_packet.insert(start, a.address);
            if (routed_packet[end + 1] != ',' || a.index == 0)
            {
                routed_packet.insert(end, ",");
            }
        }
        else if (a.type == routing_action::set)
        {
            routed_packet.insert(end, "*");
        }
        else if (a.type == routing_action::unset || a.type == routing_action::replace ||
            a.type == routing_action::decrement)
        {
            routed_packet.erase(start, count);
            routed_packet.insert(start, a.address);
        }
        else
        {
            EXPECT_TRUE(false);
        }
    }

    bool result_bool = to_string(result.routed_packet) == routed_packet;

    EXPECT_TRUE(result_bool);

    if (!result_bool)
    {
        // handy breakpoint location
        printf("test failed\n");
    }
}

bool has_marked_tests(const std::vector<route_test>& tests)
{
    for (const auto& test : tests)
    {
        if (test.mark)
        {
            return true;
        }
    }
    return false;
}

#endif

TEST(router, try_route_packet_auto_tests)
{
#ifndef APRS_ROUTE_DISABLE_AUTO_TESTING
    std::vector<route_test> tests = load_routing_tests(INPUT_TEST_FILE);
    EXPECT_TRUE(tests.empty() == false);
    bool has_marked_test = has_marked_tests(tests);
    for (const auto& test : tests)
    {
        if (has_marked_test && !test.mark)
        {
            continue;
        }

        packet p;

        router_settings settings;
        settings.enable_diagnostics = true;

        if (try_get_routing_test_set(test, p, settings))
        {
            run_test(test, p, settings);
        }
    }
#else
    EXPECT_TRUE(true);
#endif
}

TEST(address, try_parse_callsign)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    std::string address;
    std::string callsign;
    int ssid = 0;

    address = "A0BCDE-12";
    EXPECT_TRUE(try_parse_callsign(address, callsign, ssid));
    EXPECT_TRUE(callsign == "A0BCDE");
    EXPECT_TRUE(ssid == 12);

    address = "A0BCDE-12*";
    EXPECT_FALSE(try_parse_callsign(address, callsign, ssid));

    address = "A0BCDE-12*";
    EXPECT_TRUE(try_parse_callsign_with_used_flag(address, callsign, ssid));
    EXPECT_TRUE(callsign == "A0BCDE");
    EXPECT_TRUE(ssid == 12);

    address = "N0CALL";
    EXPECT_TRUE(try_parse_callsign(address, callsign, ssid));
    EXPECT_TRUE(callsign == "N0CALL");
    EXPECT_TRUE(ssid == 0);

    address = "N0CALL-01";
    EXPECT_FALSE(try_parse_callsign(address, callsign, ssid));

    address = "N0CALL-";
    EXPECT_FALSE(try_parse_callsign(address, callsign, ssid));

    address = "N0CALL-0";
    EXPECT_FALSE(try_parse_callsign(address, callsign, ssid));

    address = "N0CALL-100";
    EXPECT_FALSE(try_parse_callsign(address, callsign, ssid));

    address = "N0CALL-dd";
    EXPECT_FALSE(try_parse_callsign(address, callsign, ssid));

    address = "N0CALL-WX";
    EXPECT_FALSE(try_parse_callsign(address, callsign, ssid));

    address = "N0CALL-20";
    EXPECT_FALSE(try_parse_callsign(address, callsign, ssid));
#else
    EXPECT_TRUE(true);
#endif
}

TEST(router, try_route_packet_enable_diagnostics)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    using namespace aprs::router;

    router_settings digi { "DIGI", { "WIDE1" }, routing_option::none, true };
    routing_result result;

    // N0CALL>APRS,CALL,WIDE1,DIGI*:data
    //                        ~~~~~
    //                        23 28 - Packet has finished routing.

    packet p = { "N0CALL", "APRS", { "CALL", "WIDE1", "DIGI*" }, "data"};
    std::string packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 1);

    const routing_diagnostic& diag = result.actions[0];

    EXPECT_TRUE(diag.address == "DIGI");
    EXPECT_TRUE(diag.target == applies_to::path);
    EXPECT_TRUE(diag.type == routing_action::warn);
    EXPECT_TRUE(diag.start == 23);
    EXPECT_TRUE(diag.end == 28);
    EXPECT_TRUE(diag.index == 2);

    std::string action_address_str = packet_string.substr(result.actions[0].start, result.actions[0].end - result.actions[0].start);

    EXPECT_TRUE(action_address_str == "DIGI*");

    // N0CALL>APRS,CALL,DIGI*,WIDE1-1:data
    //                  ~~~~~
    //                  17 22 - Packet has already been routed.

    p = { "N0CALL", "APRS", { "CALL", "DIGI*", "WIDE1-1" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 1);

    EXPECT_TRUE(result.actions[0].address == "DIGI");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::warn);
    EXPECT_TRUE(result.actions[0].start == 17);
    EXPECT_TRUE(result.actions[0].end == 22);
    EXPECT_TRUE(result.actions[0].index == 1);

    action_address_str = packet_string.substr(result.actions[0].start, result.actions[0].end - result.actions[0].start);

    EXPECT_TRUE(action_address_str == "DIGI*");

    // N0CALL>APRS,A,B,C,D,E,F,G:data
    //             ~
    //           12 13 - Packet address replaced
    //                   address = DIGI
    //
    // N0CALL>APRS,DIGI*,B,C,D,E,F,G:data
    //             ~~~~
    //             12 16 - Packet address marked as 'set'

    digi.path = { "A" };
    digi.options = routing_option::substitute_explicit_address;
    p = { "N0CALL", "APRS", { "A", "B", "C", "D", "E", "F", "G" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 2);

    EXPECT_TRUE(result.actions[0].address == "DIGI");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::replace);
    EXPECT_TRUE(result.actions[0].start == 12);
    EXPECT_TRUE(result.actions[0].end == 13);
    EXPECT_TRUE(result.actions[0].index == 0);

    EXPECT_TRUE(result.actions[1].address == "DIGI");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::set);
    EXPECT_TRUE(result.actions[1].start == 12);
    EXPECT_TRUE(result.actions[1].end == 16);
    EXPECT_TRUE(result.actions[1].index == 0);

    // N0CALL>APRS,A,B,C*,D,E,F,G:data
    //                    ~
    //                    19 20 - Packet address 'D' replaced with address 'DIGI'
    //
    // N0CALL>APRS,A,B,C*,DIGI,E,F,G:data
    //                 ~~
    //                 16 18 - Packet address marked as 'unset'
    //
    // N0CALL>APRS,A,B,C,DIGI,E,F,G:data
    //                   ~~~~
    //                   18 22 - Packet address marked as 'set'

    digi.path = { "D" };
    digi.options = routing_option::substitute_explicit_address;
    p = { "N0CALL", "APRS", { "A", "B", "C*", "D", "E", "F", "G" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 3);

    EXPECT_TRUE(result.actions[0].address == "DIGI");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::replace);
    EXPECT_TRUE(result.actions[0].start == 19);
    EXPECT_TRUE(result.actions[0].end == 20);
    EXPECT_TRUE(result.actions[0].index == 3);

    EXPECT_TRUE(result.actions[1].address == "C");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::unset);
    EXPECT_TRUE(result.actions[1].start == 16);
    EXPECT_TRUE(result.actions[1].end == 18);
    EXPECT_TRUE(result.actions[1].index == 2);

    EXPECT_TRUE(result.actions[2].address == "DIGI");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::set);
    EXPECT_TRUE(result.actions[2].start == 18);
    EXPECT_TRUE(result.actions[2].end == 22);
    EXPECT_TRUE(result.actions[2].index == 3);

    // N0CALL>APRS,A,B,C,D,E,F,G:data
    //             ~
    //            12 13 - Packet address marked as 'set'

    digi.address = { "A" };
    digi.path = {};
    digi.options = routing_option::none;
    p = { "N0CALL", "APRS", { "A", "B", "C", "D", "E", "F", "G" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 1);

    EXPECT_TRUE(result.actions[0].address == "A");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::set);
    EXPECT_TRUE(result.actions[0].start == 12);
    EXPECT_TRUE(result.actions[0].end == 13);
    EXPECT_TRUE(result.actions[0].index == 0);

    // N0CALL>APRS,AB,ABC,ABCD,ABCDE:data
    //                         ~~~~~
    //                         24 29 - Packet address removed
    //
    // N0CALL>APRS,ABCDE,AB,ABC,ABCD:data
    //             ~~~~~
    //             12 17 - Packet address inserted
    //
    // N0CALL>APRS,ABCDE,AB,ABC,ABCD:data
    //             ~~~~~
    //             12 17 - Packet address marked as 'set'

    digi.address = { "ABCDE" };
    digi.path = {};
    digi.options = routing_option::preempt_front;
    p = { "N0CALL", "APRS", { "AB", "ABC", "ABCD", "ABCDE" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 3);

    EXPECT_TRUE(result.actions[0].address == "ABCDE");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::remove);
    EXPECT_TRUE(result.actions[0].start == 24);
    EXPECT_TRUE(result.actions[0].end == 29);
    EXPECT_TRUE(result.actions[0].index == 3);

    EXPECT_TRUE(result.actions[1].address == "ABCDE");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::insert);
    EXPECT_TRUE(result.actions[1].start == 12);
    EXPECT_TRUE(result.actions[1].end == 17);
    EXPECT_TRUE(result.actions[1].index == 0);

    EXPECT_TRUE(result.actions[2].address == "ABCDE");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::set);
    EXPECT_TRUE(result.actions[2].start == 12);
    EXPECT_TRUE(result.actions[2].end == 17);
    EXPECT_TRUE(result.actions[2].index == 0);

    // N0CALL>APRS,CALLA,CALLB:data
    //                   ~~~~~
    //                   18 23 - Packet address removed
    //
    // N0CALL>APRS,CALLB,CALLA:data
    //             ~~~~~
    //             12 17 - Packet address inserted
    //
    // N0CALL>APRS,DIGI,CALLB,CALLA:data
    //             ~~~~
    //             12 16 - Packet address inserted
    //
    // N0CALL>APRS,DIGI,CALLB*,CALLA:data
    //                  ~~~~~
    //                  17 22 - Packet address marked as 'set'

    digi.address = { "DIGI" };
    digi.path = { "CALLB" };
    digi.options = routing_option::preempt_front; // todo + substitute_explicit_address
    p = { "N0CALL", "APRS", { "CALLA", "CALLB" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 4);

    EXPECT_TRUE(result.actions[0].address == "CALLB");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::remove);
    EXPECT_TRUE(result.actions[0].start == 18);
    EXPECT_TRUE(result.actions[0].end == 23);
    EXPECT_TRUE(result.actions[0].index == 1);

    EXPECT_TRUE(result.actions[1].address == "CALLB");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::insert);
    EXPECT_TRUE(result.actions[1].start == 12);
    EXPECT_TRUE(result.actions[1].end == 17);
    EXPECT_TRUE(result.actions[1].index == 0);

    EXPECT_TRUE(result.actions[2].address == "DIGI");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::insert);
    EXPECT_TRUE(result.actions[2].start == 12);
    EXPECT_TRUE(result.actions[2].end == 16);
    EXPECT_TRUE(result.actions[2].index == 0);

    EXPECT_TRUE(result.actions[3].address == "CALLB");
    EXPECT_TRUE(result.actions[3].target == applies_to::path);
    EXPECT_TRUE(result.actions[3].type == routing_action::set);
    EXPECT_TRUE(result.actions[3].start == 17);
    EXPECT_TRUE(result.actions[3].end == 22);
    EXPECT_TRUE(result.actions[3].index == 1);

    // N0CALL>APRS,CITYA*,CITYB,CITYC,CITYD,CITYE:data
    //             ~~~~~~
    //             12 18 - Packet address removed
    //
    // N0CALL>APRS,CITYB,CITYC,CITYD,CITYE:data
    //             ~~~~~
    //             12 17 - Packet address removed
    //
    // N0CALL>APRS,CITYC,CITYD,CITYE:data
    //             ~~~~~
    //             12 17 - Packet address removed
    //
    // N0CALL>APRS,CITYD,CITYE:data
    //             ~~~~~
    //             12 17 - Packet address marked as 'set'

    digi.address = { "CITYD" };
    digi.path = {};
    digi.options = routing_option::preempt_drop;
    p = { "N0CALL", "APRS", { "CITYA*", "CITYB", "CITYC", "CITYD", "CITYE" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 4);

    EXPECT_TRUE(result.actions[0].address == "CITYA");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::remove);
    EXPECT_TRUE(result.actions[0].start == 12);
    EXPECT_TRUE(result.actions[0].end == 18);
    EXPECT_TRUE(result.actions[0].index == 0);

    EXPECT_TRUE(result.actions[1].address == "CITYB");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::remove);
    EXPECT_TRUE(result.actions[1].start == 12);
    EXPECT_TRUE(result.actions[1].end == 17);
    EXPECT_TRUE(result.actions[1].index == 0);

    EXPECT_TRUE(result.actions[2].address == "CITYC");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::remove);
    EXPECT_TRUE(result.actions[2].start == 12);
    EXPECT_TRUE(result.actions[2].end == 17);
    EXPECT_TRUE(result.actions[2].index == 0);

    EXPECT_TRUE(result.actions[3].address == "CITYD");
    EXPECT_TRUE(result.actions[3].target == applies_to::path);
    EXPECT_TRUE(result.actions[3].type == routing_action::set);
    EXPECT_TRUE(result.actions[3].start == 12);
    EXPECT_TRUE(result.actions[3].end == 17);
    EXPECT_TRUE(result.actions[3].index == 0);

    // N0CALL>APRS,CALLA*,CALLB*,WIDE2-3:data
    //                           ~~~~~~~
    //                           26 33 - Packet address replaced
    //
    // N0CALL>APRS,CALLA*,CALLB*,WIDE2-3:data
    //             ~~~~~~
    //             12 18 - Packet address marked as 'unset'
    //
    // N0CALL>APRS,CALLA,CALLB*,WIDE2-3:data
    //                   ~~~~~~
    //                   18 24 - Packet address marked as 'unset'
    //
    // N0CALL>APRS,CALLA,CALLB,DIGI:data
    //                         ~~~~
    //                         24 28 - Packet address marked as 'set'

    digi.address = { "DIGI" };
    digi.path = { "WIDE2-2" };
    digi.options = routing_option::trap_excessive_hops;
    p = { "N0CALL", "APRS", { "CALLA*", "CALLB*", "WIDE2-3" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 4);

    EXPECT_TRUE(result.actions[0].address == "DIGI");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::replace);
    EXPECT_TRUE(result.actions[0].start == 26);
    EXPECT_TRUE(result.actions[0].end == 33);
    EXPECT_TRUE(result.actions[0].index == 2);

    EXPECT_TRUE(result.actions[1].address == "CALLA");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::unset);
    EXPECT_TRUE(result.actions[1].start == 12);
    EXPECT_TRUE(result.actions[1].end == 18);
    EXPECT_TRUE(result.actions[1].index == 0);

    EXPECT_TRUE(result.actions[2].address == "CALLB");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::unset);
    EXPECT_TRUE(result.actions[2].start == 18);
    EXPECT_TRUE(result.actions[2].end == 24);
    EXPECT_TRUE(result.actions[2].index == 1);

    EXPECT_TRUE(result.actions[3].address == "DIGI");
    EXPECT_TRUE(result.actions[3].target == applies_to::path);
    EXPECT_TRUE(result.actions[3].type == routing_action::set);
    EXPECT_TRUE(result.actions[3].start == 24);
    EXPECT_TRUE(result.actions[3].end == 28);
    EXPECT_TRUE(result.actions[3].index == 2);

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

    digi.address = { "DIGI" };
    digi.path = { "WIDE1", "WIDE2" };
    digi.options = routing_option::none;
    p = { "N0CALL", "APRS", { "WIDE1-2" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 3);

    EXPECT_TRUE(result.actions[0].address == "WIDE1-1");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::decrement);
    EXPECT_TRUE(result.actions[0].start == 12);
    EXPECT_TRUE(result.actions[0].end == 19);
    EXPECT_TRUE(result.actions[0].index == 0);

    EXPECT_TRUE(result.actions[1].address == "DIGI");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::insert);
    EXPECT_TRUE(result.actions[1].start == 12);
    EXPECT_TRUE(result.actions[1].end == 16);
    EXPECT_TRUE(result.actions[1].index == 0);

    EXPECT_TRUE(result.actions[2].address == "DIGI");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::set);
    EXPECT_TRUE(result.actions[2].start == 12);
    EXPECT_TRUE(result.actions[2].end == 16);
    EXPECT_TRUE(result.actions[2].index == 0);

    // N0CALL>APRS,CALL,WIDE1*,WIDE2-2:data
    //                         ~~~~~~~
    //                         24 31 - Packet address decremented
    //
    // N0CALL>APRS,CALL,WIDE1,DIGI,WIDE2-2:data
    //                        ~~~~
    //                        24 28 - Packet address inserted
    //
    // N0CALL>APRS,CALL,WIDE1*,WIDE2-2:data
    //                  ~~~~~~
    //                  17 23 - Packet address marked as 'unset'
    //
    // N0CALL>APRS,CALL,WIDE1,DIGI,WIDE2-2:data
    //                        ~~~~
    //                        23 27 - Packet address marked as 'set'

    digi.address = { "DIGI" };
    digi.path = { "WIDE2", "WIDE1" };
    digi.options = routing_option::none;
    p = { "N0CALL", "APRS", { "CALL", "WIDE1*", "WIDE2-2" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 4);

    EXPECT_TRUE(result.actions[0].address == "WIDE2-1");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::decrement);
    EXPECT_TRUE(result.actions[0].start == 24);
    EXPECT_TRUE(result.actions[0].end == 31);
    EXPECT_TRUE(result.actions[0].index == 2);

    EXPECT_TRUE(result.actions[1].address == "DIGI");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::insert);
    EXPECT_TRUE(result.actions[1].start == 24);
    EXPECT_TRUE(result.actions[1].end == 28);
    EXPECT_TRUE(result.actions[1].index == 2);

    EXPECT_TRUE(result.actions[2].address == "WIDE1");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::unset);
    EXPECT_TRUE(result.actions[2].start == 17);
    EXPECT_TRUE(result.actions[2].end == 23);
    EXPECT_TRUE(result.actions[2].index == 1);

    EXPECT_TRUE(result.actions[3].address == "DIGI");
    EXPECT_TRUE(result.actions[3].target == applies_to::path);
    EXPECT_TRUE(result.actions[3].type == routing_action::set);
    EXPECT_TRUE(result.actions[3].start == 23);
    EXPECT_TRUE(result.actions[3].end == 27);
    EXPECT_TRUE(result.actions[3].index == 2);

    // N0CALL>APRS,CALL1,CALL2,CALL3,CALL4,CALL5,CALL6*,WIDE3-3:data
    //                                                  ~~~~~~~
    //                                                  49 56 - Packet address decremented
    //
    // N0CALL>APRS,CALL1,CALL2,CALL3,CALL4,CALL5,CALL6,DIGI,WIDE3-3:data
    //                                                 ~~~~
    //                                                 49 53 - Packet address inserted
    //
    // N0CALL>APRS,CALL1,CALL2,CALL3,CALL4,CALL5,CALL6*,WIDE3-3:data
    //                                           ~~~~~~
    //                                           42 48 - Packet address marked as 'unset'
    //
    // N0CALL>APRS,CALL1,CALL2,CALL3,CALL4,CALL5,CALL6,DIGI,WIDE3-3:data
    //                                                 ~~~~
    //                                                 48 52 - Packet address marked as 'set'

    digi.address = { "DIGI" };
    digi.path = { "WIDE3" };
    digi.options = routing_option::none;
    p = { "N0CALL", "APRS", { "CALL1", "CALL2", "CALL3", "CALL4", "CALL5", "CALL6*", "WIDE3-3" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 4);

    EXPECT_TRUE(result.actions[0].address == "WIDE3-2");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::decrement);
    EXPECT_TRUE(result.actions[0].start == 49);
    EXPECT_TRUE(result.actions[0].end == 56);
    EXPECT_TRUE(result.actions[0].index == 6);

    EXPECT_TRUE(result.actions[1].address == "DIGI");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::insert);
    EXPECT_TRUE(result.actions[1].start == 49);
    EXPECT_TRUE(result.actions[1].end == 53);
    EXPECT_TRUE(result.actions[1].index == 6);

    EXPECT_TRUE(result.actions[2].address == "CALL6");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::unset);
    EXPECT_TRUE(result.actions[2].start == 42);
    EXPECT_TRUE(result.actions[2].end == 48);
    EXPECT_TRUE(result.actions[2].index == 5);

    EXPECT_TRUE(result.actions[3].address == "DIGI");
    EXPECT_TRUE(result.actions[3].target == applies_to::path);
    EXPECT_TRUE(result.actions[3].type == routing_action::set);
    EXPECT_TRUE(result.actions[3].start == 48);
    EXPECT_TRUE(result.actions[3].end == 52);
    EXPECT_TRUE(result.actions[3].index == 6);

    // N0CALL>APRS,,WIDE1-1:data
    //              ~~~~~~~
    //              13 20 - Packet address decremented
    //
    // N0CALL>APRS,,WIDE1:data
    //              ~~~~~
    //              13 18 - Packet address replaced
    //
    // N0CALL>APRS,,DIGI:data
    //              ~~~~
    //              13 17 - Packet address marked as 'set'
    //
    // N0CALL>APRS,,DIGI*:data
    //             ~
    //             12 12 - Packet address removed

    digi.address = { "DIGI" };
    digi.path = { "WIDE1" };
    digi.options = routing_option::substitute_complete_hops;
    p = { "N0CALL", "APRS", { "", "WIDE1-1" }, "data"};
    packet_string = to_string(p);

    try_route_packet(p, digi, result);

    EXPECT_TRUE(result.actions.size() == 4);

    EXPECT_TRUE(result.actions[0].address == "WIDE1");
    EXPECT_TRUE(result.actions[0].target == applies_to::path);
    EXPECT_TRUE(result.actions[0].type == routing_action::decrement);
    EXPECT_TRUE(result.actions[0].start == 13);
    EXPECT_TRUE(result.actions[0].end == 20);
    EXPECT_TRUE(result.actions[0].index == 1);

    EXPECT_TRUE(result.actions[1].address == "DIGI");
    EXPECT_TRUE(result.actions[1].target == applies_to::path);
    EXPECT_TRUE(result.actions[1].type == routing_action::replace);
    EXPECT_TRUE(result.actions[1].start == 13);
    EXPECT_TRUE(result.actions[1].end == 18);
    EXPECT_TRUE(result.actions[1].index == 1);

    EXPECT_TRUE(result.actions[2].address == "DIGI");
    EXPECT_TRUE(result.actions[2].target == applies_to::path);
    EXPECT_TRUE(result.actions[2].type == routing_action::set);
    EXPECT_TRUE(result.actions[2].start == 13);
    EXPECT_TRUE(result.actions[2].end == 17);
    EXPECT_TRUE(result.actions[2].index == 1);

    EXPECT_TRUE(result.actions[3].address == "");
    EXPECT_TRUE(result.actions[3].target == applies_to::path);
    EXPECT_TRUE(result.actions[3].type == routing_action::remove);
    EXPECT_TRUE(result.actions[3].start == 12);
    EXPECT_TRUE(result.actions[3].end == 12);
    EXPECT_TRUE(result.actions[3].index == 0);
#else
    EXPECT_TRUE(true);
#endif
}

TEST(router, try_route_packet_color_diagnostics)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    using namespace aprs::router;

    router_settings digi { "DIGI", { "WIDE1", "WIDE2" }, routing_option::none, true };
    routing_result result;

    packet p = { "N0CALL", "APRS", { "WIDE1-2" }, "data"};

    try_route_packet(p, digi, result);

    std::string diag_string = to_string(result);

    std::cout << diag_string << std::endl;

    routing_diagnostic_display_lines diag_lines = format(result);

    for (auto& l : diag_lines.lines)
    {
        l.message[0] = static_cast<char>(std::tolower(l.message[0]));

        fmt::print(fg(fmt::color::blue_violet), "note: ");
        fmt::print("{}\n", l.message);
        fmt::print(fg(fmt::color::gray), "{:4}{}\n", "", l.packet_string);
        fmt::print(fg(fmt::color::lime_green), "{:4}{}\n", "", l.highlight_string);
    }
#else
    EXPECT_TRUE(true);
#endif
}

TEST(routing_result, to_string)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    // N0CALL>APRS,CALLA,CALLB*,CALLC,CALLD,CALLE,CALLF:data
    //                                      ~~~~~
    //                                      37 42 - Packet address removed
    //
    // N0CALL>APRS,CALLA,CALLB*,CALLE,CALLC,CALLD,CALLF:data
    //                          ~~~~~
    //                          25 30 - Packet address inserted
    //
    // N0CALL>APRS,CALLA,CALLB*,CALLE,CALLC,CALLD,CALLF:data
    //                   ~~~~~~
    //                   18 24 - Packet address set as 'unset'
    //
    // N0CALL>APRS,CALLA,CALLB,CALLE,CALLC,CALLD,CALLF:data
    //                         ~~~~~
    //                         24 29 - Packet address set as 'set' 

    router_settings digi { "CALLE", { "" }, routing_option::preempt_front, true };
    routing_result result;

    packet p = { "N0CALL", "APRS", { "CALLA", "CALLB*", "CALLC", "CALLD", "CALLE", "CALLF" }, "data"};

    try_route_packet(p, digi, result);

    std::string diag_string = to_string(result);

    printf("%s\n", diag_string.c_str());

    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

TEST(addresses, set_address_as_used)
{
    using namespace aprs::router;
    using namespace aprs::router::detail;
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING

    packet p = { "N0CALL", "APRS", { "CALLA", "CALLB*", "CALLC", "CALLD", "CALLE", "CALLF" }, "data"};

    std::vector<address> segments;
    try_parse_addresses(p.path, segments);
    set_addresses_offset(p, segments);

    // N0CALL>APRS,CALLA,CALLB*,CALLC,CALLD,CALLE,CALLF:data
    //             ~~~~~ ~~~~~~ ~~~~~ ~~~~~ ~~~~~ ~~~~~
    //             12 17 18 24  25 30 31 36 37 42 43 48

    EXPECT_TRUE(segments[0].offset == 12);
    EXPECT_TRUE(segments[0].length == 5);
    EXPECT_TRUE(segments[0].mark == false);
    EXPECT_TRUE(segments[1].offset == 18);
    EXPECT_TRUE(segments[1].length == 6);
    EXPECT_TRUE(segments[1].mark == true);
    EXPECT_TRUE(segments[2].offset == 25);
    EXPECT_TRUE(segments[2].length == 5);
    EXPECT_TRUE(segments[2].mark == false);
    EXPECT_TRUE(segments[3].offset == 31);
    EXPECT_TRUE(segments[3].length == 5);
    EXPECT_TRUE(segments[3].mark == false);
    EXPECT_TRUE(segments[4].offset == 37);
    EXPECT_TRUE(segments[4].length == 5);
    EXPECT_TRUE(segments[4].mark == false);
    EXPECT_TRUE(segments[5].offset == 43);
    EXPECT_TRUE(segments[5].length == 5);
    EXPECT_TRUE(segments[5].mark == false);

    set_address_as_used(segments, 4);

    // N0CALL>APRS,CALLA,CALLB,CALLC,CALLD,CALLE*,CALLF:data
    //             ~~~~~ ~~~~~ ~~~~~ ~~~~~ ~~~~~~ ~~~~~
    //             12 17 18 23 24 29 30 35 36 42  43 48

    EXPECT_TRUE(segments[0].offset == 12);
    EXPECT_TRUE(segments[0].length == 5);
    EXPECT_TRUE(segments[0].mark == false);
    EXPECT_TRUE(segments[1].offset == 18);
    EXPECT_TRUE(segments[1].length == 5);
    EXPECT_TRUE(segments[1].mark == false);
    EXPECT_TRUE(segments[2].offset == 24);
    EXPECT_TRUE(segments[2].length == 5);
    EXPECT_TRUE(segments[2].mark == false);
    EXPECT_TRUE(segments[3].offset == 30);
    EXPECT_TRUE(segments[3].length == 5);
    EXPECT_TRUE(segments[3].mark == false);
    EXPECT_TRUE(segments[4].offset == 36);
    EXPECT_TRUE(segments[4].length == 6);
    EXPECT_TRUE(segments[4].mark == true);
    EXPECT_TRUE(segments[5].offset == 43);
    EXPECT_TRUE(segments[5].length == 5);
    EXPECT_TRUE(segments[5].mark == false);

#else
    EXPECT_TRUE(true);
#endif
}

TEST(diagnostic, push_address_unset_diagnostic)
{
    using namespace aprs::router;
    using namespace aprs::router::detail;
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING

    packet p = { "N0CALL", "APRS", { "CALLA*", "CALLB*", "CALLC", "WIDE2-2*", "CALLD*", "CALLE", "CALLF" }, "data"};

    // Initialize offsets
    std::vector<address> segments;
    try_parse_addresses(p.path, segments);
    set_addresses_offset(p, segments);

    std::vector<routing_diagnostic> diag;
    push_address_unset_diagnostic(segments, 5, true, diag);

    // N0CALL>APRS,CALLA*,CALLB*,CALLC,WIDE2-2*,CALLD*,CALLE,CALLF:data
    //             ~~~~~~
    //             12 18 - Packet address marked as 'unset'
    //
    // N0CALL>APRS,CALLA,CALLB*,CALLC,WIDE2-2*,CALLD*,CALLE,CALLF:data
    //                   ~~~~~~
    //                   18 24 - Packet address marked as 'unset'
    //
    // N0CALL>APRS,CALLA,CALLB,CALLC,WIDE2-2*,CALLD*,CALLE,CALLF:data
    //                               ~~~~~~~~
    //                               30 38 - Packet address marked as 'unset'
    //
    // N0CALL>APRS,CALLA,CALLB,CALLC,WIDE2-2,CALLD*,CALLE,CALLF:data
    //                                       ~~~~~~
    //                                       38 44 - Packet address marked as 'unset'

    EXPECT_TRUE(diag.size() == 4);

    EXPECT_TRUE(diag[0].start == 12);
    EXPECT_TRUE(diag[0].end == 18);
    EXPECT_TRUE(diag[0].index == 0);
    EXPECT_TRUE(diag[0].address == "CALLA");

    EXPECT_TRUE(diag[1].start == 18);
    EXPECT_TRUE(diag[1].end == 24);
    EXPECT_TRUE(diag[1].index == 1);
    EXPECT_TRUE(diag[1].address == "CALLB");

    EXPECT_TRUE(diag[2].start == 30);
    EXPECT_TRUE(diag[2].end == 38);
    EXPECT_TRUE(diag[2].index == 3);
    EXPECT_TRUE(diag[2].address == "WIDE2-2");

    EXPECT_TRUE(diag[3].start == 38);
    EXPECT_TRUE(diag[3].end == 44);
    EXPECT_TRUE(diag[3].index == 4);
    EXPECT_TRUE(diag[3].address == "CALLD");

#else
    EXPECT_TRUE(true);
#endif
}

TEST(diagnostic, push_address_set_diagnostic)
{
    using namespace aprs::router;
    using namespace aprs::router::detail;
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING

    packet p = { "N0CALL", "APRS", { "CALLA*", "CALLB*", "CALLC", "WIDE2-2*", "CALLD*", "CALLE", "CALLF" }, "data"};

    std::vector<address> segments;
    try_parse_addresses(p.path, segments);
    set_addresses_offset(p, segments);

    set_address_as_used(segments, 5);

    // Input:
    //
    // N0CALL>APRS,CALLA*,CALLB*,CALLC,WIDE2-2*,CALLD*,CALLE,CALLF:data
    //                  ~      ~              ~      ~
    // Output:
    //
    // N0CALL>APRS,CALLA,CALLB,CALLC,WIDE2-2,CALLD,CALLE*,CALLF:data
    //                                                  ~

    std::vector<routing_diagnostic> diag;
    push_address_set_diagnostic(segments, 5, true, diag);

    // N0CALL>APRS,CALLA,CALLB,CALLC,WIDE2-2,CALLD,CALLE*,CALLF:data
    //                                             ~~~~~
    //                                             44 49 - Packet address marked as 'set'

    EXPECT_TRUE(diag[0].start == 44);
    EXPECT_TRUE(diag[0].end == 49);
    EXPECT_TRUE(diag[0].index == 5);
    EXPECT_TRUE(diag[0].address == "CALLE");

#else
    EXPECT_TRUE(true);
#endif
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
