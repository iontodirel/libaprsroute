// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 - 2025 Ion Todirel                            //
// **************************************************************** //
//
// stress_test.cpp
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

#include "../aprsroute.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

// Disable the packet loop test if defined. The packet loop test is a stress test that sends a large number of packets through the router.
#ifdef APRS_ROUTE_DISABLE_PACKET_LOOP_TEST
// Intentionally left empty
#endif

using namespace aprs::router;
using namespace aprs::router::detail;

// Disable the packet loop test, as it is lengthy and not needed for regular testing.
#define APRS_ROUTE_DISABLE_PACKET_LOOP_TEST

TEST(router, try_route_packet_loop)
{
#ifndef APRS_ROUTE_DISABLE_PACKET_LOOP_TEST
    auto start = std::chrono::high_resolution_clock::now();

    // Typically about 500K packets / second (Intel Celeron N5095, 8GB RAM)

    const size_t packet_count = 1000000;

    for (int i = 0; i < packet_count; i++)
    {
        routing_result result;
        router_settings digi;

        digi.address = "DIGI";
        digi.n_N_addresses = { "WIDE4" };

        packet p = { "N0CALL", "APRS", {"CALL", "WIDE1-3", "TEMP", "WIDE2-3", "ROUTE", "WIDE3-3", "ROUTER", "WIDE4-3"}, "The quick brown fox jumps over the lazy dog." };

        try_route_packet(p, digi, result);

        EXPECT_TRUE(result.routed == false);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;

    double elapsed_ms = elapsed.count();
    double elapsed_seconds = elapsed_ms / 1000;
    double elapsed_minutes = elapsed_seconds / 60;
    double packets_per_ms = packet_count / elapsed_ms;

    std::cout << "Benchmark: " << elapsed_ms << " ms" << std::endl;
    std::cout << "Benchmark: " << std::fixed << std::setprecision(2) << elapsed_seconds << " seconds" << std::endl;
    std::cout << "Benchmark: " << std::fixed << std::setprecision(2) << elapsed_minutes << " minutes" << std::endl;
    std::cout << "Benchmark: " << packets_per_ms << " packets / ms" << std::endl;
#else
    EXPECT_TRUE(true);
#endif
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}