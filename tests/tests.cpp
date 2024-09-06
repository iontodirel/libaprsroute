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

#include "../aprsroute.hpp"

#if defined(IS_LINUX_MAC) && !defined(APRS_ROUTE_DISABLE_AUTO_TESTING)
#include <signal.h>
#endif

using namespace aprs::router;
using namespace aprs::router::detail;

#define APRS_ROUTE_DISABLE_PACKET_LOOP_TEST

routing_result test_packet_routing_iteration(const packet& p, router_settings digi, std::vector<std::string> addresses,  std::vector<size_t> digipeated_indices, int count);

TEST(segment, to_string)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    segment s;
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

TEST(segment, try_parse_segment)
{
#ifndef APRS_ROUTE_ENABLE_ONLY_AUTO_TESTING
    segment s;
    std::string path;

    // -------------------------------------------------------------
    // Wide segment with both digits present
    // -------------------------------------------------------------

    path = "WIDE7-5";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 7);
    EXPECT_TRUE(s.N == 5);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.type == segment_type::wide);

    // -------------------------------------------------------------
    // Wide segment with a marker (*) but no digits after dash
    // -------------------------------------------------------------

    path = "WIDE*";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == true);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.type == segment_type::wide);

    // -------------------------------------------------------------
    // Wide segment with only leading digits
    // -------------------------------------------------------------

    path = "WIDE5";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 5);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.type == segment_type::wide);

    // -------------------------------------------------------------
    // Wide segment with leading digits and a mark
    // -------------------------------------------------------------

    path = "WIDE5*";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == true);
    EXPECT_TRUE(s.n == 5);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.type == segment_type::wide);

    // -------------------------------------------------------------
    // Q construct segment
    // -------------------------------------------------------------

    path = "qAR";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "qAR");
    EXPECT_TRUE(s.q == q_construct::qAR);
    EXPECT_TRUE(s.type == segment_type::q);

    // -------------------------------------------------------------
    // Wide segment with both digits and a mark
    // -------------------------------------------------------------

    path = "WIDE7-7*";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == true);
    EXPECT_TRUE(s.n == 7);
    EXPECT_TRUE(s.N == 7);
    EXPECT_TRUE(s.text == "WIDE");
    EXPECT_TRUE(s.type == segment_type::wide);

    // -------------------------------------------------------------
    // Other segment with text and digits
    // -------------------------------------------------------------

    path = "W7ION-10*";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == true);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "W7ION-10");
    EXPECT_TRUE(s.type == segment_type::other);

    // -------------------------------------------------------------
    // Other segment without a mark (*) and with no n-N digits
    // -------------------------------------------------------------

    path = "N0CALL";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "N0CALL");
    EXPECT_TRUE(s.type == segment_type::other);

    // -------------------------------------------------------------
    // Additional test cases to cover edge cases
    // -------------------------------------------------------------

    path = "WIDE-1"; // Should not be valid, missing leading digits
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE-1"); // Entire text should be preserved if parsing n-N fails
    EXPECT_TRUE(s.type == segment_type::other);

    path = "*WIDE"; // Leading mark with no valid segment
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.mark == false);
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "*WIDE");
    EXPECT_TRUE(s.type == segment_type::other);

    path = "WIDE0-4"; // 0 is not valid for the leading digit (1-7 is the valid range)
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE0-4");
    EXPECT_TRUE(s.type == segment_type::other);

    path = "WIDE8-4"; // 8 is not valid for the leading digit (1-7 is the valid range)
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE8-4");
    EXPECT_TRUE(s.type == segment_type::other);

    path = "WIDE2-0";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE2-0");
    EXPECT_TRUE(s.type == segment_type::other);

    path = "WIDE4-2-0";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE4-2-0");
    EXPECT_TRUE(s.type == segment_type::other);

    path = "WIDE4-10";
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.n == 0);
    EXPECT_TRUE(s.N == 0);
    EXPECT_TRUE(s.text == "WIDE4-10");
    EXPECT_TRUE(s.type == segment_type::other);

    path = "WIDE14-1"; // Not really valid, but we don't care that it parses to 4-1
    EXPECT_TRUE(try_parse_segment(path, s));
    EXPECT_TRUE(s.n == 4);
    EXPECT_TRUE(s.N == 1);
    EXPECT_TRUE(s.text == "WIDE1");
    EXPECT_TRUE(s.type == segment_type::other);
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

    packet p = { "N0CALL", "APRS", { "WIDE1-3" }, "data"}; // N0CALL>APRS,WIDE1-3:data

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
            try_route_packet(p, settings, result);
        }
        EXPECT_TRUE(to_string(result.original_packet) == test.original_packet);
    }

    if (!test.routed)
    {
        EXPECT_TRUE(to_string(result.original_packet) == to_string(result.routed_packet));
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
        if (try_get_routing_test_set(test, p, settings))
        {
            run_test(test, p, settings);
        }
    }
#else
    EXPECT_TRUE(true);
#endif
}

TEST(segment, try_parse_callsign)
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

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
