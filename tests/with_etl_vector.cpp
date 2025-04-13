// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 - 2025 Ion Todirel                            //
// **************************************************************** //
//
// with_etl_vector.cpp
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

#include <cassert>
#include <vector>
#include <etl/vector.h>

#define APRS_ROUTER_USE_PMR 0
#define APRS_ROUTER_DEFINE_CUSTOM_TYPES

namespace aprs::router::detail
{
    template<class T>
    using internal_vector_t = etl::vector<T, 8>;
}

#include "../aprsroute.hpp"

using namespace aprs::router;
using namespace aprs::router::detail;

int main()
{
    aprs::router::router_settings settings{ "DIGI", {}, { "WIDE1-2", "WIDE2-3" }, aprs::router::routing_option::none, false };

    std::vector<std::string> original_packet_path{ "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };

    aprs::router::routing_state routing_state;

    std::vector<routing_diagnostic> routing_actions;

    std::vector<std::string> routed_packet_path;
    routed_packet_path.reserve(8);

    aprs::router::try_route_packet("N0CALL-10", "CALL-5", original_packet_path.begin(), original_packet_path.end(), settings, std::back_inserter(routed_packet_path), routing_state, std::back_inserter(routing_actions), nullptr);

    assert((routed_packet_path == std::vector<std::string>{ "CALLA-10", "CALLB-5", "CALLC-15", "WIDE1", "DIGI", "WIDE2*" }));

    return 0;
}
