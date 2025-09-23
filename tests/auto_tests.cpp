// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 Ion Todirel                                   //
// **************************************************************** //
//
// auto_tests.cpp
//
// MIT License
//
// Copyright (c) 2025 Ion Todirel
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

// Disable the auto testing if defined. This will run the regular tests only.
#ifdef APRS_ROUTE_DISABLE_AUTO_TESTING
// Intentionally left empty
#endif // APRS_ROUTE_DISABLE_AUTO_TESTING

using namespace aprs::router;
using namespace aprs::router::detail;

// **************************************************************** //
//                                                                  //
//                                                                  //
// utility functions                                                //
//                                                                  //
//                                                                  //
// **************************************************************** //

bool try_parse_addresses(const std::vector<std::string>& addresses, internal_vector_t<address>& result)
{
    result.clear();
    size_t index = 0;
    for (const auto& a : addresses)
    {
        address s;
        try_parse_address(a, s);
        s.index = index;
        index++;
        result.push_back(s);
    }
    return true;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// auto tests utility types and functions                           //
//                                                                  //
//                                                                  //
// **************************************************************** //

#ifndef APRS_ROUTE_DISABLE_AUTO_TESTING

struct route_test
{
    bool mark = false;
    std::string id;
    bool debug = false;
    std::string comment;
    std::string address;
    std::string path;
    std::string explicit_addresses;
    std::string n_N_addresses;
    std::string original_packet;
    std::string routed_packet;
    bool routed = false;
    std::string options;
};

std::string to_lower(const std::string& str);
bool try_parse_bool(const std::string& s, bool& b);
std::vector<route_test> load_routing_tests(const std::string& test_file);
std::vector<std::string> split_comma_separated_values(const std::string& str);
std::string to_string(routing_option o);
internal_vector_t<address> get_router_n_N_addresses(const internal_vector_t<address>& router_addresses);
internal_vector_t<address> get_router_explicit_addresses(const internal_vector_t<address>& router_addresses);
template <class Allocator> std::vector<std::string> to_vector_of_string(const std::vector<address, Allocator>& addresses);
void init_router_addresses(const packet& p, const std::vector<std::string>& path, router_settings& settings);
bool try_get_routing_test_set(route_test test, packet& p, router_settings& settings);
void debugger_break();
bool run_test(const route_test& test, const packet& p, const router_settings& settings);
void test_diagnostics_reconstruct_packet_by_index(const route_test& test, const routing_result& r);
void test_diagnostics_reconstruct_packet_by_start_end(const route_test& test, const routing_result& r);
bool has_marked_tests(const std::vector<route_test>& tests);

std::string to_lower(const std::string& str)
{
    std::locale loc;
    std::string s;
    s.resize(str.size());
    for (size_t i = 0; i < str.size(); i++)
        s[i] = std::tolower(str[i], loc);
    return s;
}

bool try_parse_bool(const std::string& s, bool& b)
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

                if (route.contains("explicit_addresses"))
                {
                    test.explicit_addresses = route["explicit_addresses"].get<std::string>();
                }

                if (route.contains("n_N_addresses"))
                {
                    test.n_N_addresses = route["n_N_addresses"].get<std::string>();
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

std::string to_string(routing_option o)
{
    std::string result;

    if (enum_has_flag(o, routing_option::none))
        result += "none,";
    if (enum_has_flag(o, routing_option::preempt_drop))
        result += "preempt_drop,";
    if (enum_has_flag(o, routing_option::preempt_front))
        result += "preempt_front,";
    if (enum_has_flag(o, routing_option::trap_limit_exceeding_n_N_address))
        result += "trap_limit_exceeding_n_N_address,";
    if (enum_has_flag(o, routing_option::substitute_complete_n_N_address))
        result += "substitute_complete_n_N_address,";

    if (!result.empty())
        result.pop_back(); // Remove the trailing ','

    return result.empty() ? "unknown" : result;
}

internal_vector_t<address> get_router_n_N_addresses(const internal_vector_t<address>& router_addresses)
{
    internal_vector_t<address> result;
    for (const auto& p : router_addresses)
    {
        if (p.n > 0)
        {
            result.push_back(p);
        }
    }
    return result;
}

internal_vector_t<address> get_router_explicit_addresses(const internal_vector_t<address>& router_addresses)
{
    internal_vector_t<address> result;
    for (const auto& p : router_addresses)
    {
        if (p.n == 0)
        {
            result.push_back(p);
        }
    }
    return result;
}

template <class Allocator>
std::vector<std::string> to_vector_of_string(const std::vector<address, Allocator>& addresses)
{
    std::vector<std::string> result;
    for (const auto& p : addresses)
    {
        result.push_back(to_string(p));
    }
    return result;
}

void init_router_addresses(const packet& p, const std::vector<std::string>& path, router_settings& settings)
{
    if (path.empty())
    {
        return;
    }

    internal_vector_t<address> router_addresses;
    try_parse_addresses(path, router_addresses);

    route_state state;
    state.router_n_N_addresses = get_router_n_N_addresses(router_addresses);
    state.router_explicit_addresses = get_router_explicit_addresses(router_addresses);
    state.packet_from_address = p.from;
    state.packet_to_address = p.to;
    state.packet_path.assign(p.path.begin(), p.path.end());
    state.settings = settings;
    init_addresses(state);

    settings.explicit_addresses = to_vector_of_string(state.router_explicit_addresses);
    settings.n_N_addresses = to_vector_of_string(state.router_n_N_addresses);
}

bool try_get_routing_test_set(route_test test, packet& p, router_settings& settings)
{
    try_decode_packet(test.original_packet, p);

    settings.address = test.address;
    settings.explicit_addresses = {};
    settings.n_N_addresses = {};

    std::vector<std::string> path = split_comma_separated_values(test.path);
    std::vector<std::string> explicit_addresses = split_comma_separated_values(test.explicit_addresses);
    std::vector<std::string> n_N_addresses = split_comma_separated_values(test.n_N_addresses);

    init_router_addresses(p, path, settings);

    settings.explicit_addresses.insert(settings.explicit_addresses.end(), explicit_addresses.begin(), explicit_addresses.end());
    settings.n_N_addresses.insert(settings.n_N_addresses.end(), n_N_addresses.begin(), n_N_addresses.end());

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

bool run_test(const route_test& test, const packet& p, const router_settings& settings)
{
    bool routed_packet_result = false;

    routing_result result;

    bool result_bool = try_route_packet(p, settings, result);

    if (result_bool != test.routed)
    {
        printf("test %s failed\n", test.id.c_str());
    }

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
        routed_packet_result = to_string(result.routed_packet) == test.routed_packet;
        EXPECT_TRUE(routed_packet_result);
        if (!routed_packet_result || test.debug)
        {
            if (!routed_packet_result)
            {
                printf("test %s failed\n", test.id.c_str());
            }

            printf("settings address: %s\n", settings.address.c_str());
            printf("settings path: ");
            for (const auto& path : settings.explicit_addresses)
            {
                printf("%s ", path.c_str());
            }
            for (const auto& path : settings.n_N_addresses)
            {
                printf("%s ", path.c_str());
            }
            printf("\nsettings options: %s\n", to_string(settings.options).c_str());
            printf("original packet: %s\n", test.original_packet.c_str());
            printf("expected packet: %s\n", test.routed_packet.c_str());
            printf("  actual packet: %s\n", to_string(result.routed_packet).c_str());

            if (test.debug)
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

    if (test.debug)
    {
        printf("%s", diag_string.c_str());
    }

    test_diagnostics_reconstruct_packet_by_index(test, result);
    test_diagnostics_reconstruct_packet_by_start_end(test, result);

    return routed_packet_result;
}

void test_diagnostics_reconstruct_packet_by_index(const route_test& test, const routing_result& result)
{
    if (result.state != routing_state::routed)
    {
        return;
    }

    packet routed_packet;

    EXPECT_TRUE(try_route_packet_by_index(result, routed_packet));

    bool result_bool = result.routed_packet == routed_packet;

    EXPECT_TRUE(result_bool);

    if (!result_bool)
    {
        // handy breakpoint location
        printf("test %s failed\n", test.id.c_str());
    }
}

void test_diagnostics_reconstruct_packet_by_start_end(const route_test& test, const routing_result& result)
{
    if (result.state != routing_state::routed)
    {
        return;
    }

    packet routed_packet;

    EXPECT_TRUE(try_route_packet_by_start_end(result, routed_packet));

    bool result_bool = result.routed_packet == routed_packet;

    EXPECT_TRUE(result_bool);

    if (!result_bool)
    {
        // handy breakpoint location
        printf("test %s failed\n", test.id.c_str());
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

#endif // APRS_ROUTE_DISABLE_AUTO_TESTING

// **************************************************************** //
//                                                                  //
//                                                                  //
// auto tests                                                       //
//                                                                  //
//                                                                  //
// **************************************************************** //

TEST(router, try_route_packet_auto_tests)
{
#ifndef APRS_ROUTE_DISABLE_AUTO_TESTING
    std::vector<route_test> tests = load_routing_tests(INPUT_TEST_FILE);
    EXPECT_TRUE(tests.empty() == false);
    printf("loaded %zu tests\n", tests.size());
    bool has_marked_test = has_marked_tests(tests);
    int success_count = 0;
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
            bool result = run_test(test, p, settings);
            if (result)
            {
                success_count++;
            }
        }
    }
    printf("success count: %d/%zu\n", success_count, tests.size());
#else
    EXPECT_TRUE(true);
#endif
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}