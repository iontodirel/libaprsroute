// **************************************************************** //
// libaprsroute - APRS header only routing library                  //
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 - 2025 Ion Todirel                            //
// **************************************************************** //
//
// with_etl.cpp
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

#include <cassert>
#include <vector>
#include <algorithm>

#include <etl/vector.h>
#include <etl/string.h>

#define APRS_ROUTER_USE_PMR 0
#define APRS_ROUTER_DEFINE_CUSTOM_TYPES

namespace aprs::router::detail
{
    template<class T>
    using internal_vector_t = etl::vector<T, 8>;

    template<class T>
    using internal_string_t = etl::string<20>;
}

#include "../aprsroute.hpp"

using namespace aprs::router;
using namespace aprs::router::detail;

TEST(router, try_route_packet)
{
    aprs::router::router_settings settings{ "DIGI", {}, { "WIDE1-2", "WIDE2-3" }, aprs::router::routing_option::none, false };

    std::vector<std::string> original_packet_path{ "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };

    aprs::router::routing_state routing_state;

    std::vector<routing_diagnostic> routing_actions;

    std::vector<std::string> routed_packet_path;
    routed_packet_path.reserve(8);

    aprs::router::try_route_packet("N0CALL-10", "CALL-5", original_packet_path.begin(), original_packet_path.end(), settings, std::back_inserter(routed_packet_path), routing_state, std::back_inserter(routing_actions), nullptr);

    EXPECT_TRUE((routed_packet_path == std::vector<std::string>{ "CALLA-10", "CALLB-5", "CALLC-15", "WIDE1", "DIGI", "WIDE2*" }));
}

TEST(router, try_route_packet_with_etl_string)
{
    aprs::router::router_settings settings{ "DIGI", {}, { "WIDE1-2", "WIDE2-3" }, aprs::router::routing_option::none, false };

    std::vector<etl::string<20>> original_packet_path{ "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };

    aprs::router::routing_state routing_state;

    std::vector<routing_diagnostic> routing_actions;

    std::vector<std::string> routed_packet_path;
    routed_packet_path.reserve(8);

    aprs::router::try_route_packet("N0CALL-10", "CALL-5", original_packet_path.begin(), original_packet_path.end(), settings, std::back_inserter(routed_packet_path), routing_state, std::back_inserter(routing_actions), nullptr);

    EXPECT_TRUE((routed_packet_path == std::vector<std::string>{ "CALLA-10", "CALLB-5", "CALLC-15", "WIDE1", "DIGI", "WIDE2*" }));
}

TEST(router, try_route_packet_with_etl_vector)
{
    aprs::router::router_settings settings{ "DIGI", {}, { "WIDE1-2", "WIDE2-3" }, aprs::router::routing_option::none, true };

    etl::vector<etl::string<20>, 5> original_packet_path{ "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };

    aprs::router::routing_state routing_state;

    etl::vector<routing_diagnostic, 10> routing_actions;
    routing_actions.resize(10);

    etl::vector<std::string, 8> routed_packet_path;
    routed_packet_path.resize(8);

    auto [routed_packet_path_out_end_it, routing_actions_out_end_it, result] = aprs::router::try_route_packet("N0CALL-10", "CALL-5", original_packet_path.begin(), original_packet_path.end(), settings, routed_packet_path.begin(), routing_state, routing_actions.begin(), nullptr);

    size_t routed_size = std::distance(routed_packet_path.begin(), routed_packet_path_out_end_it);
    size_t actions_size = std::distance(routing_actions.begin(), routing_actions_out_end_it);

    EXPECT_TRUE(routed_size == 6);

    EXPECT_TRUE(actions_size == 7);

    EXPECT_TRUE(routing_actions[0].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[0].index == 4);
    EXPECT_TRUE(routing_actions[0].start == 53);
    EXPECT_TRUE(routing_actions[0].end == 60);
    EXPECT_TRUE(routing_actions[0].type == aprs::router::routing_action::decrement);
    EXPECT_TRUE(routing_actions[0].address == "WIDE2");
    EXPECT_TRUE(to_string(routing_actions[0].message_type) == "Packet address decremented");

    EXPECT_TRUE(routing_actions[1].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[1].index == 0);
    EXPECT_TRUE(routing_actions[1].start == 17);
    EXPECT_TRUE(routing_actions[1].end == 26);
    EXPECT_TRUE(routing_actions[1].type == aprs::router::routing_action::unset);
    EXPECT_TRUE(routing_actions[1].address == "CALLA-10");
    EXPECT_TRUE(to_string(routing_actions[1].message_type) == "Packet address marked as 'unset'");

    EXPECT_TRUE(routing_actions[2].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[2].index == 1);
    EXPECT_TRUE(routing_actions[2].start == 26);
    EXPECT_TRUE(routing_actions[2].end == 34);
    EXPECT_TRUE(routing_actions[2].type == aprs::router::routing_action::unset);
    EXPECT_TRUE(routing_actions[2].address == "CALLB-5");
    EXPECT_TRUE(to_string(routing_actions[2].message_type) == "Packet address marked as 'unset'");

    EXPECT_TRUE(routing_actions[3].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[3].index == 2);
    EXPECT_TRUE(routing_actions[3].start == 34);
    EXPECT_TRUE(routing_actions[3].end == 43);
    EXPECT_TRUE(routing_actions[3].type == aprs::router::routing_action::unset);
    EXPECT_TRUE(routing_actions[3].address == "CALLC-15");
    EXPECT_TRUE(to_string(routing_actions[3].message_type) == "Packet address marked as 'unset'");

    EXPECT_TRUE(routing_actions[4].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[4].index == 3);
    EXPECT_TRUE(routing_actions[4].start == 43);
    EXPECT_TRUE(routing_actions[4].end == 49);
    EXPECT_TRUE(routing_actions[4].type == aprs::router::routing_action::unset);
    EXPECT_TRUE(routing_actions[4].address == "WIDE1");
    EXPECT_TRUE(to_string(routing_actions[4].message_type) == "Packet address marked as 'unset'");

    EXPECT_TRUE(routing_actions[5].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[5].index == 4);
    EXPECT_TRUE(routing_actions[5].start == 49);
    EXPECT_TRUE(routing_actions[5].end == 54);
    EXPECT_TRUE(routing_actions[5].type == aprs::router::routing_action::set);
    EXPECT_TRUE(routing_actions[5].address == "WIDE2");
    EXPECT_TRUE(to_string(routing_actions[5].message_type) == "Packet address marked as 'set'");

    EXPECT_TRUE(routing_actions[6].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[6].index == 4);
    EXPECT_TRUE(routing_actions[6].start == 49);
    EXPECT_TRUE(routing_actions[6].end == 53);
    EXPECT_TRUE(routing_actions[6].type == aprs::router::routing_action::insert);
    EXPECT_TRUE(routing_actions[6].address == "DIGI");
    EXPECT_TRUE(to_string(routing_actions[6].message_type) == "Packet address inserted");

    EXPECT_TRUE((std::vector<std::string>(routed_packet_path.begin(), routed_packet_path_out_end_it) == std::vector<std::string>{ "CALLA-10", "CALLB-5", "CALLC-15", "WIDE1", "DIGI", "WIDE2*" }));
}

TEST(router, try_route_packet_output_iterator_stack)
{
    aprs::router::router_settings settings{ "DIGI", {}, { "WIDE1-2", "WIDE2-3" }, aprs::router::routing_option::none, true };

    char original_packet_path[5][20] = { "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };

    aprs::router::routing_state routing_state;

    routing_diagnostic routing_actions[10];

    std::string routed_packet_path[8];

    auto [routed_packet_path_out_end_it, routing_actions_out_end_it, result] = aprs::router::try_route_packet("N0CALL-10", "CALL-5", std::begin(original_packet_path), std::end(original_packet_path), settings, std::begin(routed_packet_path), routing_state, std::begin(routing_actions), nullptr);

    size_t routed_size = std::distance(std::begin(routed_packet_path), routed_packet_path_out_end_it);
    size_t actions_size = std::distance(std::begin(routing_actions), routing_actions_out_end_it);

    EXPECT_TRUE(routed_size == 6);

    EXPECT_TRUE(actions_size == 7);

    EXPECT_TRUE(routing_actions[0].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[0].index == 4);
    EXPECT_TRUE(routing_actions[0].start == 53);
    EXPECT_TRUE(routing_actions[0].end == 60);
    EXPECT_TRUE(routing_actions[0].type == aprs::router::routing_action::decrement);
    EXPECT_TRUE(routing_actions[0].address == "WIDE2");
    EXPECT_TRUE(to_string(routing_actions[0].message_type) == "Packet address decremented");

    EXPECT_TRUE(routing_actions[1].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[1].index == 0);
    EXPECT_TRUE(routing_actions[1].start == 17);
    EXPECT_TRUE(routing_actions[1].end == 26);
    EXPECT_TRUE(routing_actions[1].type == aprs::router::routing_action::unset);
    EXPECT_TRUE(routing_actions[1].address == "CALLA-10");
    EXPECT_TRUE(to_string(routing_actions[1].message_type) == "Packet address marked as 'unset'");

    EXPECT_TRUE(routing_actions[2].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[2].index == 1);
    EXPECT_TRUE(routing_actions[2].start == 26);
    EXPECT_TRUE(routing_actions[2].end == 34);
    EXPECT_TRUE(routing_actions[2].type == aprs::router::routing_action::unset);
    EXPECT_TRUE(routing_actions[2].address == "CALLB-5");
    EXPECT_TRUE(to_string(routing_actions[2].message_type) == "Packet address marked as 'unset'");

    EXPECT_TRUE(routing_actions[3].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[3].index == 2);
    EXPECT_TRUE(routing_actions[3].start == 34);
    EXPECT_TRUE(routing_actions[3].end == 43);
    EXPECT_TRUE(routing_actions[3].type == aprs::router::routing_action::unset);
    EXPECT_TRUE(routing_actions[3].address == "CALLC-15");
    EXPECT_TRUE(to_string(routing_actions[3].message_type) == "Packet address marked as 'unset'");

    EXPECT_TRUE(routing_actions[4].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[4].index == 3);
    EXPECT_TRUE(routing_actions[4].start == 43);
    EXPECT_TRUE(routing_actions[4].end == 49);
    EXPECT_TRUE(routing_actions[4].type == aprs::router::routing_action::unset);
    EXPECT_TRUE(routing_actions[4].address == "WIDE1");
    EXPECT_TRUE(to_string(routing_actions[4].message_type) == "Packet address marked as 'unset'");

    EXPECT_TRUE(routing_actions[5].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[5].index == 4);
    EXPECT_TRUE(routing_actions[5].start == 49);
    EXPECT_TRUE(routing_actions[5].end == 54);
    EXPECT_TRUE(routing_actions[5].type == aprs::router::routing_action::set);
    EXPECT_TRUE(routing_actions[5].address == "WIDE2");
    EXPECT_TRUE(to_string(routing_actions[5].message_type) == "Packet address marked as 'set'");

    EXPECT_TRUE(routing_actions[6].target == aprs::router::applies_to::path);
    EXPECT_TRUE(routing_actions[6].index == 4);
    EXPECT_TRUE(routing_actions[6].start == 49);
    EXPECT_TRUE(routing_actions[6].end == 53);
    EXPECT_TRUE(routing_actions[6].type == aprs::router::routing_action::insert);
    EXPECT_TRUE(routing_actions[6].address == "DIGI");
    EXPECT_TRUE(to_string(routing_actions[6].message_type) == "Packet address inserted");

    EXPECT_TRUE((std::vector<std::string>(std::begin(routed_packet_path), routed_packet_path_out_end_it) == std::vector<std::string>{ "CALLA-10", "CALLB-5", "CALLC-15", "WIDE1", "DIGI", "WIDE2*" }));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
