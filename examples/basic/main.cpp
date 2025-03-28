// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 - 2025 Ion Todirel                            //
// **************************************************************** //
//
// main.cpp
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

#include "external/aprsroute.hpp"

#include <cstdio>

using namespace aprs::router;

int main()
{
    // Example of using the APRS routing library to route a packet
    //
    // Router is configured to route packets with the following settings:
    //
    // Router address: DIGI
    // Router explicit address: none
    // Router n_N addresses: WIDE1
    // Routing options: none
    // Enabled dignostics: true
    //
    // Packet: N0CALL>APRS,WIDE1-3:data
    // Routed packet: N0CALL>APRS,DIGI*,WIDE1-2:data

    router_settings digi{ "DIGI", {}, { "WIDE1" }, routing_option::none, true };
    routing_result result;

    packet p = "N0CALL>APRS,WIDE1-3:data";

    try_route_packet(p, digi, result);

    printf("%s\n", to_string(result).c_str());

    return 0;
}