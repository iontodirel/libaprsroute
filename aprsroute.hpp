// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 Ion Todirel                                   //
// **************************************************************** //
//
// aprsroute.hpp
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
//
// Parts of this library uses code from libaprs: https://github.com/iontodirel/libaprs
// Copyright (c) 2023 Ion Todirel

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <span>
#include <tuple>
#include <cassert>
#include <optional>

#ifndef APRS_ROUTER_NAMESPACE_BEGIN
#define APRS_ROUTER_NAMESPACE_BEGIN namespace aprs::router {
#endif
#ifndef APRS_ROUTER_NAMESPACE_END
#define APRS_ROUTER_NAMESPACE_END }
#endif
#ifndef APRS_ROUTER_INLINE
#define APRS_ROUTER_INLINE inline
#endif
#ifndef APRS_ROUTER_APRS_DETAIL_NAMESPACE
#define APRS_ROUTER_APRS_DETAIL_NAMESPACE detail
#endif
#ifndef APRS_ROUTER_DETAIL_NAMESPACE_BEGIN
#define APRS_ROUTER_DETAIL_NAMESPACE_BEGIN namespace APRS_ROUTER_APRS_DETAIL_NAMESPACE {
#endif
#ifndef APRS_ROUTER_APRS_DETAIL_NS_REF
#define APRS_ROUTER_APRS_DETAIL_NS_REF APRS_ROUTER_APRS_DETAIL_NAMESPACE :: 
#endif
#ifndef APRS_ROUTER_DETAIL_NAMESPACE_END
#define APRS_ROUTER_DETAIL_NAMESPACE_END }
#endif
#ifdef APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY
// Intentionally left empty
#endif

// **************************************************************** //
//                                                                  //
//                                                                  //
// PUBLIC DECLARATIONS                                              //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_NAMESPACE_BEGIN

struct packet
{
    std::string from;
    std::string to;
    std::vector<std::string> path;
    std::string data;
};

enum class routing_option : int
{
    none = 0,
    route_self = 1,               // Enable routing packets originating from ourselves.
    preempt_front = 2,            // Preemptively move our address to the front of the route.
    preempt_truncate = 4,         // Preemptively move our address behind the last used address and erase all other addresses.
    substitute_complete_hops = 8, // Replace an CALLn-N address with our address when N decrements to 0.
    trap_excessive_hops = 16,     // Replace a harmful path with our callsign to prevent network issues (e.g., WIDE7-7).
    reject_excessive_hops = 32,   // Reject the packet if the paths has excessive hops (e.g., PATH7-7).
    strict = 64
};

struct router_settings
{
    std::string address;
    std::vector<std::string> path = { "WIDE1-2", "WIDE2-2", "TRACE1-2", "TRACE2-2", "WIDE", "RELAY", "TRACE" };
    routing_option options = routing_option::none;
    bool enable_diagnostics = true;
};

enum class routing_state
{
    routed,
    not_routed,
    already_routed,
    no_matching_aliases,
    cannot_route_self
};

enum class routing_action
{
    insert,
    remove,
    update,
    error,
    warn,
    message
};

struct routing_diagnostic
{
    int start;
    int end;
    routing_action type;
    std::string message;
};

struct routing_result
{
    bool routed = false;
    bool success = false;
    packet original_packet;
    packet routed_packet;
    routing_state state;
    std::vector<routing_diagnostic> actions;
};

struct packet_trace
{
    packet routed_packet;
    int trace_index;
    int route_index;
    std::string router_address;
    std::string message;
};

APRS_ROUTER_NAMESPACE_END

// **************************************************************** //
//                                                                  //
//                                                                  //
// PUBLIC FORWARD DECLARATIONS                                      //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_NAMESPACE_BEGIN

APRS_ROUTER_INLINE routing_option operator|(routing_option lhs, routing_option rhs);
APRS_ROUTER_INLINE bool try_parse_routing_option(std::string_view str, routing_option& result);
APRS_ROUTER_INLINE bool enum_has_flag(routing_option value, routing_option flag);
APRS_ROUTER_INLINE size_t hash(const packet& p);
APRS_ROUTER_INLINE std::string to_string(const packet& p);
APRS_ROUTER_INLINE bool try_decode_packet(std::string_view packet_string, packet& result);
APRS_ROUTER_INLINE bool try_route_packet(const struct packet& packet, const router_settings& settings, routing_result& result);
APRS_ROUTER_INLINE bool try_trace_packet(const std::vector<packet>& packets, std::vector<packet_trace>& traces);

APRS_ROUTER_NAMESPACE_END

// **************************************************************** //
//                                                                  //
//                                                                  //
// PRIVATE DECLARATIONS                                             //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_NAMESPACE_BEGIN

APRS_ROUTER_DETAIL_NAMESPACE_BEGIN

enum class q_construct
{
    none,
    qAC,
    qAX,
    qAU,
    qAo,
    qAO,
    qAS,
    qAr,
    qAR,
    qAZ,
    qAI
};

enum class segment_type
{
    other,
    wide,
    trace,
    relay,
    temp,
    tcpxx,
    tcpip,
    nogate,
    rfonly,
    igatecall,
    q,
    opntrk,
    opntrc
};

struct segment
{
    std::string text;
    int n = 0;
    int N = 0;
    bool mark = false;
    segment_type type = segment_type::other;
    q_construct q = q_construct::none;
    size_t index = 0;
};

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END

// **************************************************************** //
//                                                                  //
//                                                                  //
// PRIVATE FORWARD DECLARATIONS                                     //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_NAMESPACE_BEGIN

APRS_ROUTER_DETAIL_NAMESPACE_BEGIN

APRS_ROUTER_INLINE std::string to_string(const segment& p);
APRS_ROUTER_INLINE q_construct parse_q_construct(const std::string& input);
APRS_ROUTER_INLINE segment_type parse_segment_type(const std::string& text);
APRS_ROUTER_INLINE bool try_parse_segment(std::string_view path, segment& s);
APRS_ROUTER_INLINE bool try_parse_segments(const std::vector<std::string>& addresses, std::vector<segment>& result);
APRS_ROUTER_INLINE bool try_parse_callsign(std::string address, std::string& callsign, int& ssid);
APRS_ROUTER_INLINE bool try_parse_callsign_with_used_flag(std::string address, std::string& callsign, int& ssid);
APRS_ROUTER_INLINE std::vector<segment> get_router_n_N_addresses(const std::vector<segment>& router_addresses);
APRS_ROUTER_INLINE std::vector<segment> get_router_generic_addresses(const std::vector<segment>& router_addresses);
APRS_ROUTER_INLINE bool is_packet_valid(const struct packet& packet, const std::vector<segment>& packet_addresses, routing_option options);
APRS_ROUTER_INLINE bool is_packet_from_us(const struct packet& packet, std::string_view router_address);
APRS_ROUTER_INLINE bool is_packet_sent_to_us(const struct packet& packet, std::string_view router_address);
APRS_ROUTER_INLINE bool has_packet_routing_ended(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index);
APRS_ROUTER_INLINE bool has_packet_been_routed_by_us(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, std::string_view router_address);
APRS_ROUTER_INLINE std::vector<segment> parse_packet_addresses(const struct packet& packet);
APRS_ROUTER_INLINE void init_routing_result(const struct packet& packet, routing_result& result);
APRS_ROUTER_INLINE bool adresses_contains_address(const std::vector<segment>& packet_addresses, const segment& address);
APRS_ROUTER_INLINE std::optional<std::pair<size_t, size_t>> find_first_available_n_N_address_index(const std::vector<segment>& packet_addresses, const std::vector<segment>& router_addresses, routing_option options);
APRS_ROUTER_INLINE std::optional<size_t> find_last_used_address_index(const std::vector<segment>& packet_addresses);
APRS_ROUTER_INLINE std::optional<size_t> find_address_index(const std::vector<segment>& packet_addresses, size_t offset, std::string_view router_address, const std::vector<segment>& router_addresses);
APRS_ROUTER_INLINE bool create_routing_result(const packet& p, const std::vector<segment>& packet_addresses, routing_result& result);
APRS_ROUTER_INLINE bool create_routing_result(routing_state state, routing_result& result);
APRS_ROUTER_INLINE void unset_all_used_addresses(std::vector<segment>& packet_addresses, size_t offset, size_t count);
APRS_ROUTER_INLINE void set_address_as_used(std::vector<segment>& packet_addresses, size_t index);
APRS_ROUTER_INLINE void set_address_as_used(std::vector<segment>& packet_addresses, segment& address);
APRS_ROUTER_INLINE std::optional<size_t> find_unused_address_index(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, std::string_view router_address, const std::vector<segment>& router_generic_addresses);
APRS_ROUTER_INLINE void update_addresses_index(std::vector<segment>& addresses);
APRS_ROUTER_INLINE bool trap_n_N_route(std::vector<segment>& packet_addresses, size_t packet_n_N_address_index, const std::vector<segment>& router_addresses, size_t router_n_N_address_index, std::string_view router_address, routing_option options);
APRS_ROUTER_INLINE bool insert_n_N_route(std::vector<segment>& packet_addresses, size_t& index, std::string_view router_address, routing_option options);
APRS_ROUTER_INLINE bool insert_generic_route(std::vector<segment>& packet_addresses, size_t index, std::string_view router_address);
APRS_ROUTER_INLINE bool try_preempt_explicit_route(std::vector<segment>& packet_addresses, size_t router_address_index, std::optional<size_t> maybe_last_used_address_index, routing_option options);
APRS_ROUTER_INLINE bool is_explicit_routing(bool is_routing_self, std::optional<size_t> maybe_router_address_index, routing_option options);
APRS_ROUTER_INLINE bool try_explicit_route(std::vector<segment>& packet_addresses, std::string_view router_address, std::optional<size_t> maybe_last_used_address_index, std::optional<size_t> maybe_router_address_index, routing_option options);
APRS_ROUTER_INLINE bool try_n_N_route(std::vector<segment>& packet_addresses, const std::vector<segment>& router_n_N_addresses, const std::string& router_address, routing_option options);
APRS_ROUTER_INLINE bool move_address_to_position(std::vector<segment>& packet_addresses, size_t from_index, size_t to_index);
APRS_ROUTER_INLINE bool truncate_address_range(std::vector<segment>& packet_addresses, size_t from_index, size_t to_index);
APRS_ROUTER_INLINE void decrement_address_n_N(std::vector<segment>& packet_addresses, size_t index);
APRS_ROUTER_INLINE void decrement_address_n_N(segment& s);
APRS_ROUTER_INLINE void remove_completed_n_N_address(std::vector<segment>& packet_addresses, std::string_view router_address, size_t index);

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END

// **************************************************************** //
//                                                                  //
//                                                                  //
// PUBLIC DEFINITIONS                                               //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_NAMESPACE_BEGIN

#ifndef APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY

APRS_ROUTER_INLINE routing_option operator|(routing_option lhs, routing_option rhs)
{
    return (routing_option)((int)lhs | (int)rhs);
}

APRS_ROUTER_INLINE bool try_parse_routing_option(std::string_view str, routing_option& result)
{
    if (str == "none")
    {
        result = routing_option::none;
    }
    else if (str == "route_self")
    {
        result = routing_option::route_self;
    }
    else if (str == "preempt_front")
    {
        result = routing_option::preempt_front;
    }
    else if (str == "preempt_truncate")
    {
        result = routing_option::preempt_truncate;
    }
    else if (str == "substitute_complete_hops")
    {
        result = routing_option::substitute_complete_hops;
    }
    else if (str == "trap_excessive_hops")
    {
        result = routing_option::trap_excessive_hops;
    }
    else if (str == "reject_excessive_hops")
    {
        result = routing_option::reject_excessive_hops;
    }
    else if (str == "strict")
    {
        result = routing_option::strict;
    }
    else
    {
        return false;
    }

    return true;
}

APRS_ROUTER_INLINE bool enum_has_flag(routing_option value, routing_option flag)
{
    return ((int)value & (int)flag) != 0;
}

APRS_ROUTER_INLINE size_t hash(const packet& p)
{
    size_t result = 17; // Start with a prime number
    result = result * 31 + std::hash<std::string>()(p.from);
    result = result * 31 + std::hash<std::string>()(p.to);
    result = result * 31 + std::hash<std::string>()(p.data);
    return result;
}

APRS_ROUTER_INLINE std::string to_string(const packet& p)
{
    std::string result = p.from + ">" + p.to;

    if (!p.path.empty())
    {
        result += ",";
        for (size_t i = 0; i < p.path.size(); ++i)
        {
            result += p.path[i];
            if (i < p.path.size() - 1) // not the last one
            {
                result += ",";
            }
        }
    }

    result += ":" + p.data;

    return result;
}

APRS_ROUTER_INLINE bool try_decode_packet(std::string_view packet_string, packet& result)
{
    result.path.clear();

    size_t from_pos = packet_string.find('>');
    if (from_pos == std::string::npos)
    {
        return false;
    }

    result.from = packet_string.substr(0, from_pos);

    size_t colon_pos = packet_string.find(':', from_pos);
    if (colon_pos == std::string::npos)
    {
        return false;
    }

    std::string_view to_and_path = packet_string.substr(from_pos + 1, colon_pos - from_pos - 1);
    size_t comma_pos = to_and_path.find(',');
    if (comma_pos == std::string::npos)
    {
        result.to = to_and_path;
    }
    else
    {
        result.to = to_and_path.substr(0, comma_pos);

        // Extract the 'path' field
        std::string_view path_str = to_and_path.substr(comma_pos + 1);
        while (!path_str.empty())
        {
            size_t next_comma = path_str.find(',');
            std::string_view segment_str = path_str.substr(0, next_comma);
            result.path.push_back(std::string(segment_str));

            if (next_comma == std::string_view::npos)
            {
                break;
            }

            path_str.remove_prefix(next_comma + 1);
        }
    }

    result.data = packet_string.substr(colon_pos + 1);

    return true;
}

APRS_ROUTER_INLINE bool try_route_packet(const struct packet& packet, const router_settings& settings, routing_result& result)
{
#ifdef APRS_ROUTER_APRS_DETAIL_NAMESPACE
    using namespace APRS_ROUTER_APRS_DETAIL_NAMESPACE;
#endif

    init_routing_result(packet, result);

    const std::string& router_address = settings.address;
    const routing_option options = settings.options;

    std::vector<segment> router_addresses;
    try_parse_segments(settings.path, router_addresses);
    
    // Addresses: N0CALL>APRS,CALL,WIDE1-1,WIDE2-2:data
    //                        ~~~~ ~~~~~~~ ~~~~~~~
    // n-N addresses: N0CALL>APRS,CALL,WIDE1-1,WIDE2-2:data 
    //                                 ~~~~~~~ ~~~~~~~
    // Generic addresses: N0CALL>APRS,CALL,WIDE1-1,WIDE2-2:data 
    //                                ~~~~
    std::vector<segment> router_n_N_addresses = get_router_n_N_addresses(router_addresses);
    std::vector<segment> router_generic_addresses = get_router_generic_addresses(router_addresses);
    std::vector<segment> packet_addresses = parse_packet_addresses(packet);

    if (settings.address.empty() || !is_packet_valid(packet, packet_addresses, options))
    {   
        return create_routing_result(routing_state::not_routed, result);
    }

    // Last used address: N0CALL>APRS,CALL*,DIGI,WIDE1,ROUTE,WIDE2-2:data 
    //                                ~~~~~
    // Router address: N0CALL>APRS,CALL*,DIGI,WIDE1,ROUTE,WIDE2-2:data
    //                                   ~~~~
    std::optional<size_t> maybe_last_used_address_index = find_last_used_address_index(packet_addresses);
    std::optional<size_t> maybe_router_address_index = find_unused_address_index(packet_addresses, maybe_last_used_address_index, router_address, router_generic_addresses);

    // Packet has finished routing: N0CALL>APRS,CALL,WIDE1,DIGI*:data
    //                                                     ~~~~~
    if (has_packet_routing_ended(packet_addresses, maybe_last_used_address_index))
    {
        return create_routing_result(routing_state::not_routed, result);
    }

    // Packet has already been routing by us: N0CALL>APRS,CALL,DIGI*,WIDE1-1,WIDE2-2:data
    //                                                         ~~~~~
    // Packet has ben sent to us: N0CALL>DIGI,CALL,WIDE1-1,WIDE2-2:data 
    //                                   ~~~~
    if (has_packet_been_routed_by_us(packet_addresses, maybe_last_used_address_index, router_address) ||
        is_packet_sent_to_us(packet, router_address))
    {
        return create_routing_result(routing_state::already_routed, result);
    }

    // Packet has ben sent by us: DIGI>APRS,CALL,WIDE1-1,WIDE2-2:data 
    //                            ~~~~
    bool is_routing_self = is_packet_from_us(packet, router_address);

    if (is_explicit_routing(is_routing_self, maybe_router_address_index, options))
    {
        if (try_explicit_route(packet_addresses, router_address, maybe_last_used_address_index, maybe_router_address_index, options))
        {
            return create_routing_result(packet, packet_addresses, result);
        }
        else
        {
            return create_routing_result(routing_state::not_routed, result);
        }
    }

    // Self routing is only allowed in explicit routing mode
    if (is_routing_self)
    {
        return create_routing_result(routing_state::cannot_route_self, result);
    }

    if (try_n_N_route(packet_addresses, router_n_N_addresses, router_address, options))
    {
        return create_routing_result(packet, packet_addresses, result);
    }

    return create_routing_result(routing_state::not_routed, result);
}

APRS_ROUTER_INLINE bool try_trace_packet(const std::vector<packet>& packets, std::vector<packet_trace>& traces)
{
    (void)packets;
    (void)traces;

    std::map<size_t, std::vector<packet>> packet_groups;

    for (size_t i = 0; i < packets.size(); i++)
    {
        const packet& p = packets[i];
        size_t packet_hash = hash(p);
        if (!packet_groups.contains(packet_hash))
        {
            packet_groups[packet_hash] = {};
        }
        packet_groups[packet_hash].push_back(p);
    }

    return true;
}

#endif

APRS_ROUTER_NAMESPACE_END

// **************************************************************** //
//                                                                  //
//                                                                  //
// PRIVATE DEFINITIONS                                              //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_NAMESPACE_BEGIN

APRS_ROUTER_DETAIL_NAMESPACE_BEGIN

#ifndef APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY

APRS_ROUTER_INLINE std::string to_string(const segment& p)
{
    std::string result = p.text;

    if (p.n > 0)
    {
        result += std::to_string(p.n);
    }

    if (p.N > 0)
    {
        result += "-" + std::to_string(p.N);
    }

    if (p.mark)
    {
        result += "*";
    }

    return result;
}

APRS_ROUTER_INLINE q_construct parse_q_construct(const std::string& input)
{
    static const std::unordered_map<std::string, q_construct> q_construct_map =
    {
        {"qAC", q_construct::qAC},
        {"qAX", q_construct::qAX},
        {"qAU", q_construct::qAU},
        {"qAo", q_construct::qAo},
        {"qAO", q_construct::qAO},
        {"qAS", q_construct::qAS},
        {"qAr", q_construct::qAr},
        {"qAR", q_construct::qAR},
        {"qAZ", q_construct::qAZ},
        {"qAI", q_construct::qAI}
    };

    auto it = q_construct_map.find(input);
    if (it != q_construct_map.end())
    {
        return it->second;
    }

    return q_construct::none;
}

APRS_ROUTER_INLINE segment_type parse_segment_type(const std::string& text)
{
    static const std::unordered_map<std::string, segment_type> lookup_table =
    {
        {"WIDE", segment_type::wide},
        {"TRACE", segment_type::trace},
        {"RELAY", segment_type::relay},
        {"TEMP", segment_type::temp},
        {"TCPIP", segment_type::tcpip},
        {"TCPXX", segment_type::tcpxx},
        {"NOGATE", segment_type::nogate},
        {"RFONLY", segment_type::rfonly},
        {"IGATECALL", segment_type::igatecall},        
        {"OPNTRK", segment_type::opntrk},
        {"OPNTRC", segment_type::opntrc}
    };

    auto it = lookup_table.find(text);
    return (it != lookup_table.end()) ? it->second : segment_type::other;
}

APRS_ROUTER_INLINE bool try_parse_segment(std::string_view path, segment& s)
{
    s.text = path;
    s.mark = false;

    q_construct q = parse_q_construct(s.text);
    s.q = q;
    if (q != q_construct::none)
    {
        s.n = 0;
        s.N = 0;
        s.type = segment_type::q;
    }
    else
    {
        if (!path.empty() && path.back() == '*')
        {
            s.mark = true;
            path.remove_suffix(1);
            s.text = path;
            s.N = 0;
            s.n = 0;
        }

        auto sep_pos = path.find("-");
        if (sep_pos != std::string::npos)
        {
            // Check if there are digits before and after the dash
            if (sep_pos > 0 && isdigit(path[sep_pos - 1]) && sep_pos + 1 < path.size() && isdigit(path[sep_pos + 1]))
            {
                // Ensure only one digit after the dash
                if (sep_pos + 2 == path.size())
                {
                    // Left of the dash + digit
                    s.text = path.substr(0, sep_pos - 1);

                    // Parse the n value (digit before the dash)
                    s.n = path[sep_pos - 1] - '0'; // Convert char to int

                    // Parse the N value (digit after the dash)
                    s.N = path[sep_pos + 1] - '0'; // Convert char to int

                    // Ensure valid range 1 - 7
                    if (s.N <= 0 || s.N > 7 || s.n <= 0 || s.n > 7)
                    {
                        s.N = 0;
                        s.n = 0;
                        s.text = path;
                    }
                }
            }
        }
        else // If there's no '-', assume only text or text followed by a single digit
        {
            s.text = path;

            // Check for trailing digit without '-'
            if (!path.empty() && isdigit(path.back()))
            {
                s.n = path.back() - '0'; // Last digit is n
                path.remove_suffix(1);   // Remove the digit from the text
                s.text = path;

                if (s.n <= 0 || s.n > 7)
                {
                    s.n = 0;
                    s.text = path;
                }
            }
            else
            {
                s.n = 0; // No digit found, default n to 0
            }

            s.N = 0; // No N specified, default to 0
        }

        s.type = parse_segment_type(s.text);
    }

    // Never fail. Failure to parse a segment means that n-N will not be set.
    return true;
}

APRS_ROUTER_INLINE bool try_parse_segments(const std::vector<std::string>& addresses, std::vector<segment>& result)
{
    for (const auto& a : addresses)
    {
        segment s;
        try_parse_segment(a, s);
        result.push_back(s);
    }
    return true;
}

APRS_ROUTER_INLINE bool try_parse_callsign(std::string address, std::string& callsign, int& ssid)
{
    if (address.empty())
    {
        return false;
    }

    if (address.size() > 9)
    {
        return false;
    }

    auto sep_pos = address.find("-");

    if (sep_pos != std::string::npos)
    {
        if (sep_pos == address.size() - 1 || ((sep_pos + 3) < address.size()))
        {
            return false;
        }

        callsign = address.substr(0, sep_pos);

        std::string ssid_str = std::string(address.substr(sep_pos + 1));

        if (ssid_str[0] == '0')
        {
            return false;
        }

        if (!std::isdigit(static_cast<unsigned char>(ssid_str[0])) ||
            (ssid_str.size() > 1 && !std::isdigit(static_cast<unsigned char>(ssid_str[1]))))
        {
            return false;
        }

        ssid = atoi(ssid_str.c_str());

        if (ssid < 0 || ssid > 15)
        {
            ssid = 0;
            return false;
        }
    }
    else
    {
        callsign = address;
        ssid = 0;
    }

    if (callsign.size() > 6)
    {
        return false;
    }

    for (char c : callsign)
    {
        if ((!std::isalnum(static_cast<unsigned char>(c)) || !std::isdigit(static_cast<unsigned char>(c))) &&
            !std::isupper(static_cast<unsigned char>(c)))
        {
            return false;
        }
    }

    return true;
}

APRS_ROUTER_INLINE bool try_parse_callsign_with_used_flag(std::string address, std::string& callsign, int& ssid)
{
    if (address.empty())
    {
        return false;
    }

    if (address.back() == '*')
    {
        address = address.substr(0, address.size() - 1);
    }

    return try_parse_callsign(address, callsign, ssid);
}

APRS_ROUTER_INLINE std::vector<segment> get_router_n_N_addresses(const std::vector<segment>& router_addresses)
{
    std::vector<segment> result;
    size_t i = 0;
    for (auto p : router_addresses)
    {
        if (p.n > 0)
        {
            p.index = i;
            result.push_back(p);
            i++;
        }
    }
    return result;
}

APRS_ROUTER_INLINE std::vector<segment> get_router_generic_addresses(const std::vector<segment>& router_addresses)
{
    std::vector<segment> result;
    for (const auto& p : router_addresses)
    {
        if (p.n == 0)
        {
            result.push_back(p);
        }
    }
    return result;
}

APRS_ROUTER_INLINE bool is_packet_valid(const struct packet& packet, const std::vector<segment>& packet_addresses, routing_option options)
{
    (void)packet_addresses;

    if (packet.from.empty() || packet.to.empty())
    {
        return false;
    }

    if (packet.path.empty() || packet.path.size() > 8)
    {
        return false;
    }

    if (!enum_has_flag(options, routing_option::strict))
    {        
        return true;
    }

    if (packet.data.size() > 256)
    {
        return false;
    }

    std::string callsign;
    int ssid;

    if (!try_parse_callsign_with_used_flag(packet.from, callsign, ssid) ||
        !try_parse_callsign_with_used_flag(packet.to, callsign, ssid))
    {
        return false;
    }

    for (const auto& p : packet.path)
    {
        if (!try_parse_callsign_with_used_flag(p, callsign, ssid))
        {
            return false;
        }
    }

    return true;
}

APRS_ROUTER_INLINE bool is_packet_from_us(const struct packet& packet, std::string_view router_address)
{
    return packet.from == router_address;
}

APRS_ROUTER_INLINE bool is_packet_sent_to_us(const struct packet& packet, std::string_view router_address)
{
    return packet.to == router_address;
}

APRS_ROUTER_INLINE bool has_packet_routing_ended(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index)
{
    if (maybe_last_used_address_index)
    {
        return (packet_addresses.size() == 0 || (maybe_last_used_address_index.value() == (packet_addresses.size() - 1)));
    }
    return false;
}

APRS_ROUTER_INLINE bool has_packet_been_routed_by_us(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, std::string_view router_address)
{
    if (!maybe_last_used_address_index)
    {
        return false;
    }

    size_t last_used_address_index = maybe_last_used_address_index.value();

    assert(last_used_address_index < packet_addresses.size());

    return (packet_addresses[last_used_address_index].text == router_address && packet_addresses[last_used_address_index].mark);
}

APRS_ROUTER_INLINE std::vector<segment> parse_packet_addresses(const struct packet& packet)
{
    std::vector<segment> segments;
    try_parse_segments(packet.path, segments);
    int i = 0;
    for (auto& s : segments)
    {
        s.index = i;
        i++;
    }
    return segments;
}

APRS_ROUTER_INLINE void init_routing_result(const struct packet& packet, routing_result& result)
{
    result.routed = false;
    result.success = false;
    result.original_packet = packet;
    result.routed_packet = packet;
}

APRS_ROUTER_INLINE bool adresses_contains_address(const std::vector<segment>& packet_addresses, const segment& address)
{
    for (const auto& a : packet_addresses)
    {
        if (address.text == a.text && address.n == a.n)
        {
            return true;
        }
    }
    return false;
}

APRS_ROUTER_INLINE std::optional<std::pair<size_t, size_t>> find_first_available_n_N_address_index(const std::vector<segment>& packet_addresses, const std::vector<segment>& router_addresses, routing_option options)
{
    bool reject_excessive_hops = enum_has_flag(options, routing_option::reject_excessive_hops);

    for (const auto& address : packet_addresses)
    {
        for (const auto& p : router_addresses)
        {
            if (address.text == p.text && address.n == p.n && address.N > 0)
            {
                if (reject_excessive_hops && p.N > 0 && address.N > p.N)
                {
                    continue;
                }
                return std::make_pair(address.index, p.index);
            }
        }
    }

    return {};
}

APRS_ROUTER_INLINE std::optional<size_t> find_last_used_address_index(const std::vector<segment>& packet_addresses)
{
    // Retrieve the last address that has been digipeated.
    // For example, if the packet is "FROM>TO,CALL*,TEST*,ADDRESS*,WIDE1-2:data",
    // the last digipeated address will be "ADDRESS".

    if (packet_addresses.empty())
    {
        return {};
    }

    for (int i = static_cast<int>(packet_addresses.size() - 1); i >= 0; i--)
    {
        if (packet_addresses[i].mark)
        {
            return i;
        }
    }

    return {};
}

APRS_ROUTER_INLINE std::optional<size_t> find_address_index(const std::vector<segment>& packet_addresses, size_t offset, std::string_view router_address, const std::vector<segment>& router_addresses)
{
    for (size_t i = offset; i < packet_addresses.size(); i++)
    {
        if (packet_addresses[i].text == router_address)
        {
            return i;
        }

        for (size_t j = 0; j < router_addresses.size(); j++)
        {
            if (packet_addresses[i].text == router_addresses[j].text)
            {
                return i;
            }
        }
    }

    return {};
}

APRS_ROUTER_INLINE bool create_routing_result(const packet& p, const std::vector<segment>& packet_addresses, routing_result& result)
{
    packet routed_packet = { p.from, p.to, {}, p.data };

    for (auto segment : packet_addresses)
    {
        if (!segment.text.empty())
        {
            routed_packet.path.push_back(to_string(segment));
        }
    }

    result.success = true;
    result.routed = true;
    result.routed_packet = routed_packet;
    result.state = routing_state::routed;

    return true;
}

APRS_ROUTER_INLINE bool create_routing_result(routing_state state, routing_result& result)
{
    result.success = true;
    result.routed = (state == routing_state::routed);
    result.state = state;
    return result.routed;
}

APRS_ROUTER_INLINE void unset_all_used_addresses(std::vector<segment>& packet_addresses, size_t offset, size_t count)
{
    for (size_t i = offset, n = 0; i < packet_addresses.size() && n < count; i++, n++)
    {
        packet_addresses[i].mark = false;
    }
}

APRS_ROUTER_INLINE void set_address_as_used(std::vector<segment>& packet_addresses, size_t index)
{
    assert(index < packet_addresses.size());
    unset_all_used_addresses(packet_addresses, 0, packet_addresses.size());
    packet_addresses[index].mark = true;
}

APRS_ROUTER_INLINE void set_address_as_used(std::vector<segment>& packet_addresses, segment& packet_address)
{
    unset_all_used_addresses(packet_addresses, 0, packet_addresses.size());
    packet_address.mark = true;
}

APRS_ROUTER_INLINE std::optional<size_t> find_unused_address_index(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, std::string_view router_address, const std::vector<segment>& router_generic_addresses)
{
    // Unused address mathing router's address or an address in the router's path:
    //
    // FROM>TO,CALL,ROUTE*,DIGI,WIDE2-1:data - if explicit routing
    //                     ~~~~
    // FROM>TO,CALL,ROUTE*,DIGI,WIDE2-1:data - if n-N routing
    //                          ~~~~~~~

    size_t last_used_address_index = 0;

    // there might be no "used" address
    if (maybe_last_used_address_index)
    {
        last_used_address_index = maybe_last_used_address_index.value();
    }

    assert(last_used_address_index < packet_addresses.size());

    std::optional<size_t> maybe_address_index = find_address_index(
        packet_addresses,
        last_used_address_index,
        router_address,
        router_generic_addresses);

    if (!maybe_address_index)
    {
        return {};
    }

    size_t address_index = maybe_address_index.value();

    assert(address_index < packet_addresses.size());

    if (!packet_addresses[address_index].mark)
    {
        return address_index;
    }

    return {};
}

APRS_ROUTER_INLINE void update_addresses_index(std::vector<segment>& addresses)
{
    for (size_t i = 0; i < addresses.size(); i++)
    {
        addresses[i].index = i;
    }
}

APRS_ROUTER_INLINE bool trap_n_N_route(std::vector<segment>& packet_addresses, size_t packet_n_N_address_index, const std::vector<segment>& router_addresses, size_t router_n_N_address_index, std::string_view router_address, routing_option options)
{
    assert(packet_n_N_address_index < packet_addresses.size());
    assert(router_n_N_address_index < router_addresses.size());

    // Replace an excessive hop with the router's address
    // to stop the packet from harming the network
    // 
    // Example:
    // Input:  FROM>TO,WIDE7-7:data
    // Output: FROM>TO,DIGI*:data

    bool trap_excessive_hops = enum_has_flag(options, routing_option::trap_excessive_hops);

    segment& n_N_address = packet_addresses[packet_n_N_address_index];
    const segment& router_n_N_address = router_addresses[router_n_N_address_index];

    if (trap_excessive_hops && router_n_N_address.N > 0 && n_N_address.N > router_n_N_address.N)
    {
        n_N_address.text = router_address;
        n_N_address.mark = true;
        n_N_address.n = 0;
        n_N_address.N = 0;

        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool insert_n_N_route(std::vector<segment>& packet_addresses, size_t& index, std::string_view router_address, routing_option options)
{
    assert(index < packet_addresses.size());

    bool substitute_zero_hops = enum_has_flag(options, routing_option::substitute_complete_hops);    

    segment& n_N_address = packet_addresses[index];

    decrement_address_n_N(n_N_address);

    // If we are in a position which will require us to insert more than 8 addresses
    // just return, the only thing we can do is increment the counter
    //
    // This packet:
    //
    // FROM>TO,A,B,C,D,E,F,G,WIDE2-2:data
    //         ~ ~ ~ ~ ~ ~ ~ ~~~~~~~
    //
    // Becomes:
    //
    // Output: FROM>TO,A,B,C,D,E,F,G,WIDE2-1:data
    //                               ~~~~~~~
    //
    // There are a few circumstances where we can still continue with the 
    // insertion even when the address count is 8. For example if we will
    // shortly shrink the packet path (substitute_complete_hops)

    if (packet_addresses.size() >= 8)
    {
        if (!substitute_zero_hops && n_N_address.N == 0)
        {
            set_address_as_used(packet_addresses, n_N_address); 
            return true;
        }
        if (!substitute_zero_hops || n_N_address.N > 0)
        {
            return true;
        }
    }

    unset_all_used_addresses(packet_addresses, 0, packet_addresses.size());

    // Insert router's address in front of the PATHn-N address:
    // 
    // FROM>TO,CALL,DIGI,WIDE2-2:data
    //              ~~~~
    //
    // Mark address as used:
    //
    // FROM>TO,CALL,DIGI*,WIDE2-2:data
    //                  ~

    segment s;
    s.text = router_address;    
    s.type = segment_type::other;

    if (substitute_zero_hops || n_N_address.N > 0)
    {
        s.mark = true;
    }
    else
    {
        set_address_as_used(packet_addresses, n_N_address);
    }

    packet_addresses.insert(packet_addresses.begin() + index, s);

    index++;    

    update_addresses_index(packet_addresses);

    return true;
}

APRS_ROUTER_INLINE bool insert_generic_route(std::vector<segment>& packet_addresses, size_t index, std::string_view router_address)
{
    assert(index < packet_addresses.size());

//    if (index < 0 || index >= packet_addresses.size())
    if (index >= packet_addresses.size())
    {
        return false;
    }

    unset_all_used_addresses(packet_addresses, 0, packet_addresses.size());

    segment s;
    s.text = router_address;

    packet_addresses.insert(packet_addresses.begin() + index, s);

    update_addresses_index(packet_addresses);
    return true;
}

APRS_ROUTER_INLINE bool try_preempt_explicit_route(std::vector<segment>& packet_addresses, size_t router_address_index, std::optional<size_t> maybe_last_used_address_index, routing_option options)
{
    assert(router_address_index < packet_addresses.size());

    size_t unused_address_index = 0;

    if (maybe_last_used_address_index)
    {
        unused_address_index = maybe_last_used_address_index.value() + 1;
    }

    assert(unused_address_index < packet_addresses.size());

    if (enum_has_flag(options, routing_option::preempt_front))
    {
        move_address_to_position(packet_addresses, router_address_index, unused_address_index);
        return true;
    }
    else if (enum_has_flag(options, routing_option::preempt_truncate))
    {
        truncate_address_range(packet_addresses, unused_address_index, router_address_index);
        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool is_explicit_routing(bool is_routing_self, std::optional<size_t> maybe_router_address_index, routing_option options)
{
    if (maybe_router_address_index)
    {
        return (!is_routing_self || enum_has_flag(options, routing_option::route_self));
    }
    return false;
}

APRS_ROUTER_INLINE bool try_explicit_route(std::vector<segment>& packet_addresses, std::string_view router_address, std::optional<size_t> maybe_last_used_address_index, std::optional<size_t> maybe_router_address_index, routing_option options)
{
    // If explicitly routing a packet through the ruter
    // find the router's address in the packet and mark it as used (*)
    // Also unmark all the previously used addresses.
    //
    // Preemtive routing allows us to ignore other packets in front of
    // us and proceed on the routing. There are two strategies that
    // can be used with preemtive routing, one which prioritizes our address
    // and what that eliminates unused addresses from the packet.

    if (!maybe_router_address_index)
    {
        return false;
    }

    size_t router_address_index = maybe_router_address_index.value();

    size_t unused_address_index = 0;

    if (maybe_last_used_address_index)
    {
        unused_address_index = maybe_last_used_address_index.value() + 1;
    }

    assert(unused_address_index < packet_addresses.size());
    assert(router_address_index < packet_addresses.size());

    bool is_path_based_routing = packet_addresses[router_address_index].text != router_address;
    bool have_other_addresses_ahead = router_address_index != unused_address_index;

    // If we don't have any addresses ahead of us, then proceed
    if (!have_other_addresses_ahead)
    {
        if (is_path_based_routing)
        {
            insert_generic_route(packet_addresses, unused_address_index, router_address);
            set_address_as_used(packet_addresses, router_address_index + 1);
        }
        else
        {
            set_address_as_used(packet_addresses, router_address_index);
        }
        return true;
    }
    else
    {
        if (try_preempt_explicit_route(packet_addresses, router_address_index, maybe_last_used_address_index, options))
        {
            if (is_path_based_routing)
            {
                insert_generic_route(packet_addresses, unused_address_index, router_address);
            }
            set_address_as_used(packet_addresses, unused_address_index);
            return true;
        }
    }

    return false;
}

APRS_ROUTER_INLINE bool try_n_N_route(std::vector<segment>& packet_addresses, const std::vector<segment>& router_n_N_addresses, const std::string& router_address, routing_option options)
{
    // Route an PATHn-N address. 
    //
    // Route a packet through a path that matches an address with the n-N format (e.g., WIDE2-1).
    //
    // Pick the very first matching address among the packet's addresses:
    //
    //   1. Decrement the 'N' part of the PATHn-N address to indicate one less hop remaining.
    //
    //   2. Insert the router's address in front of the matched address to mark it as the next hop.
    //      Mark the router's address with an asterix (*) to mark it as "used"
    //
    //   3. Normalize the remaining path by keeping only the router's address as used
    //
    //      If at step 1) the N-th part of the PATH is decremented to 0, then:
    //
    //        a) Replace the n-0 address with the router's address and mark it as "used".
    //           This is done when option
    //
    //   4. Create and update the routing result to reflect these changes.

    auto address_n_N_pair = find_first_available_n_N_address_index(packet_addresses, router_n_N_addresses, options);

    if (!address_n_N_pair)
    {
        return false;
    }

    auto [address_n_N_index, router_n_N_index] = address_n_N_pair.value();

    if (trap_n_N_route(packet_addresses, address_n_N_index, router_n_N_addresses, router_n_N_index, router_address, options))
    {
        return true;
    }

    insert_n_N_route(packet_addresses, address_n_N_index, router_address, options);

    if (enum_has_flag(options, routing_option::substitute_complete_hops))
    {
        remove_completed_n_N_address(packet_addresses, router_address, address_n_N_index);
    }
    
    return true;
}

APRS_ROUTER_INLINE bool move_address_to_position(std::vector<segment>& packet_addresses, size_t from_index, size_t to_index)
{
    // Shift and re-insert and address into the packet path, typically used for "preempt_front"
    //
    // If the router's address is "CALLD", a packet "N0CALL>APRS,CALLA*,CALLB,CALLC,CALLD,CALLE:data"
    //                                                                  ~~~~~~~~~~~~~~~~~
    // will be updated to move the "CALLD" addresses in the front of the path and become
    // "N0CALL>APRS,CALLA,CALLD*,CALLB,CALLC,CALLE:data"
    //                    ~~~~~~
    // The digipeated marker isn't done by this function

    if (from_index >= packet_addresses.size() ||
        to_index >= packet_addresses.size())
    {
        return false;
    }

    if (from_index == to_index)
    {
        return true;
    }

    segment s = packet_addresses[from_index];

    packet_addresses.erase(packet_addresses.begin() + from_index);

    packet_addresses.insert(packet_addresses.begin() + to_index, s);

    return true;
}

APRS_ROUTER_INLINE bool truncate_address_range(std::vector<segment>& packet_addresses, size_t start_index, size_t end_index)
{
    // Truncate a range of addresses, typically used for "preempt_truncate"
    //
    // If the router's address is "CALLD", a packet "N0CALL>APRS,CALLA*,CALLB,CALLC,CALLD,CALLE:data"
    //                                                                  ~~~~~~~~~~~~~~~~~
    // will be truncated of the "CALLB,CALLC,CALLD" addresses in the path and become
    // "N0CALL>APRS,CALLA,CALLD*,CALLE:data"
    //                    ~~~~~~
    // The digipeated marker and re-insertion of "CALLD" isn't done by this function

    if (start_index >= packet_addresses.size() ||
        end_index >= packet_addresses.size() ||
        start_index >= end_index)
    {
        return false;
    }

    segment s = packet_addresses[end_index];
    
    packet_addresses.erase(packet_addresses.begin() + start_index, packet_addresses.begin() + end_index + 1);
    
    packet_addresses.insert(packet_addresses.begin() + start_index, s);

    return true;
}

APRS_ROUTER_INLINE void decrement_address_n_N(std::vector<segment>& packet_addresses, size_t index)
{
    // Decrements an n-N address, ex: WIDE2-2 becomes WIDE2-1

    assert(index < packet_addresses.size());
    segment& s = packet_addresses[index];
    decrement_address_n_N(s);
}

APRS_ROUTER_INLINE void decrement_address_n_N(segment& s)
{
    // Decrements an n-N address, ex: WIDE2-2 becomes WIDE2-1

    if (s.N > 0)
    {
        s.N--;
    }
}

APRS_ROUTER_INLINE void remove_completed_n_N_address(std::vector<segment>& packet_addresses, std::string_view router_address, size_t index)
{
    assert(index < packet_addresses.size());

    // If the last PATHn-N hop has been exhausted, shrink the packet path
    //
    // Example:
    // Input:  FROM>TO,WIDE1-1,WIDE2-1:data
    // Output: FROM>TO,DIGI*,WIDE2-1:data - replace WIDE1 with DIGI

    segment& s = packet_addresses[index];
    if (s.N == 0)
    {
        unset_all_used_addresses(packet_addresses, 0, packet_addresses.size());

        s.text = router_address;
        s.N = 0;
        s.n = 0;
        s.mark = true;
        s.type = segment_type::other;

        packet_addresses.erase(packet_addresses.begin() + index - 1);

        update_addresses_index(packet_addresses);
    }
}

#endif

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END
