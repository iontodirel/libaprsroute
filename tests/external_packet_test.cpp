// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 - 2025 Ion Todirel                            //
// **************************************************************** //
//
// external_packet_test.cpp
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

#include <string>
#include <vector>
#include <cassert>

#define APRS_NAMESPACE_BEGIN namespace aprs {
#define APRS_NAMESPACE_END }

APRS_NAMESPACE_BEGIN

struct packet
{
    packet() = default;
    packet(const packet& other) = default;
    packet& operator=(const packet& other) = default;
    packet(const std::string& from, const std::string& to, const std::vector<std::string>& path, const std::string& data);
    packet(const char* packet_string);
    packet(const std::string packet_string);
    operator std::string() const;

    std::string from;
    std::string to;
    std::vector<std::string> path;
    std::string data;
};

bool operator==(const packet& lhs, const packet& rhs);
bool operator!=(const packet& lhs, const packet& rhs);
size_t hash(const packet& p);
std::string to_string(const packet& p);
bool try_decode_packet(std::string_view packet_string, packet& result);

packet::packet(const std::string& from, const std::string& to, const std::vector<std::string>& path, const std::string& data) : from(from), to(to), path(path), data(data)
{
}

packet::packet(const char* packet_string) : packet(std::string(packet_string))
{
}

packet::packet(const std::string packet_string)
{
    bool result = try_decode_packet(packet_string, *this);
    (void)result;
    assert(result);
}

packet::operator std::string() const
{
    return to_string(*this);
}

size_t hash(const struct packet& packet)
{
    size_t result = 17; // Start with a prime number
    result = result * 31 + std::hash<std::string>()(packet.from);
    result = result * 31 + std::hash<std::string>()(packet.to);
    result = result * 31 + std::hash<std::string>()(packet.data);
    return result;
}

std::string to_string(const struct packet& packet)
{
    // Does not guarantee formatting a correct packet string
    // if the input packet is invalid ex: missing path

    std::string result = packet.from + ">" + packet.to;

    if (!packet.path.empty())
    {
        for (const auto& address : packet.path)
        {
            result += "," + address;
        }
    }

    result += ":" + packet.data;

    return result;
}

bool operator==(const packet& lhs, const packet& rhs)
{
    return lhs.from == rhs.from &&
        lhs.to == rhs.to &&
        lhs.path == rhs.path &&
        lhs.data == rhs.data;
}

bool operator!=(const packet& lhs, const packet& rhs)
{
    return !(lhs == rhs);
}

bool try_decode_packet(std::string_view packet_string, packet& result)
{
    // Parse a packet: N0CALL>APRS,CALLA,CALLB*,CALLC,CALLD,CALLE,CALLF,CALLG:data
    //                 ~~~~~~ ~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~
    //                 from   to   path                                       data
    //
    // If packet string is invalid, filling of the the packet fields is not guaranteed,
    // e.g. missing data separator ":", or missig "path"

    result.path.clear();

    size_t from_position = packet_string.find('>');
    size_t colon_position = packet_string.find(':', from_position);

    if (from_position == std::string::npos || colon_position == std::string::npos)
    {
        return false;
    }

    result.from = packet_string.substr(0, from_position);

    std::string_view to_and_path = packet_string.substr(from_position + 1, colon_position - from_position - 1);

    size_t comma_position = to_and_path.find(',');

    result.to = to_and_path.substr(0, comma_position);

    if (comma_position != std::string::npos)
    {
        std::string_view path = to_and_path.substr(comma_position + 1);

        while (!path.empty())
        {
            size_t next_comma = path.find(',');

            std::string_view address = path.substr(0, next_comma);

            result.path.push_back(std::string(address));

            if (next_comma == std::string_view::npos)
            {
                break;
            }

            path.remove_prefix(next_comma + 1);
        }
    }

    result.data = packet_string.substr(colon_position + 1);

    return true;
}

APRS_NAMESPACE_END

#define APRS_ROUTER_ENABLE_PACKET_SUPPORT false
#define APRS_ROUTER_PACKET_NAMESPACE aprs
#include "../aprsroute.hpp"

#include <gtest/gtest.h>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

using namespace aprs;
using namespace aprs::router;

TEST(router, external_packet_test)
{
    router_settings digi{ "DIGI", {}, { "WIDE1" }, routing_option::none, true };
    routing_result result;

    packet p = "N0CALL>APRS,WIDE1-3:data";

    EXPECT_TRUE(try_route_packet(p, digi, result));

    EXPECT_TRUE(result.routed == true);
    EXPECT_TRUE(to_string(result.routed_packet) == "N0CALL>APRS,DIGI*,WIDE1-2:data");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}