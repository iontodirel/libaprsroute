// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 Ion Todirel                                   //
// **************************************************************** //
//
// routes.h
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

#pragma once

#include <string>
#include <vector>

#include "../aprsroute.hpp"

using namespace aprs::router;
using namespace aprs::router::detail;

// **************************************************************** //
//                                                                  //
//                                                                  //
// route_test                                                       //
//                                                                  //
//                                                                  //
// **************************************************************** //

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

// **************************************************************** //
//                                                                  //
//                                                                  //
// utility functions                                                //
//                                                                  //
//                                                                  //
// **************************************************************** //

std::string to_lower(const std::string& str);
bool try_parse_bool(const std::string& s, bool& b);
std::vector<std::string> split_comma_separated_values(const std::string& str);
void debugger_break();

template <class Allocator>
inline std::vector<std::string> to_vector_of_string(const std::vector<address, Allocator>& addresses)
{
    std::vector<std::string> result;
    for (const auto& p : addresses)
    {
        result.push_back(to_string(p));
    }
    return result;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// routing and addresses                                            //
//                                                                  //
//                                                                  //
// **************************************************************** //

bool try_parse_addresses(const std::vector<std::string>& addresses, internal_vector_t<address>& result);
internal_vector_t<address> get_router_n_N_addresses(const internal_vector_t<address>& router_addresses);
internal_vector_t<address> get_router_explicit_addresses(const internal_vector_t<address>& router_addresses);
void init_router_addresses(const packet& p, const std::vector<std::string>& path, router_settings& settings);
std::string to_string(routing_option o);

// **************************************************************** //
//                                                                  //
//                                                                  //
// tests                                                            //
//                                                                  //
//                                                                  //
// **************************************************************** //

std::vector<route_test> load_routing_tests(const std::string& test_file);
bool try_get_routing_test_set(route_test test, packet& p, router_settings& settings);
bool has_marked_tests(const std::vector<route_test>& tests);
