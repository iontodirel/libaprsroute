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
//
// References:
//
//   - APRS specification: http://www.aprs.org/doc/APRS101.PDF
//   - APRS 1.1 specification addendum: http://www.aprs.org/aprs11.html
//   - APRS 1.2 specification addendum: http://www.aprs.org/aprs12.html
//   - The New n-N Paradigm: http://www.aprs.org/fix14439.html
//   - Preemptive Digipeating: http://www.aprs.org/aprs12/preemptive-digipeating.txt
//   - Q Construct: https://www.aprs-is.net/q.aspx
//   - APRS digipeaters: https://dvbr.net/cloud/direwolf/direwolf_git_v1.7/direwolf-doc_git/APRS-Digipeaters.pdf
//   - How APRS paths work: https://blog.aprs.fi/2020/02/how-aprs-paths-work.html

#pragma once

#include <string>
#include <string_view>
#include <vector>
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

// Routing options:
//
// ----------
// route_self
// ----------
// 
// Enables routing packets originating from ourselves.
// This option only works in explicit routing mode.
//
// This packet: DIGI>APRS,DIGI:data
//              ~~~~
// Will be routed as: DIGI>APRS,DIGI*:data
//                    ~~~~      ~~~~~
// However a packet using n-N routing: DIGI>APRS,WIDE2-2:data, will not be routed even with route_self set
//                                     ~~~~ 
// -------------
// preempt_front
// -------------
//
// Enables preemtive routing for explicitly routed packets.
// With this option, our address is moved to the front of the route.
//
// This packet: N0CALL>APRS,CALLA,CALLB*,CALLC,DIGI,CALLD,CALLE,CALLF:data
//                                             ~~~~
// Will be routed as: N0CALL>APRS,CALLA,CALLB,DIGI*,CALLC,CALLD,CALLE,CALLF:data
//                                            ~~~~~
// ----------------
// preempt_truncate
// ----------------
//
// Enables preemtive routing for explicitly routed packets.
// With this option, our address is moved behind the last used address, and all other addresses are erased.
//
// This packet: N0CALL>APRS,CALLA,CALLB*,CALLC,DIGI,CALLD,CALLE,CALLF:data
//                                             ~~~~
// Will be routed as: N0CALL>APRS,CALLA,CALLB,DIGI*,CALLD,CALLE,CALLF:data
//                                            ~~~~~
// ------------
// preempt_drop
// ------------
//
// Enables preemtive routing for explicitly routed packets.
// With this option, all other addresses in front of us are erased.
//
// This packet: N0CALL>APRS,CALLA,CALLB*,CALLC,DIGI,CALLD,CALLE,CALLF:data
//                                             ~~~~
// Will be routed as: N0CALL>APRS,DIGI*,CALLD,CALLE,CALLF:data
//                                ~~~~~
// ------------------------
// substitute_complete_hops
// ------------------------
//
// Replace exaused hops.
//
// This packet: N0CALL>APRS,CALLA,WIDE1-1,WIDE2-2:data
//                                ~~~~~~~
// Will be routed as: N0CALL>APRS,CALLA,DIGI*,WIDE2-2:data
//                                      ~~~~~
// ------------------------
// trap_excessive_hops
// ------------------------
//
// Replace excessive hops.
//
// This packet: N0CALL>APRS,CALLA,WIDE7-7,WIDE2-2:data
//                                ~~~~~~~
// Will be routed as: N0CALL>APRS,CALLA,DIGI*,WIDE2-2:data
//                                      ~~~~~
// ------------------------
// reject_excessive_hops
// ------------------------
//
// Reject excessive hops.
//
// This packet won't be routed: N0CALL>APRS,CALLA,WIDE7-7,WIDE2-2:data
//                                                ~~~~~~~
// ------
// strict
// ------
//
// Enforce strict packet validation.
//
// This ensures that addresses (callsigns) are valid, that data is present in the packet,
// and it does not exceed the APRS limit of 256.
//
// ---------------------------
// substitute_explicit_address
// ---------------------------
//
// Replace an address with the router's callsign when explicit routing
//
// This packet: N0CALL>APRS,CALLA:data
//                          ~~~~~
// Will be routed as: N0CALL>APRS,DIGI*:data
//                                ~~~~~
// But if the option is not set, it will be routed as: N0CALL>APRS,DIGI,CALLA*:data
//                                                                 ~~~~~~~~~~~
// If the address matches the router's address or one of the generic addresses,
// I do believe it's a good idea to mirror the n-N routing behavior and add 
// the router's address in front of the generic address, so we have traceability

enum class routing_option : int
{
    none = 0,
    route_self = 1,                     // Enable routing packets originating from ourselves.
    preempt_front = 2,                  // Preemptively move our address to the front of the route.
    preempt_truncate = 4,               // Preemptively move our address behind the last used address and erase all other addresses.
    preempt_drop = 8,                   // Preemptively erase all addresses in front of our address.
    preempt_mark = 16,                  // Preemptively mark our address as used, while leaving the rest of the path as is.
    substitute_complete_hops = 32,      // Replace an CALLn-N address with our address when N decrements to 0.
    trap_excessive_hops = 64,           // Replace a harmful path with our callsign to prevent network issues (e.g., WIDE7-7).
    reject_excessive_hops = 128,        // Reject the packet if the paths has excessive hops (e.g., PATH7-7).
    strict = 256,                       // Don't route if the packet is malformed
    preempt_n_N = 512,                  // Enables preemptive routing in packets using n-N routing
    substitute_explicit_address = 1024  // Replace an address with the router's callsign when explicit routing
};

// Routing settings:
//
// address - this is our callsign (or the router's callsign)
//           if implementing a digipeater, this should be set to the digipeater's callsign
//
// path - contains an optional list of aliases, and an optional list of generic n-N addresses
//
//        unlike most digipeaters, this setting contains both aliases and n-N addresses at the same time
//        however, they are different settings, and the router will process and prioritize them both correctly
//    
//        ex: WIDE1,RELAY,WIDE2,WIDE3-2
//            ~~~~~       ~~~~~
//            n-N addresses the router will respond to
//
//        ex: WIDE1,RELAY,WIDE2,WIDE3-2
//                  ~~~~~
//                  an alias the router will respond to (alongside the n-N addresses)
//
//        ex: WIDE1,RELAY,WIDE2,WIDE3-2
//                              ~~~~~~~
//                              an n-N address with a hop constrain
//                              packets matching this n-N address, ex: N0CALL>APRS,WIDE3-4:data
//                              will be rejected or trapped if their hop count exceeds the maximum (2)
//                              specified (WIDE3-4 > WIDE3-2 and will be rejected or trapped)
//
//        default setting: WIDE1-2,WIDE2-2,TRACE1-2,TRACE2-2,WIDE,RELAY,TRACE
//
// options - contains a list of options, ex: "preempt_front | trap_excessive_hops" will enable two options on the router
//
// enable_diagnostics - generates routing diagnostics that can be accessed via the routing_result::actions

struct router_settings
{
    std::string address;
    std::vector<std::string> path = { "WIDE1-2", "WIDE2-2", "TRACE1-2", "TRACE2-2", "WIDE", "RELAY", "TRACE" };
    routing_option options = routing_option::none;
    bool enable_diagnostics = false;
};

enum class routing_state
{
    routed,
    not_routed,
    already_routed,
    no_matching_addresses,
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
    qAC, // Server: Verified login via bidirectional port.
    qAX, // Server: Unverified login.
    qAU, // Server: Direct via UDP.
    qAo, // Server: Gated packet via client-only port.
    qAO, // Server: Non-gated packet via send-only port or indirect packet via client-only port. Client: Gated packet from RF without messaging.
    qAS, // Server: Packet via server without q construct.
    qAr, // Server: Gated packet using ,I construct from remote IGate.
    qAR, // Server: Gated packet using ,I construct with verified IGate login. Client: Gated packet from RF.
    qAZ, // Client: Server-client command packet.
    qAI  // Client: Trace packet.
};

enum class segment_type
{
    other,
    wide,
    trace,
    relay,
    echo,
    gate,
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
    int ssid = 0;
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
APRS_ROUTER_INLINE std::vector<segment> parse_packet_addresses(const struct packet& packet);
APRS_ROUTER_INLINE std::optional<std::pair<size_t, size_t>> find_first_unused_n_N_address_index(const std::vector<segment>& packet_addresses, const std::vector<segment>& router_addresses, routing_option options);
APRS_ROUTER_INLINE std::optional<size_t> find_last_used_address_index(const std::vector<segment>& packet_addresses);
APRS_ROUTER_INLINE std::optional<size_t> find_address_index(const std::vector<segment>& packet_addresses, size_t offset, std::string_view router_address, const std::vector<segment>& router_addresses);
APRS_ROUTER_INLINE std::optional<size_t> find_unused_address_index(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, std::string_view router_address, const std::vector<segment>& router_generic_addresses);
APRS_ROUTER_INLINE bool is_packet_valid(const struct packet& packet, routing_option options);
APRS_ROUTER_INLINE bool is_packet_from_us(const struct packet& packet, std::string_view router_address);
APRS_ROUTER_INLINE bool is_packet_sent_to_us(const struct packet& packet, std::string_view router_address);
APRS_ROUTER_INLINE bool has_packet_routing_ended(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index);
APRS_ROUTER_INLINE bool has_packet_been_routed_by_us(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, std::string_view router_address);
APRS_ROUTER_INLINE void init_routing_result(const struct packet& packet, routing_result& result);
APRS_ROUTER_INLINE bool create_routing_result(const packet& p, const std::vector<segment>& packet_addresses, routing_result& result);
APRS_ROUTER_INLINE bool create_routing_result(routing_state state, routing_result& result);
APRS_ROUTER_INLINE void unset_all_used_addresses(std::vector<segment>& packet_addresses, size_t offset, size_t count);
APRS_ROUTER_INLINE void set_address_as_used(std::vector<segment>& packet_addresses, size_t index);
APRS_ROUTER_INLINE void set_address_as_used(std::vector<segment>& packet_addresses, segment& address);
APRS_ROUTER_INLINE void update_addresses_index(std::vector<segment>& addresses);
APRS_ROUTER_INLINE bool try_preempt_explicit_route(std::vector<segment>& packet_addresses, size_t router_address_index, size_t unused_address_index, bool is_path_based_routing, std::string_view router_address, routing_option options);
APRS_ROUTER_INLINE bool try_preempt_transform_explicit_route(std::vector<segment>& packet_addresses, size_t router_address_index, size_t& set_address_index, routing_option options);
APRS_ROUTER_INLINE bool try_insert_address(std::vector<segment>& packet_addresses, size_t index, std::string_view router_address);
APRS_ROUTER_INLINE bool is_explicit_routing(bool is_routing_self, std::optional<size_t> maybe_router_address_index, routing_option options);
APRS_ROUTER_INLINE bool try_explicit_route(std::vector<segment>& packet_addresses, std::string_view router_address, std::optional<size_t> maybe_last_used_address_index, std::optional<size_t> maybe_router_address_index, routing_option options);
APRS_ROUTER_INLINE bool try_explicit_basic_route(std::vector<segment>& packet_addresses, bool is_path_based_routing, size_t unused_address_index, size_t set_address_index, std::string_view router_address, routing_option options);
APRS_ROUTER_INLINE bool try_move_address_to_position(std::vector<segment>& packet_addresses, size_t from_index, size_t to_index);
APRS_ROUTER_INLINE bool try_truncate_address_range(std::vector<segment>& packet_addresses, size_t from_index, size_t to_index);
APRS_ROUTER_INLINE bool try_n_N_route(std::vector<segment>& packet_addresses, const std::vector<segment>& router_n_N_addresses, const std::string& router_address, routing_option options);
APRS_ROUTER_INLINE bool try_n_N_route_no_trap(std::vector<segment>& packet_addresses, size_t& packet_n_N_address_index, std::string_view router_address, routing_option options);
APRS_ROUTER_INLINE bool try_complete_n_N_route(std::vector<segment>& packet_addresses, segment& n_N_address, bool substitute_zero_hops);
APRS_ROUTER_INLINE bool try_insert_n_N_route(std::vector<segment>& packet_addresses, size_t& packet_n_N_address_index, std::string_view router_address, bool substitute_zero_hops);
APRS_ROUTER_INLINE bool try_trap_n_N_route(segment& packet_n_N_address, const segment& router_n_N_address, std::string_view router_address, routing_option options);
APRS_ROUTER_INLINE bool try_substitute_complete_n_N_address(std::vector<segment>& packet_addresses, size_t packet_n_N_address_index, std::string_view router_address);
APRS_ROUTER_INLINE bool try_decrement_address_n_N(std::vector<segment>& packet_addresses, size_t index);
APRS_ROUTER_INLINE bool try_decrement_address_n_N(segment& s);

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
    return static_cast<routing_option>(static_cast<int>(lhs) | static_cast<int>(rhs));
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
    else if (str == "preempt_drop")
    {
        result = routing_option::preempt_drop;
    }
    else if (str == "preempt_mark")
    {
        result = routing_option::preempt_mark;
    }
    else if (str == "substitute_complete_hops")
    {
        result = routing_option::substitute_complete_hops;
    }
    else if (str == "substitute_explicit_address")
    {
        result = routing_option::substitute_explicit_address;
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
    return (static_cast<int>(value) & static_cast<int>(flag)) != 0;
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
    // Does not guarantee formatting a correct packet string
    // if the input packet is invalid ex: missing path

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
    // Parse a packet: N0CALL>APRS,CALLA,CALLB*,CALLC,CALLD,CALLE,CALLF,CALLG:data
    //                 ~~~~~~ ~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~
    //                 from   to   path                                       data
    //
    // If packet string is invalid, filling the the packet fields is not guaranteed,
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
    //
    // n-N addresses: N0CALL>APRS,CALL,WIDE1-1,WIDE2-2:data 
    //                                 ~~~~~~~ ~~~~~~~
    //
    // Generic addresses: N0CALL>APRS,CALL,WIDE1-1,WIDE2-2:data 
    //                                ~~~~
    std::vector<segment> router_n_N_addresses = get_router_n_N_addresses(router_addresses);
    std::vector<segment> router_generic_addresses = get_router_generic_addresses(router_addresses);
    std::vector<segment> packet_addresses = parse_packet_addresses(packet);

    if (settings.address.empty() || !is_packet_valid(packet, options))
    {   
        return create_routing_result(routing_state::not_routed, result);
    }

    // Last used address: N0CALL>APRS,CALL*,DIGI,WIDE1,ROUTE,WIDE2-2:data 
    //                                ~~~~~
    //
    // Router addresses: N0CALL>APRS,CALL*,DIGI,WIDE1,ROUTE,WIDE2-2:data
    //                                     ~~~~
    //                   N0CALL>APRS,CALL*,DIGI,WIDE,WIDE1,ROUTE,WIDE2-2:data
    //                                          ~~~~
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
    if (has_packet_been_routed_by_us(packet_addresses, maybe_last_used_address_index, router_address))
    {
        return create_routing_result(routing_state::already_routed, result);
    }

    // Packet has been sent to us: N0CALL>DIGI,CALL,WIDE1-1,WIDE2-2:data 
    //                                    ~~~~
    if (is_packet_sent_to_us(packet, router_address))
    {
        return create_routing_result(routing_state::not_routed, result);
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
    if (p.text.empty())
    {
        return "";
    }

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

APRS_ROUTER_INLINE q_construct parse_q_construct(const std::string& text)
{
    static const std::unordered_map<std::string, q_construct> lookup_table =
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

    if (auto it = lookup_table.find(text); it != lookup_table.end())
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
        {"ECHO", segment_type::echo},
        {"GATE", segment_type::gate},
        {"TEMP", segment_type::temp},
        {"TCPIP", segment_type::tcpip},
        {"TCPXX", segment_type::tcpxx},
        {"NOGATE", segment_type::nogate},
        {"RFONLY", segment_type::rfonly},
        {"IGATECALL", segment_type::igatecall},        
        {"OPNTRK", segment_type::opntrk},
        {"OPNTRC", segment_type::opntrc}
    };

    if (auto it = lookup_table.find(text); it != lookup_table.end())
    {
        return it->second;
    }

    return segment_type::other;
}

APRS_ROUTER_INLINE bool try_parse_segment(std::string_view path, segment& s)
{
    s.text = path;
    s.mark = false;
    s.ssid = 0;

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

        auto sep_position = path.find("-");

        if (sep_position != std::string::npos)
        {
            // Check if there are digits before and after the dash
            if (sep_position > 0 && isdigit(path[sep_position - 1]) && (sep_position + 1) < path.size() && isdigit(path[sep_position + 1]))
            {
                // Ensure only one digit after the dash
                if (sep_position + 2 == path.size())
                {
                    // Left of the dash + digit
                    s.text = path.substr(0, sep_position - 1);

                    // Parse the n value (digit before the dash)
                    s.n = path[sep_position - 1] - '0'; // Convert char to int

                    // Parse the N value (digit after the dash)
                    s.N = path[sep_position + 1] - '0'; // Convert char to int

                    // Ensure valid range 1 - 7
                    if (s.N <= 0 || s.N > 7 || s.n <= 0 || s.n > 7)
                    {
                        s.N = 0;
                        s.n = 0;
                        s.text = path;
                    }
                }
            }
            // Handle parsing of a callsign SSID
            else if ((sep_position + 1) < path.size() && isdigit(path[sep_position + 1]))
            {
                s.text = path;
                s.N = 0;
                s.n = 0;

                std::string ssid_str = std::string(path.substr(sep_position + 1));

                if (ssid_str.size() == 1 || (ssid_str.size() == 2 && std::isdigit(ssid_str[1])))
                {
                    s.ssid = atoi(ssid_str.c_str());
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

    auto sep_position = address.find("-");

    if (sep_position != std::string::npos)
    {
        if (sep_position == address.size() - 1 || ((sep_position + 3) < address.size()))
        {
            return false;
        }

        callsign = address.substr(0, sep_position);

        std::string ssid_string = std::string(address.substr(sep_position + 1));

        if (ssid_string[0] == '0')
        {
            return false;
        }

        if (!std::isdigit(static_cast<unsigned char>(ssid_string[0])) ||
            (ssid_string.size() > 1 && !std::isdigit(static_cast<unsigned char>(ssid_string[1]))))
        {
            return false;
        }

        ssid = atoi(ssid_string.c_str());

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
        // Has to be alphanumeric and uppercase, or a digit
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

APRS_ROUTER_INLINE std::vector<segment> parse_packet_addresses(const struct packet& packet)
{
    std::vector<segment> segments;
    try_parse_segments(packet.path, segments);
    size_t i = 0;
    for (auto& s : segments)
    {
        s.index = i;
        i++;
    }
    return segments;
}

APRS_ROUTER_INLINE std::optional<std::pair<size_t, size_t>> find_first_unused_n_N_address_index(const std::vector<segment>& packet_addresses, const std::vector<segment>& router_addresses, routing_option options)
{
    // Find the first unused n-N address inside the packet
    // using the router's matching addresses
    //
    // Packet: N0CALL>APRS,WIDE1,WIDE2-2:data
    //                           ~~~~~~~
    // Router addresses: WIDE1,WIDE2,WIDE3-2
    //                         ~~~~~
    // Found addresses in packet: WIDE2-2
    //
    // If "reject_excessive_hops" is set, n-N addresses with excessive hops with
    // be ignored: N0CALL>APRS,WIDE3-3,WIDE2-2:data
    //                         ~~~~~~~
    // Found addresses in packet: WIDE2-2

    bool reject_excessive_hops = enum_has_flag(options, routing_option::reject_excessive_hops);

    for (const auto& address : packet_addresses)
    {
        for (const auto& p : router_addresses)
        {
            if (address.n == p.n && address.N > 0 && address.text == p.text)
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
    // Find the last address that has been marked as "used" in the packet path.
    // For example, if the packet is: FROM>TO,CALL*,TEST,ADDRESS*,WIDE1-2:data
    //                                                   ~~~~~~~~
    // the last used address will be "ADDRESS".

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

APRS_ROUTER_INLINE std::optional<size_t> find_unused_address_index(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, std::string_view router_address, const std::vector<segment>& router_generic_addresses)
{
    // Find unused address mathing router's address or an address in the router's path.
    // Start the search from the last used address.
    //
    // FROM>TO,CALL,ROUTE*,DIGI,WIDE2-1:data - if explicit routing by router address
    //                     ~~~~
    // FROM>TO,CALL,ROUTE*,WIDE,DIGI,WIDE2-1:data - if explicit routing by router generic addresses
    //                     ~~~~
    // FROM>TO,CALL,ROUTE*,DIGI,WIDE2-1:data - if n-N routing
    //                          ~~~~~~~

    size_t start_search_address_index = 0;

    // there might be no "used" address
    if (maybe_last_used_address_index)
    {
        start_search_address_index = maybe_last_used_address_index.value();
    }

    assert(start_search_address_index < packet_addresses.size());

    std::optional<size_t> maybe_address_index = find_address_index(
        packet_addresses,
        start_search_address_index,
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

APRS_ROUTER_INLINE bool is_packet_valid(const struct packet& packet, routing_option options)
{
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

    if (packet.data.size() == 0 || packet.data.size() > 256)
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
    // Packet routing ended: N0CALL>APRS,A,B,C,D,E,F,G,CALL*:data
    //                                                 ~~~~~
    //
    // If maybe_last_used_address_index is not set,
    // then we have not found address with the set flag
    //
    // If the last address in the packet is an address with the used flag (*)
    // then the packet routing has ended

    if (maybe_last_used_address_index)
    {
        return (packet_addresses.size() == 0 || (maybe_last_used_address_index.value() == (packet_addresses.size() - 1)));
    }
    return false;
}

APRS_ROUTER_INLINE bool has_packet_been_routed_by_us(const std::vector<segment>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, std::string_view router_address)
{
    // Packet was routed by us: N0CALL>APRS,A,B,C,D,DIGI*,F,G,CALL:data
    //                                              ~~~~~
    // 
    // A packet that has already been routed by us, should not be routed again
    // This should be true, even if the routed packet matches one of our aliases
    //
    // Packet routed by us via one of the aliases: N0CALL>APRS,A,B,C,D,ROUTER*,DIGI,F,G,CALL:data
    //                                                                 ~~~~~~~

    if (!maybe_last_used_address_index)
    {
        return false;
    }

    size_t last_used_address_index = maybe_last_used_address_index.value();

    assert(last_used_address_index < packet_addresses.size());

    const segment& last_used_address = packet_addresses[last_used_address_index];

    return (last_used_address.text == router_address && last_used_address.mark);
}

APRS_ROUTER_INLINE void init_routing_result(const struct packet& packet, routing_result& result)
{
    result.routed = false;
    result.success = false;
    result.original_packet = packet;
    result.routed_packet = packet;
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
    // Unset all addresses marked as used inside a packet path
    //
    // Packet: N0CALL>APRS,CALLA*,CALLB,CALLC*,CALLD,WIDE1-2:data
    //                     ~~~~~~       ~~~~~~
    // Will be updated to: N0CALL>APRS,CALLA,CALLB,CALLC,CALLD,WIDE1-2:data

    for (size_t i = offset, n = 0; i < packet_addresses.size() && n < count; i++, n++)
    {
        packet_addresses[i].mark = false;
    }
}

APRS_ROUTER_INLINE void set_address_as_used(std::vector<segment>& packet_addresses, size_t index)
{
    // Mark an address at "index" as used: N0CALL>APRS,CALLA,CALLB,CALLC,CALLD,WIDE1-2:data
    //
    // If index is "2" packet will be updated to: N0CALL>APRS,CALLA,CALLB,CALLC*,CALLD,WIDE1-2:data
    //                                                                    ~~~~~~

    assert(index < packet_addresses.size());
    unset_all_used_addresses(packet_addresses, 0, packet_addresses.size());
    packet_addresses[index].mark = true;
}

APRS_ROUTER_INLINE void set_address_as_used(std::vector<segment>& packet_addresses, segment& packet_address)
{
    unset_all_used_addresses(packet_addresses, 0, packet_addresses.size());
    packet_address.mark = true;
}

APRS_ROUTER_INLINE void update_addresses_index(std::vector<segment>& addresses)
{
    for (size_t i = 0; i < addresses.size(); i++)
    {
        addresses[i].index = i;
    }
}

APRS_ROUTER_INLINE bool try_preempt_explicit_route(std::vector<segment>& packet_addresses, size_t router_address_index, size_t unused_address_index, bool is_path_based_routing, std::string_view router_address, routing_option options)
{
    if (try_preempt_transform_explicit_route(packet_addresses, router_address_index, unused_address_index, options))
    {
        try_explicit_basic_route(packet_addresses, is_path_based_routing, unused_address_index, unused_address_index, router_address, options);
        return true;
    }
    return false;
}

APRS_ROUTER_INLINE bool try_preempt_transform_explicit_route(std::vector<segment>& packet_addresses, size_t router_address_index, size_t& set_address_index, routing_option options)
{
    assert(router_address_index < packet_addresses.size());
    assert(set_address_index < packet_addresses.size());

    if (enum_has_flag(options, routing_option::preempt_front))
    {
        try_move_address_to_position(packet_addresses, router_address_index, set_address_index);
        return true;
    }
    else if (enum_has_flag(options, routing_option::preempt_truncate))
    {
        try_truncate_address_range(packet_addresses, set_address_index, router_address_index);
        return true;
    }
    else if (enum_has_flag(options, routing_option::preempt_drop))
    {
        try_truncate_address_range(packet_addresses, 0, router_address_index);
        set_address_index = 0;
        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool try_insert_address(std::vector<segment>& packet_addresses, size_t index, std::string_view router_address)
{
    assert(index < packet_addresses.size());

    if (packet_addresses.size() >= 8)
    {
        return false;
    }

    segment new_address;
    new_address.text = router_address;

    assert(packet_addresses.size() < 8);

    packet_addresses.insert(packet_addresses.begin() + index, new_address);

    update_addresses_index(packet_addresses);

    return true;
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
    // If explicitly routing a packet through the router
    // find the router's address in the packet and mark it as used (*)
    // Also unmark all the previously used addresses.
    //
    // Preemtive routing allows us to ignore other packets in front of
    // us and proceed on the routing. There are two strategies that
    // can be used with preemtive routing, one which prioritizes our address
    // and what that eliminates unused addresses from the packet.

    if (!maybe_router_address_index)
    {
        // We did not find router's address 
        // or a generic router address in the packet path
        return false;
    }

    size_t router_address_index = maybe_router_address_index.value();

    assert(router_address_index < packet_addresses.size());

    size_t unused_address_index = 0;

    if (maybe_last_used_address_index)
    {
        unused_address_index = maybe_last_used_address_index.value() + 1;
    }

    assert(unused_address_index < packet_addresses.size());

    // is_path_based_routing - this means that the matching address is not the router's address,
    // but comes from the router's path
    //
    // Example:
    //
    // Router's address: DIGI
    //
    // Router's path:    WIDE1,CALLA,CALLB
    //
    // A packet matching the router's address: N0CALL>APRS,DIGI:data
    //                                                     ~~~~
    //                                                     is_path_based_routing=false
    //
    // A packet matching the router's path: N0CALL>APRS,CALLA:data
    //                                                  ~~~~~
    //                                                  is_path_based_routing=true

    bool is_path_based_routing = packet_addresses[router_address_index].text != router_address;
    bool have_other_unused_addresses_ahead = router_address_index != unused_address_index;

    bool preempt_drop = enum_has_flag(options, routing_option::preempt_drop);

    // If we don't have any unused addresses ahead of us, then proceed
    // If preempt_drop mode is enabled, different processing of the packet is required
    if (!have_other_unused_addresses_ahead && !preempt_drop)
    {
        try_explicit_basic_route(packet_addresses, is_path_based_routing, unused_address_index, router_address_index, router_address, options);
        return true;
    }
    else
    {
        if (try_preempt_explicit_route(packet_addresses, router_address_index, unused_address_index, is_path_based_routing, router_address, options))
        {
            return true;
        }
    }

    return false;
}

APRS_ROUTER_INLINE bool try_explicit_basic_route(std::vector<segment>& packet_addresses, bool is_path_based_routing, size_t unused_address_index, size_t set_address_index, std::string_view router_address, routing_option options)
{

    assert(set_address_index < packet_addresses.size());

    bool substitute_explicit_address = enum_has_flag(options, routing_option::substitute_explicit_address);

    if (substitute_explicit_address)
    {
        packet_addresses[set_address_index].text = router_address;
        set_address_as_used(packet_addresses, set_address_index);
        return true;
    }

    if (is_path_based_routing)
    {
        if (try_insert_address(packet_addresses, unused_address_index, router_address))
        {
            set_address_as_used(packet_addresses, set_address_index + 1);
        }
        else
        {
            packet_addresses[set_address_index].text = router_address;
            set_address_as_used(packet_addresses, set_address_index);
        }
    }
    else
    {
        set_address_as_used(packet_addresses, set_address_index);
    }

    return true;
}

APRS_ROUTER_INLINE bool try_move_address_to_position(std::vector<segment>& packet_addresses, size_t from_index, size_t to_index)
{
    // Shift and re-insert and address into the packet path, typically used for "preempt_front"
    //
    // If the router's address is "CALLD", a packet "N0CALL>APRS,CALLA*,CALLB,CALLC,CALLD,CALLE:data"
    //                                                                  ~~~~~~~~~~~~~~~~~
    // will be updated to move the "CALLD" addresses in the front of the path and become
    // "N0CALL>APRS,CALLA,CALLD*,CALLB,CALLC,CALLE:data"
    //                    ~~~~~~
    // The digipeated marker isn't done by this function

    assert(from_index < packet_addresses.size());
    assert(to_index < packet_addresses.size());

    if (from_index >= packet_addresses.size() ||
        to_index >= packet_addresses.size())
    {
        return false;
    }

    if (from_index == to_index)
    {
        return false;
    }

    segment address = packet_addresses[from_index];

    packet_addresses.erase(packet_addresses.begin() + from_index);

    assert(packet_addresses.size() < 8);

    packet_addresses.insert(packet_addresses.begin() + to_index, address);

    return true;
}

APRS_ROUTER_INLINE bool try_truncate_address_range(std::vector<segment>& packet_addresses, size_t start_index, size_t end_index)
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

    segment address = packet_addresses[end_index];
    
    packet_addresses.erase(packet_addresses.begin() + start_index, packet_addresses.begin() + end_index + 1);

    assert(packet_addresses.size() < 8);
    
    packet_addresses.insert(packet_addresses.begin() + start_index, address);

    return true;
}

APRS_ROUTER_INLINE bool try_n_N_route(std::vector<segment>& packet_addresses, const std::vector<segment>& router_n_N_addresses, const std::string& router_address, routing_option options)
{
    // Route an ADDRESSn-N address. 
    //
    // Route a packet through a routing "path" that matches an address with the n-N format (e.g., WIDE2-1).
    //
    // Pick the very first matching n-N address among the packet's addresses:
    //
    //   1. Decrement the 'N' part of the ADDRESSn-N address to indicate one less hop remaining.
    //
    //   2. Insert the router's address in front of the matched address to mark it as the next hop.
    //
    //      2.a. Mark the router's address as "used" with an asterix (*), if N > 0
    //
    //   3. If the N part of the ADDRESSn-N is decremented to 0, then:
    //
    //      3.a. If "substitute_complete_hops" is set, remove the ADDRESSn-N where N is 0
    //
    //      3.b. If "substitute_complete_hops" is not set, mark the ADDRESSn-N as "used"
    //
    //   4. Create and update the routing result to reflect these changes.
    //
    // Example:
    //
    // Router address:  DIGI
    //
    // Router path:     WIDE1-1,WIDE2-1
    //                  ~~~~~~~~~~~~~~~
    //                  router_n_N_addresses
    //
    // Original packet: N0CALL>APRS,CALL*,WIDE1-1,WIDE2-1:data
    //                              ~~~~~~~~~~~~~~~~~~~~~
    //                              packet_addresses
    //
    // After step 1:    N0CALL>APRS,CALL*,WIDE1,WIDE2-1:data
    // After step 2:    N0CALL>APRS,CALL*,DIGI,WIDE1,WIDE2-1:data
    // After step 2.a:  N0CALL>APRS,CALL*,DIGI,WIDE1,WIDE2-1:data
    // After step 3.a:  N0CALL>APRS,CALL,DIGI*,WIDE2-1:data
    // After step 3.b:  N0CALL>APRS,CALL,DIGI,WIDE1*,WIDE2-1:data

    auto unused_address_index_pair = find_first_unused_n_N_address_index(packet_addresses, router_n_N_addresses, options);

    if (!unused_address_index_pair)
    {
        return false;
    }

    auto [address_n_N_index, router_n_N_index] = unused_address_index_pair.value();

    assert(address_n_N_index < packet_addresses.size());
    assert(router_n_N_index < router_address.size());

    if (try_trap_n_N_route(packet_addresses[address_n_N_index], router_n_N_addresses[router_n_N_index], router_address, options))
    {
        return true;
    }

    try_n_N_route_no_trap(packet_addresses, address_n_N_index, router_address, options);
    
    return true;
}

APRS_ROUTER_INLINE bool try_n_N_route_no_trap(std::vector<segment>& packet_addresses, size_t& packet_n_N_address_index, std::string_view router_address, routing_option options)
{
    assert(packet_n_N_address_index < packet_addresses.size());

    bool substitute_zero_hops = enum_has_flag(options, routing_option::substitute_complete_hops);    

    segment& n_N_address = packet_addresses[packet_n_N_address_index];

    assert(n_N_address.n > 0);

    try_decrement_address_n_N(n_N_address);

    // If we are in a position which will require us to insert more than 8 addresses
    // just return, the only thing we can do is increment the counter
    
    if (try_complete_n_N_route(packet_addresses, n_N_address, substitute_zero_hops))
    {
        return true;
    }

    if (substitute_zero_hops && n_N_address.N == 0)
    {
        try_substitute_complete_n_N_address(packet_addresses, packet_n_N_address_index, router_address);
        return true;
    }

    try_insert_n_N_route(packet_addresses, packet_n_N_address_index, router_address, substitute_zero_hops);

    return true;
}

APRS_ROUTER_INLINE bool try_complete_n_N_route(std::vector<segment>& packet_addresses, segment& n_N_address, bool substitute_zero_hops)
{
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
        // The n-N address has no remaining hops, but we cannot substitute it
        // with our router's address because "substitute_zero_hops" is unset
        // we also have more than 8 addresses, so just mark the completed address as "set"
        if (!substitute_zero_hops && n_N_address.N == 0)
        {
            set_address_as_used(packet_addresses, n_N_address); 
            return true;
        }

        // The n-N address has remaining hops, and we have more than 8 addresses
        // We've decremented it earlier (above), don't change any used flag,
        // just exit as we are done
        if (!substitute_zero_hops || n_N_address.N > 0)
        {
            return true;
        }
    }

    return false;
}

APRS_ROUTER_INLINE bool try_insert_n_N_route(std::vector<segment>& packet_addresses, size_t& packet_n_N_address_index, std::string_view router_address, bool substitute_zero_hops)
{
    // Insert router's address in front of the n-N address:
    // 
    // Original packet: FROM>TO,CALL,WIDE2-1:data
    //                               ~~~~~~~
    //
    // Updated packet: FROM>TO,CALL,DIGI*,WIDE2-1:data
    //                              ~~~~~
    //                              Insert and mark the router's address as used
    //                              n-N address is left unchanged by this function

    assert(packet_n_N_address_index < packet_addresses.size());
    assert(packet_addresses.size() < 8);

    segment& n_N_address = packet_addresses[packet_n_N_address_index];

    segment new_address;
    new_address.text = router_address;    
    new_address.type = segment_type::other;

    if (substitute_zero_hops || n_N_address.N > 0)
    {
        set_address_as_used(packet_addresses, new_address);
    }
    else
    {
        set_address_as_used(packet_addresses, n_N_address);
    }

    packet_addresses.insert(packet_addresses.begin() + packet_n_N_address_index, new_address);

    packet_n_N_address_index++;    

    update_addresses_index(packet_addresses);

    return true;
}

APRS_ROUTER_INLINE bool try_trap_n_N_route(segment& packet_n_N_address, const segment& router_n_N_address, std::string_view router_address, routing_option options)
{
    // Replace an excessive hop with the router's address
    // to stop the packet from harming the network
    //
    // packet_n_N_address - is the address inside the packet's path
    // router_n_N_address - is the address inside the router's path, matching the packet address
    //
    // Example:
    //
    // Router path: WIDE2-2,WIDE7-2
    //                      ~~~~~~~ 
    //                      router_n_N_address
    //
    // Input:  FROM>TO,WIDE7-7:data
    //                 ~~~~~~~
    //                 packet_n_N_address
    //
    // Output: FROM>TO,DIGI*:data
    //                 ~~~~~
    //                 router_address

    bool trap_excessive_hops = enum_has_flag(options, routing_option::trap_excessive_hops);

    if (trap_excessive_hops)
    {
        if (router_n_N_address.N > 0 && packet_n_N_address.N > router_n_N_address.N)
        {
            packet_n_N_address.text = router_address;
            packet_n_N_address.mark = true;
            packet_n_N_address.n = 0;
            packet_n_N_address.N = 0;

            return true;
        }
    }

    return false;
}

APRS_ROUTER_INLINE bool try_substitute_complete_n_N_address(std::vector<segment>& packet_addresses, size_t packet_n_N_address_index, std::string_view router_address)
{
    assert(packet_n_N_address_index < packet_addresses.size());

    // If the last n-N hop has been exhausted, replace the hop with the routr's address
    //
    // Example:
    // Input:  FROM>TO,WIDE1-1,WIDE2-1:data
    // Output: FROM>TO,DIGI*,WIDE2-1:data - replace WIDE1 with DIGI

    segment& address = packet_addresses[packet_n_N_address_index];

    if (address.N == 0)
    {
        address.text = router_address;
        address.N = 0;
        address.n = 0;
        address.type = segment_type::other;

        set_address_as_used(packet_addresses, address);

        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool try_decrement_address_n_N(std::vector<segment>& packet_addresses, size_t index)
{
    // Decrements an n-N address, ex: WIDE2-2 becomes WIDE2-1

    assert(index < packet_addresses.size());
    segment& s = packet_addresses[index];
    return try_decrement_address_n_N(s);
}

APRS_ROUTER_INLINE bool try_decrement_address_n_N(segment& s)
{
    // Decrements an n-N address, ex: WIDE2-2 becomes WIDE2-1

    if (s.N > 0)
    {
        s.N--;
        return true;
    }

    return false;
}

#endif

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END
