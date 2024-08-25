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

#pragma once

#include <string>
#include <string_view>
#include <vector>

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
#ifndef APRS_ROUTER_DETAIL_NAMESPACE_END
#define APRS_ROUTER_DETAIL_NAMESPACE_END }
#endif
#ifndef APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE
#define APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE APRS_ROUTER_APRS_DETAIL_NAMESPACE::
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


enum class routing_option
{
    preempt_explicit_self,   // Preemptively move our callsign in front and repeat without waiting for other stations to repeat.
    replace_alias_on_expiry, // Replace the alias with our callsign when the n-N alias is decremented to 0.
    substitute_excessive_hops,   // Replace a harmful alias with our callsign to prevent network issues (e.g., ALIAS7-7).
    reject_excessive_hops    // Reject the packet if the alias has excessive hops (e.g., ALIAS7-7).
};

struct router_settings
{
    std::string callsign;
    std::vector<std::string> aliases = { "WIDE1", "WIDE2", "TRACE1", "TRACE2", "RELAY1", "RELAY2" };
    int max_N = 3;
    std::vector<routing_option> options;
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
    modify,
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
    tcpip,
    nogate,
    rfonly,
    igatecall,
    q
};

struct segment
{
    std::string text;
    int n = 0;
    int N = 0;
    bool mark = false;
    segment_type type = segment_type::other;
    q_construct q = q_construct::none;
    int index = -1;
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
APRS_ROUTER_INLINE q_construct parse_q_construct(std::string_view input);
APRS_ROUTER_INLINE segment_type parse_segment_type(const std::string& text);
APRS_ROUTER_INLINE bool try_parse_segment(std::string_view path, segment& s);
APRS_ROUTER_INLINE std::vector<segment> parse_router_aliases(const router_settings& settings);
APRS_ROUTER_INLINE bool is_packet_self_routing(const struct packet& packet, const std::string& callsign);
APRS_ROUTER_INLINE bool has_packet_routing_ended(const std::vector<segment>& addresses, int last_digipeated_segment);
APRS_ROUTER_INLINE bool has_packet_already_been_routed(const std::vector<segment>& addresses, int last_digipeated_segment, const std::string& callsign);
APRS_ROUTER_INLINE std::vector<segment> parse_packet_addresses(const struct packet& packet);
APRS_ROUTER_INLINE void init_routing_result(const struct packet& packet, routing_result& result);
APRS_ROUTER_INLINE bool aliases_contains_address(const std::vector<segment>& aliases, const segment& address);
APRS_ROUTER_INLINE int find_first_matching_address_alias_index(const std::vector<segment>& addresses, const std::vector<segment>& aliases);
APRS_ROUTER_INLINE int find_last_digipeated_address_index(const std::vector<segment>& addresses);
APRS_ROUTER_INLINE int find_router_callsign_index(const std::vector<segment>& addresses, int offset, const std::string& callsign);
APRS_ROUTER_INLINE bool create_routing_result(const packet& p, const std::vector<segment>& addresses, routing_result& result);
APRS_ROUTER_INLINE bool create_routing_result(routing_state state, routing_result& result);
APRS_ROUTER_INLINE void unset_all_digipeated_addresses(std::vector<segment>& addresses, int offset, int count);
APRS_ROUTER_INLINE void mark_address_as_digipeated(std::vector<segment>& addresses, int index);
APRS_ROUTER_INLINE int find_matching_callsign(const std::vector<segment>& addresses, int index, const std::string& callsign);
APRS_ROUTER_INLINE void update_addresses_index(std::vector<segment>& addresses);
APRS_ROUTER_INLINE bool insert_route(std::vector<segment>& addresses, int& index, const std::string& callsign);
APRS_ROUTER_INLINE void decrement_address_n_N(std::vector<segment>& addresses, int index);
APRS_ROUTER_INLINE void decrement_address_n_N(segment& s);
APRS_ROUTER_INLINE void normalize_digipeated_path(std::vector<segment>& addresses, const std::string& callsign, int alias_index);
APRS_ROUTER_INLINE bool may_have_callsigns_ahead(std::vector<segment>& addresses, int offset, int count, const std::vector<segment>& aliases);

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
    // Routing algorithm
    //
    //  1) If the packe source address is the same as the digipeater's callsign, return false
    //  2) Find the first unused (unmarked) address in the packet path, if none is found return false
    //  3) 

    APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE init_routing_result(packet, result);

    // The router's callsign.
    const std::string& callsign = settings.callsign;

    // Reject packets that originate from the digipeater itself
    // A digipeater should not digipeat packets it originally sent.
    // For instance, if the digipeater's callsign is "DIGI", it should reject packets like "DIGI>TO,WIDE2-1:data"
    // since the source callsign matches the digipeater's callsign.
    if (APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE is_packet_self_routing(packet, callsign))
    {
        APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE create_routing_result(routing_state::cannot_route_self, result);
        return false;
    }

    // Parse all digipeater aliases into segments
    // so that we can make direct comparisons with the packet segments.
    std::vector<APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE segment> aliases = APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE parse_router_aliases(settings);

    // Process packet addresses and split them into queryable
    // segments.
    std::vector<APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE segment> addresses = APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE parse_packet_addresses(packet);

    // Retrieve the last address that has been digipeated.
    // For example, if the packet is "FROM>TO,CALL*,TEST*,ADDRESS*,WIDE1-2:data",
    // the last digipeated address will be "ADDRESS".
    int last_digipeated_address_index = APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE find_last_digipeated_address_index(addresses);    

    // If all possible digipeated addresses have been handled, no further action is needed.
    // The asterisk (*) marks the last address that was digipeated,
    // indicating that all addresses preceding it have already been processed.
    // For example, in the packet "FROM>TO,ADDRESS,WIDE1*:data",
    // "WIDE1*" shows that all addresses up to and including "WIDE1*" have been digipeated.
    if (APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE has_packet_routing_ended(addresses, last_digipeated_address_index))
    {
        APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE create_routing_result(routing_state::not_routed, result);
        return false;
    }

    // If packet has been already routed by this router, exit
    //
    // Example:
    // Input:  N0CALL>APRS,OH7RDA*,WIDE1-1:data
    // Output: N0CALL>APRS,OH7RDA*,WIDE1-1:data
    if (APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE has_packet_already_been_routed(addresses, last_digipeated_address_index, callsign))
    {
        APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE create_routing_result(routing_state::already_routed, result);
        return false;
    }

    // If explicitly routing a packet through the digipeater
    // find the digipeater's address in the packet and mark it as digipeated (*)
    // Also unmark all the previously digipeated addresses.
    // 
    // Example:
    // Input:  FROM>TO,CALL1*,DIGI,CALL2:data
    // Output: FROM>TO,CALL1,DIGI*,CALL2:data
    int router_callsign_index = APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE find_matching_callsign(addresses, last_digipeated_address_index, callsign);
    bool has_other_callsigns_ahead = APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE may_have_callsigns_ahead(addresses, router_callsign_index, addresses.size(), aliases);
    if (router_callsign_index != -1)
    {
        if (!has_other_callsigns_ahead)
        {
            APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE mark_address_as_digipeated(addresses, router_callsign_index);
            APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE create_routing_result(packet, addresses, result);
            return true;
        }
        else
        {
            APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE create_routing_result(routing_state::not_routed, result);
            return false;
        }
    }

    // Route an ALIASn-N address. 
    // Route a packet through an address that matches an alias with a specific format (e.g., ALIASn-N).
    // Pick the very first matching alias among the packet's addresses:
    //   1. Decrement the 'N' part of the ALIASn-N address to indicate one less hop remaining.
    //   2. Insert the digipeater's callsign in front of the matched alias to mark it as the next hop.
    //      Mark the digipeater's callsign with an asterix (*) to mark it as "digipeated"
    //   3. Normalize the remaining path by marking the digipeater's callsign and clearing any previous marks.
    //      If at step 1) the N-th part of the ALIAS is decremented to 0
    //        a) Replace the ALIAS with the digipeater's callsign and mark it as "digipeated"
    //        b) Do not insert the digipeater's callsign in from of the the replaced ALIAS
    //   4. Create and update the routing result to reflect these changes.
    int first_matching_address_alias_index = APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE find_first_matching_address_alias_index(addresses, aliases);
    if (first_matching_address_alias_index != -1)
    {
        APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE insert_route(addresses, first_matching_address_alias_index, callsign);
        APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE normalize_digipeated_path(addresses, callsign, first_matching_address_alias_index);
        APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE create_routing_result(packet, addresses, result);
        return true;
    }

    APRS_ROUTER_DETAIL_NAMESPACE_REFERENCE create_routing_result(routing_state::not_routed, result);

    return false;
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

APRS_ROUTER_INLINE q_construct parse_q_construct(std::string_view input)
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

    auto it = q_construct_map.find(std::string(input));
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
        {"NOGATE", segment_type::nogate},
        {"RFONLY", segment_type::rfonly},
        {"IGATECALL", segment_type::igatecall}
    };

    auto it = lookup_table.find(text);
    return (it != lookup_table.end()) ? it->second : segment_type::other;
}

APRS_ROUTER_INLINE bool try_parse_segment(std::string_view path, segment& s)
{
    s.text = path;

    q_construct q = parse_q_construct(path);
    if (q != q_construct::none)
    {
        s.n = 0;
        s.N = 0;
        s.type = segment_type::q;
        s.q = q;
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

    // Never fail. Failure to parse a segment in most cases means that n-N will not be set.
    return true;
}

APRS_ROUTER_INLINE std::vector<segment> parse_router_aliases(const router_settings& settings)
{
    std::vector<segment> aliases;
    for (const auto& alias : settings.aliases)
    {
        segment s;
        try_parse_segment(alias, s);
        aliases.push_back(s);
    }
    return aliases;
}

APRS_ROUTER_INLINE bool is_packet_self_routing(const struct packet& packet, const std::string& callsign)
{
    return packet.from == callsign;
}

APRS_ROUTER_INLINE bool has_packet_routing_ended(const std::vector<segment>& addresses, int last_digipeated_segment)
{
    return (last_digipeated_segment == addresses.size() - 1);
}

APRS_ROUTER_INLINE bool has_packet_already_been_routed(const std::vector<segment>& addresses, int last_digipeated_segment, const std::string& callsign)
{
    return (addresses[last_digipeated_segment].text == callsign && addresses[last_digipeated_segment].mark);
}

APRS_ROUTER_INLINE std::vector<segment> parse_packet_addresses(const struct packet& packet)
{
    std::vector<segment> segments;
    int i = 0;
    for (const auto& path : packet.path)
    {
        segment s;
        s.index = i;
        try_parse_segment(path, s);
        segments.push_back(s);
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

APRS_ROUTER_INLINE bool aliases_contains_address(const std::vector<segment>& aliases, const segment& address)
{
    for (const auto& a : aliases)
    {
        if (address.text == a.text && address.n == a.n)
        {
            return true;
        }
    }
    return false;
}

APRS_ROUTER_INLINE int find_first_matching_address_alias_index(const std::vector<segment>& addresses, const std::vector<segment>& aliases)
{
    for (const auto& address : addresses)
    {
        for (const auto& alias : aliases)
        {
            if (address.text == alias.text && address.n == alias.n && address.N > 0)
            {
                return address.index;
            }
        }
    }
    return -1;
}

APRS_ROUTER_INLINE int find_last_digipeated_address_index(const std::vector<segment>& addresses)
{
    for (int i = addresses.size() - 1; i >= 0; i--)
    {
        if (addresses[i].mark)
        {
            return i;
        }
    }
    return -1;
}

APRS_ROUTER_INLINE int find_router_callsign_index(const std::vector<segment>& addresses, int offset, const std::string& callsign)
{
    for (int i = offset; i < addresses.size(); i++)
    {
        if (addresses[i].text == callsign)
        {
            return i;
        }
    }
    return -1;
}

APRS_ROUTER_INLINE bool create_routing_result(const packet& p, const std::vector<segment>& addresses, routing_result& result)
{
    packet routed_packet = p;

    routed_packet.path.clear();

    for (auto segment : addresses)
    {
        routed_packet.path.push_back(to_string(segment));
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

APRS_ROUTER_INLINE void unset_all_digipeated_addresses(std::vector<segment>& addresses, int offset, int count)
{
    for (int i = offset, n = 0; i < addresses.size() && n < count; i++, n++)
    {
        addresses[i].mark = false;
    }
}

APRS_ROUTER_INLINE void mark_address_as_digipeated(std::vector<segment>& addresses, int index)
{
    unset_all_digipeated_addresses(addresses, 0, addresses.size());
    addresses[index].mark = true;
}

APRS_ROUTER_INLINE int find_matching_callsign(const std::vector<segment>& addresses, int index, const std::string& callsign)
{
    int callsign_index = find_router_callsign_index(
        addresses,
        index == -1 ? 0 : index, // there might be no digipeated address
        callsign);

    if (callsign_index != -1 && !addresses[callsign_index].mark)
    {
        return callsign_index;
    }

    return -1;
}

APRS_ROUTER_INLINE void update_addresses_index(std::vector<segment>& addresses)
{
    for (int i = 0; i < addresses.size(); i++)
    {
        addresses[i].index = i;
    }
}

APRS_ROUTER_INLINE bool insert_route(std::vector<segment>& addresses, int& index, const std::string& callsign)
{
    unset_all_digipeated_addresses(addresses, 0, addresses.size());

    decrement_address_n_N(addresses, index);

    // if address count is 8 we cannot insert as it will result in more than
    // 8 addresses in the path, instead we need to decrement and mark 
    // the address before the current one as digipeated
    //
    // Example:
    // Input:  FROM>TO,A,B,C,D,E,F,G*,WIDE2-2:data
    // Output: FROM>TO,A,B,C,D,E,F,G*,WIDE2-1:data
    if (addresses.size() > 7 && addresses[index].N > 1)
    {
        addresses[index - 1].mark = true;
        return false;
    }

    // Insert router's callsign in front of the ALIASn-N
    //
    // Example:
    // Input:  FROM>TO,CALL,WIDE2-1:data
    // Output: FROM>TO,CALL,DIGI*,WIDE2-1:data

    segment s;
    s.text = callsign;
    s.mark = true;
    s.type = segment_type::other;

    addresses.insert(addresses.begin() + index, s);

    index++;

    update_addresses_index(addresses);

    return true;
}

APRS_ROUTER_INLINE void decrement_address_n_N(std::vector<segment>& addresses, int index)
{
    segment& s = addresses[index];
    if (s.N > 0)
    {
        s.N--;
    }
}

APRS_ROUTER_INLINE void decrement_address_n_N(segment& s)
{
    if (s.N > 0)
    {
        s.N--;
    }
}

APRS_ROUTER_INLINE void normalize_digipeated_path(std::vector<segment>& addresses, const std::string& callsign, int alias_index)
{
    segment& s = addresses[alias_index];
    if (s.N == 0)
    {
        unset_all_digipeated_addresses(addresses, 0, addresses.size());
        s.text = callsign;
        s.N = 0;
        s.n = 0;
        s.mark = true;
        s.type = segment_type::other;
        addresses.erase(addresses.begin() + alias_index - 1);
        update_addresses_index(addresses);
    }
}

APRS_ROUTER_INLINE bool may_have_callsigns_ahead(std::vector<segment>& addresses, int offset, int count, const std::vector<segment>& aliases)
{
    if (offset <= 0)
    {
        return false;
    }

    for (int i = offset - 1, n = 0; i >= 0 && n < count; i--, n++)
    {
        // If any of the addresses ahead are aliases or q constructs
        // Then ignore them and route with our callsign
        //
        // Examples:
        // Input:  FROM>TO,WIDE1-1,WIDE2-1,DIGI:data
        // Output: FROM>TO,WIDE1-1,WIDE2-1,DIGI*:data - we can repeat the packet with the DIGI callsign
        //
        // Input:  FROM>TO,WIDE1-1,WIDE2-1,DIGI:data
        // Output: FROM>TO,CALL,WIDE2-1,DIGI*:data - don't repeat, CALL likely a callsign

        if (!(aliases_contains_address(aliases, addresses[i]) || addresses[i].type == segment_type::q ||
            addresses[i].type == segment_type::wide || addresses[i].type == segment_type::trace ||
            addresses[i].type == segment_type::relay || addresses[i].type == segment_type::temp ||
            addresses[i].type == segment_type::nogate || addresses[i].type == segment_type::igatecall ||
            addresses[i].type == segment_type::tcpip || addresses[i].type == segment_type::rfonly ||
            addresses[i].mark))
        {
            return true;
        }
    }
    return false;
}

#endif

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END
