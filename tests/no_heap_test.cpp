// **************************************************************** //
// libaprsroute - APRS header only routing library                  //
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 Ion Todirel                                   //
// **************************************************************** //
//
// no_heap_test.cpp
//
// MIT License
//
// Copyright (c) 2026 Ion Todirel
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

#include <array>
#include <cstdlib>
#include <new>
#include <string_view>

#include <gtest/gtest.h>

#include "../aprsroute.hpp"

bool tracking_enabled = false;
size_t allocation_count = 0;
size_t allocation_bytes = 0;

void* operator new(size_t requested_bytes)
{
    void* allocated_pointer = std::malloc(requested_bytes ? requested_bytes : 1);
    if (!allocated_pointer) throw std::bad_alloc();
    if (tracking_enabled)
    {
        allocation_count++;
        allocation_bytes += requested_bytes;
    }
    return allocated_pointer;
}

void* operator new[](size_t requested_bytes)
{
    return ::operator new(requested_bytes);
}

void operator delete(void* allocated_pointer) noexcept { std::free(allocated_pointer); }
void operator delete[](void* allocated_pointer) noexcept { std::free(allocated_pointer); }
void operator delete(void* allocated_pointer, size_t) noexcept { std::free(allocated_pointer); }
void operator delete[](void* allocated_pointer, size_t) noexcept { std::free(allocated_pointer); }

TEST(no_heap, try_route_packet_low_level_overload_one_million_packets)
{
    constexpr size_t packet_count = 1'000'000;

    const std::string_view packet_from = "N0CALL-10";
    const std::string_view packet_to   = "CALL-5";
    const std::string_view router_address = "DIGI";

    const std::array<std::string_view, 5> packet_path{ "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };
    const std::array<std::string_view, 6> expected_routed_path{ "CALLA-10", "CALLB-5", "CALLC-15", "WIDE1", "DIGI", "WIDE2*" };
    const std::array<std::string_view, 0> explicit_addresses{};
    const std::array<std::string_view, 2> n_N_addresses{ "WIDE1-1", "WIDE2-1" };

    std::array<std::array<char, 10>, 8> routed_packet_path{};
    std::array<size_t, 8> routed_packet_path_address_sizes{};

    aprs::router::routing_state routing_state;
    aprs::router::route_state   reusable_route_state;

    // Track only routing calls; route_state ctor allocates a 16-byte
    // _Container_proxy under MSVC iterator-debug (Debug builds).
    allocation_count = 0;
    allocation_bytes = 0;
    tracking_enabled = true;

    for (size_t iteration = 0; iteration < packet_count; ++iteration)
    {
        auto [routed_packet_path_end, routed_packet_path_address_sizes_end, routing_succeeded] = aprs::router::try_route_packet(
            packet_from, packet_to,
            packet_path.begin(), packet_path.end(),
            router_address,
            explicit_addresses.begin(), explicit_addresses.end(),
            n_N_addresses.begin(), n_N_addresses.end(),
            aprs::router::routing_option::none,
            routed_packet_path.begin(),
            routed_packet_path_address_sizes.begin(),
            routing_state, reusable_route_state);

        (void)routed_packet_path_end;
        (void)routed_packet_path_address_sizes_end;
        (void)routing_succeeded;
    }

    tracking_enabled = false;

    EXPECT_EQ(allocation_count, 0u)
        << "low-level try_route_packet performed " << allocation_count
        << " heap allocation(s) totaling " << allocation_bytes << " bytes across "
        << packet_count << " routing calls";

    // Spot-check the final routed path to ensure routing actually executed.
    for (size_t address_index = 0; address_index < expected_routed_path.size(); ++address_index)
    {
        EXPECT_EQ(std::string_view(routed_packet_path[address_index].data(), routed_packet_path_address_sizes[address_index]),
                  expected_routed_path[address_index]);
    }
}

TEST(no_heap, init_router_then_no_init_overload_one_million_packets)
{
    constexpr size_t packet_count = 1'000'000;

    const std::string_view packet_from = "N0CALL-10";
    const std::string_view packet_to   = "CALL-5";
    const std::string_view router_address = "DIGI";

    const std::array<std::string_view, 5> packet_path{ "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };
    const std::array<std::string_view, 6> expected_routed_path{ "CALLA-10", "CALLB-5", "CALLC-15", "WIDE1", "DIGI", "WIDE2*" };
    const std::array<std::string_view, 0> explicit_addresses{};
    const std::array<std::string_view, 2> n_N_addresses{ "WIDE1-1", "WIDE2-1" };

    std::array<std::array<char, 10>, 8> routed_packet_path{};
    std::array<size_t, 8> routed_packet_path_address_sizes{};

    aprs::router::routing_state routing_state;
    aprs::router::route_state   reusable_route_state;

    aprs::router::init_router(router_address, explicit_addresses.begin(), explicit_addresses.end(), n_N_addresses.begin(), n_N_addresses.end(), aprs::router::routing_option::none, false, reusable_route_state);

    // Track only routing calls; route_state ctor allocates a 16-byte
    // _Container_proxy under MSVC iterator-debug (Debug builds).
    allocation_count = 0;
    allocation_bytes = 0;
    tracking_enabled = true;

    for (size_t iteration = 0; iteration < packet_count; ++iteration)
    {
        auto [routed_packet_path_end, routed_packet_path_address_sizes_end, routing_actions_end, routing_succeeded] = aprs::router::try_route_packet(
            packet_from, packet_to,
            packet_path.begin(), packet_path.end(),
            routed_packet_path.begin(),
            routed_packet_path_address_sizes.begin(),
            aprs::router::detail::discard_output_iterator{},
            routing_state, reusable_route_state);

        (void)routed_packet_path_end;
        (void)routed_packet_path_address_sizes_end;
        (void)routing_actions_end;
        (void)routing_succeeded;
    }

    tracking_enabled = false;

    EXPECT_EQ(allocation_count, 0u)
        << "init_router + no-init try_route_packet performed " << allocation_count
        << " heap allocation(s) totaling " << allocation_bytes << " bytes across "
        << packet_count << " routing calls";

    for (size_t address_index = 0; address_index < expected_routed_path.size(); ++address_index)
    {
        EXPECT_EQ(std::string_view(routed_packet_path[address_index].data(), routed_packet_path_address_sizes[address_index]),
                  expected_routed_path[address_index]);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
