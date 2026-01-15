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

#include "routes.h"

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

TEST(router, try_route_packet_auto_tests)
{
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

    EXPECT_TRUE(success_count > 0);

    printf("success count: %d/%zu\n", success_count, tests.size());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}