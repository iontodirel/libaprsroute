// **************************************************************** //
// libaprsroute - APRS header only routing library                  // 
// Version 0.1.0                                                    //
// https://github.com/iontodirel/libaprsroute                       //
// Copyright (c) 2024 - 2025 Ion Todirel                            //
// **************************************************************** //
//
// aprsroute.hpp
//
// MIT License
//
// Copyright (c) 2024 - 2025 Ion Todirel
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
//   - APRS digipeaters v2: https://github.com/wb2osz/direwolf-doc/blob/main/APRS-Digipeaters.pdf
//   - How APRS paths work: https://blog.aprs.fi/2020/02/how-aprs-paths-work.html
//   - APRS Digipeating and Path Selection: http://wa8lmf.net/DigiPaths
//   - Examining Ambiguities in the Automatic Packet Reporting System: https://digitalcommons.calpoly.edu/theses/1341
//   - Digi Question (viscous QRM): https://web.archive.org/web/20151022062516/https://tapr.org/pipermail/aprssig/2013-March/041554.html

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <iterator>
#include <cassert>
#include <optional>
#include <array>
#include <algorithm>
#include <charconv>
#include <memory_resource>

// This header only library can be compiled in a TU and shared between TUs
// to minimize compilation time, by defining the APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY preprocessor directive.
//
// Example:
//
// In the TU where the library is compiled:
//
// #define APRS_ROUTER_INLINE
// #include "aprsroute.hpp"
//
// In the header where the library is included:
//
// #define APRS_ROUTER_INLINE
// #define APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY
// #include "aprsroute.hpp"

#ifndef APRS_ROUTER_NAMESPACE
#define APRS_ROUTER_NAMESPACE aprs::router
#endif
#ifndef APRS_ROUTER_NAMESPACE_BEGIN
#define APRS_ROUTER_NAMESPACE_BEGIN namespace APRS_ROUTER_NAMESPACE {
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
#ifndef APRS_ROUTER_DETAIL_NAMESPACE_USE
#define APRS_ROUTER_DETAIL_NAMESPACE_USE using namespace APRS_ROUTER_APRS_DETAIL_NAMESPACE;
#endif
#ifndef APRS_ROUTER_DETAIL_NAMESPACE_END
#define APRS_ROUTER_DETAIL_NAMESPACE_END }
#endif
#ifndef APRS_ROUTER_ENABLE_PACKET_SUPPORT
#define APRS_ROUTER_ENABLE_PACKET_SUPPORT true
#endif
#ifndef APRS_ROUTER_PACKET_NAMESPACE
#define APRS_ROUTER_PACKET_NAMESPACE APRS_ROUTER_NAMESPACE
#endif
#ifndef APRS_ROUTER_PACKET_NAMESPACE_BEGIN
#define APRS_ROUTER_PACKET_NAMESPACE_BEGIN namespace APRS_ROUTER_PACKET_NAMESPACE {
#endif
#ifndef APRS_ROUTER_PACKET_NAMESPACE_END
#define APRS_ROUTER_PACKET_NAMESPACE_END }
#endif
#ifndef APRS_ROUTER_PACKET_NAMESPACE_REFERENCE
#define APRS_ROUTER_PACKET_NAMESPACE_REFERENCE APRS_ROUTER_PACKET_NAMESPACE ::
#endif
#ifdef APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY
// Intentionally left empty
#endif
// Configure the library whether it should use PMR and PMR data structures or not.
#ifndef APRS_ROUTER_USE_PMR
#define APRS_ROUTER_USE_PMR 0
#endif

// **************************************************************** //
//                                                                  //
//                                                                  //
// PUBLIC DECLARATIONS                                              //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_PACKET_NAMESPACE_BEGIN

// Built-in packet type support can be disabled by using the APRS_ROUTER_ENABLE_PACKET_SUPPORT preprocessor directive.
// If packet support is disabled, an external packet implementation must be provided.
// This extensbility is useful for sharing the same packet type among various different libraries.
//
// Example:
//
// #define APRS_ROUTER_ENABLE_PACKET_SUPPORT false
// #define APRS_ROUTER_PACKET_NAMESPACE aprs
// #include "aprsroute.hpp"
//
// If the packet type is not in a namespace, also define the APRS_ROUTER_PACKET_NAMESPACE_REFERENCE
//
// Example:
//
// #define APRS_ROUTER_ENABLE_PACKET_SUPPORT false
// #define APRS_ROUTER_PACKET_NAMESPACE
// #define APRS_ROUTER_PACKET_NAMESPACE_REFERENCE
// #include "aprsroute.hpp"
//
// The test external_packet_test provides an example of how to use the external packet type.

#if APRS_ROUTER_ENABLE_PACKET_SUPPORT

struct packet
{
    packet() = default;
    packet(const packet& other) = default;
    packet& operator=(const packet& other) = default;
    packet(const std::string& from, const std::string& to, const std::vector<std::string>& path, const std::string& data);
    packet(const char* packet_string);
    packet(const std::string& packet_string);
    operator std::string() const;

    std::string from;
    std::string to;
    std::vector<std::string> path;
    std::string data;
};

#endif

typedef APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet packet_type;

APRS_ROUTER_PACKET_NAMESPACE_END

APRS_ROUTER_NAMESPACE_BEGIN

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
// Enables preemptive routing for explicitly routed packets.
// With this option, our matched address is moved to the front of the route.
//
// This packet: N0CALL>APRS,CALLA,CALLB*,CALLC,DIGI,CALLD,CALLE,CALLF:data
//                                             ~~~~
// Will be routed as: N0CALL>APRS,CALLA,CALLB,DIGI*,CALLC,CALLD,CALLE,CALLF:data
//                                            ~~~~~
// ----------------
// preempt_truncate
// ----------------
//
// Enables preemptive routing for explicitly routed packets.
// With this option, our address is moved behind the last used address, and all other addresses are erased.
// This is the same as Direwolf's "TRACE" routing option.
//
// This packet: N0CALL>APRS,CALLA,CALLB*,CALLC,DIGI,CALLD,CALLE,CALLF:data
//                                             ~~~~
// Will be routed as: N0CALL>APRS,CALLA,CALLB,DIGI*,CALLD,CALLE,CALLF:data
//                                            ~~~~~
// ------------
// preempt_drop
// ------------
//
// Enables preemptive routing for explicitly routed packets.
// With this option, all other addresses in front of us are erased.
//
// This packet: N0CALL>APRS,CALLA,CALLB*,CALLC,DIGI,CALLD,CALLE,CALLF:data
//                                             ~~~~
// Will be routed as: N0CALL>APRS,DIGI*,CALLD,CALLE,CALLF:data
//                                ~~~~~
// ------------
// preempt_mark
// ------------
//
// Enables preemptive routing for explicitly routed packets.
// With this option, all other addresses in front of us are ignored.
//
// This packet: N0CALL>APRS,CALLA,CALLB*,CALLC,DIGI,CALLD,CALLE,CALLF:data
//                                             ~~~~
// Will be routed as: N0CALL>APRS,CALLA,CALLB,CALLC,DIGI*,CALLD,CALLE,CALLF:data
//                                                  ~~~~~
// ------------------------
// substitute_complete_n_N_address
// ------------------------
//
// Replace exaused hops. Similar to substitute_explicit_address but for n-N routing.
//
// This packet: N0CALL>APRS,CALLA,WIDE1-1,WIDE2-2:data
//                                ~~~~~~~
// Will be routed as: N0CALL>APRS,CALLA,DIGI*,WIDE2-2:data
//                                      ~~~~~
// ------------------------
// skip_complete_n_N_address
// ------------------------
//
// Skip complete n-N addresses even if unset. The completed/unset address has to be in the n_N_addresses list.
//
// This packet: N0CALL>APRS,CALLA*,WIDE1,WIDE2-2:data
//                                 ~~~~~
//
// Will be routed as: N0CALL>APRS,CALLA,WIDE1,DIGI*,WIDE2-1:data
//
// ------------------------
// trap_limit_exceeding_n_N_address
// ------------------------
//
// Replace excessive hops.
//
// This packet: N0CALL>APRS,CALLA,WIDE2-2,WIDE3-2:data
//                                ~~~~~~~
// Will be routed as: N0CALL>APRS,CALLA,DIGI*,WIDE3-2:data
//                                      ~~~~~
//
// If the matching address in the n_N_addresses list, has a hop limit, ex: WIDE2-1
//
// ------------------------
// reject_limit_exceeding_n_N_address
// ------------------------
//
// Reject excessive hops.
//
// This packet won't be routed: N0CALL>APRS,CALLA,WIDE2-2,WIDE3-2:data
//                                                ~~~~~~~
//
// If the matching address in the n_N_addresses list, has a hop limit, ex: WIDE2-1
//
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
// Replace an address with the router's address when explicit routing.
// Similar to substitute_complete_n_N_address but for explicit routing.
//
// This packet: N0CALL>APRS,CALLA:data
//                          ~~~~~
// Will be routed as: N0CALL>APRS,DIGI*:data
//                                ~~~~~
// But if the option is not set, it will be routed as: N0CALL>APRS,DIGI,CALLA*:data
//                                                                 ~~~~~~~~~~~
// If the address matches the router's address or one of the explicit addresses,
// I do believe it's a good idea to mirror the n-N routing behavior and add 
// the router's address in front of the explicit address, so we have traceability.

enum class routing_option : int
{
    none = 0,
    route_self = 1,                           // Enable routing packets originating from ourselves.
    preempt_front = 2,                        // Preemptively move our address to the front of the route.
    preempt_truncate = 4,                     // Preemptively move our address behind the last used address and erase all other addresses.
    preempt_drop = 8,                         // Preemptively erase all addresses in front of our address.
    preempt_mark = 16,                        // Preemptively mark our address as used, while leaving the rest of the path as is.
    substitute_complete_n_N_address = 32,     // Replace an PATHn-N address with our address when N is decremented to 0.
    skip_complete_n_N_address = 64,           // Skip complete n-N addresses even if unset (e.g. CALL*,WIDE1,WIDE2-2).
    trap_limit_exceeding_n_N_address = 128,   // Replace a harmful path with our callsign to prevent network issues (e.g., WIDE7-7).
    reject_limit_exceeding_n_N_address = 256, // Reject the packet if the paths has excessive hops (e.g., PATH7-7).
    strict = 512,                             // Don't route if the packet is malformed
    preempt_n_N = 1024,                       // Enables preemptive routing in packets using n-N routing
    substitute_explicit_address = 2048,       // Replace an address with the router's callsign when explicit routing
    recommended = route_self | preempt_front |
                  substitute_complete_n_N_address | trap_limit_exceeding_n_N_address |
                  strict | preempt_n_N |
                  substitute_explicit_address
};

// Routing settings:
//
// address - this is our callsign (or the router's callsign)
//           if implementing a digipeater, this should be set to the digipeater's callsign
//
// explicit_addresses - contains an optional list of aliases
// n_N_addresses - contains an optional list of n-N addresses (generic addresses)
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
//        default setting: none
//
// options - contains a list of options, ex: "preempt_front | trap_limit_exceeding_n_N_address" will enable two options on the router
//
// enable_diagnostics - generates routing diagnostics that can be accessed via the routing_result::actions

struct router_settings
{
    std::string address;
    std::vector<std::string> explicit_addresses;
    std::vector<std::string> n_N_addresses;
    routing_option options = routing_option::none;
    bool enable_diagnostics = false;
};

enum class routing_state
{
    routed,
    not_routed,
    already_routed,
    cannot_route_self
};

enum class routing_action
{
    none,
    insert,    // Adress was inserted, CALLA,CALLC -> CALLA,CALLB,CALLC
    remove,    // Address was removed, CALLA,CALLC -> CALLA
    replace,   // Address was replaced, CALLA,CALLB -> CALLA,CALLC
    unset,     // Address was unset, CALL* -> CALL
    set,       // Address was set, CALL -> CALL*
    decrement, // Address was decremented, WIDE2-2 -> WIDE2-1
    error,     // No action was taken, an error occured
    warn,      // No action was taken, a warning occured
    message    // No action was taken, a message was issued
};

enum class applies_to
{
    none,
    from,
    to,
    path,
    data
};

struct routing_diagnostic
{
    applies_to target = applies_to::none;
    size_t index = 0; // Address index within the packet path
    size_t start = 0; // Address index within the packet string
    size_t end = 0;   // Address index within the packet string
    routing_action type = routing_action::none;
    std::string address;
    std::string message;
};

struct routing_diagnostic_display_entry
{
    std::string message;
    std::string packet_string;
    std::string highlight_string;
};

struct routing_diagnostic_display
{
    std::vector<routing_diagnostic_display_entry> entries;
};

struct routing_result
{
    bool routed = false;
    bool success = false;
    APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet original_packet;
    APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet routed_packet;
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

APRS_ROUTER_PACKET_NAMESPACE_BEGIN

#if APRS_ROUTER_ENABLE_PACKET_SUPPORT

bool operator==(const packet& lhs, const packet& rhs);
bool operator!=(const packet& lhs, const packet& rhs);
size_t hash(const packet& p);
std::string to_string(const packet& p);
bool try_decode_packet(std::string_view packet_string, packet& result);

#endif

APRS_ROUTER_PACKET_NAMESPACE_END

APRS_ROUTER_NAMESPACE_BEGIN

routing_option operator|(routing_option lhs, routing_option rhs);
bool try_parse_routing_option(std::string_view str, routing_option& result);
bool enum_has_flag(routing_option value, routing_option flag);
std::string to_string(const routing_result& result);
std::string to_string(routing_action action);
std::string to_string(applies_to target);
routing_diagnostic_display format(const routing_result& result);
bool try_route_packet(const struct APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& packet, const router_settings& settings, routing_result& result);
bool try_route_packet(std::string_view original_packet_from, std::string_view original_packet_to, const std::vector<std::string>& original_packet_path, const router_settings& settings, std::vector<std::string>& routed_packet_path, enum routing_state& routing_state, std::vector<routing_diagnostic>& routing_actions);

template<class InputIterator, class OutputIterator1, class OutputIterator2>
bool try_route_packet(std::string_view original_packet_from, std::string_view original_packet_to, InputIterator original_packet_path_begin, InputIterator original_packet_path_end, const router_settings& settings, OutputIterator1 routed_packet_path_out, enum routing_state& routing_state, OutputIterator2 routing_actions_out, std::pmr::memory_resource* memory_resource = std::pmr::get_default_resource());

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

enum class address_kind
{
    other,
    trace,     // TRACE
    wide,      // WIDE
    relay,     // RELAY
    echo,      // ECHO
    gate,      // GATE
    temp,      // TEMP
    tcpxx,     // TCPXX  
    tcpip,     // TCPIP
    nogate,    // NOGATE
    rfonly,    // RFONLY
    igatecall, // IGATECALL
    q,         // Q construct: qAC, qAX, qAU, qAo, qAO, qAS, qAr, qAR, qAZ, qAI
    opntrk,    // OPNTRK
    opntrc     // OPNTRC
};

struct address
{
    std::string text;
    int n = 0; // the n component of a n_N address, ex: WIDE1-2, n=1
    int N = 0; // the N component of a n_N address, ex: WIDE1-2, N=2
    int ssid = 0; // the ssid component of an address, ex: CALL-1, ssid=1
    bool mark = false; // whether the address is marked as used, ex: 'CALL*' used, 'CALL' unused
    address_kind kind = address_kind::other;
    q_construct q = q_construct::none;
    size_t index = 0;  // index inside the packet path
    size_t offset = 0; // string offset within the packet
    size_t length = 0; // the string length of the address
};

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END

// Library container and memory configuration.
// The library can used with custom allocators via standard C++17 PMR support.
// This allows the containers to be allocated on the heap or on the stack.
// However, a PMR allocator and supporting containers incure a small performance penalty.
// To this end, the library can be configured to use standard C++17 containers
// or PMR containers. The default is to use PMR containers.

APRS_ROUTER_NAMESPACE_BEGIN

APRS_ROUTER_DETAIL_NAMESPACE_BEGIN

#ifndef APRS_ROUTER_DEFINE_CUSTOM_TYPES

#if APRS_ROUTER_USE_PMR

template<class T>
using internal_vector_t = std::pmr::vector<T>;

#else // !APRS_ROUTER_USE_PMR

template<class T>
using internal_vector_t = std::vector<T>;

#endif // APRS_ROUTER_USE_PMR

#endif // APRS_ROUTER_DEFINE_CUSTOM_TYPES

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_BEGIN

APRS_ROUTER_DETAIL_NAMESPACE_BEGIN

struct route_state
{
#if APRS_ROUTER_USE_PMR
    explicit route_state(std::pmr::memory_resource* memory_resource = std::pmr::get_default_resource()) : packet_path(memory_resource), packet_addresses(memory_resource),
        router_n_N_addresses(memory_resource), router_explicit_addresses(memory_resource), actions(memory_resource), memory_resource(memory_resource)
    {
        assert(memory_resource != nullptr);
    }
#endif

    std::string_view packet_from_address;
    std::string_view packet_to_address;
    internal_vector_t<std::string> packet_path;
    std::optional<std::reference_wrapper<const router_settings>> settings;
    internal_vector_t<address> packet_addresses;
    std::optional<size_t> maybe_last_used_address_index;
    std::optional<size_t> maybe_router_address_index;
    address router_address;
    internal_vector_t<address> router_n_N_addresses;
    internal_vector_t<address> router_explicit_addresses;
    internal_vector_t<routing_diagnostic> actions;
    bool is_path_based_routing = false;
    size_t unused_address_index = 0;
#if APRS_ROUTER_USE_PMR
    std::pmr::memory_resource* memory_resource = std::pmr::get_default_resource();
#endif
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

bool try_explicit_or_n_N_route(route_state& state, routing_state& result);
bool is_explicit_routing(bool is_routing_self, std::optional<size_t> maybe_router_address_index, routing_option options);
bool is_explicit_routing(bool is_routing_self, const route_state& state);
bool try_explicit_route(route_state& state);
bool try_explicit_basic_route(route_state& state, size_t set_address_index);
bool try_preempt_explicit_route(route_state& state);
bool try_preempt_transform_explicit_route(route_state& state);
bool try_n_N_route(route_state& state);
bool try_n_N_route_no_trap(route_state& state, size_t packet_n_N_address_index);
bool try_complete_n_N_route(route_state& state, address& n_N_address, bool substitute_zero_hops);
bool try_insert_n_N_route(route_state& state, size_t& packet_n_N_address_index);
bool try_trap_n_N_route(route_state& state, address& packet_n_N_address, const address& router_n_N_address);

bool try_route_packet_by_index(const struct routing_result& routing_result, APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& result);
bool try_route_packet_by_start_end(const struct routing_result& routing_result, APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& result);

void init_routing_result(const struct APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& packet, routing_result& result);
template <class OutputIterator> bool create_routing_ended_routing(const route_state& state, enum routing_state& routing_state, OutputIterator routing_actions_out);
bool create_routing_ended_routing(const route_state& state, enum routing_state& routing_state, internal_vector_t<routing_diagnostic>& routing_actions);
template <class OutputIterator> bool create_routed_by_us_routing(const route_state& state, enum routing_state& routing_state, OutputIterator routing_actions_out);
bool create_routed_by_us_routing(const route_state& state, enum routing_state& routing_state, internal_vector_t<routing_diagnostic>& routing_actions);
template <class OutputIterator1, class OutputIterator2> bool create_routed_routing(route_state& state, OutputIterator1 routed_packet_path_out, OutputIterator2 routing_actions_out);
bool create_routed_routing(route_state& state, internal_vector_t<std::string>& routed_packet_path, internal_vector_t<routing_diagnostic>& routing_actions);

bool push_routing_ended_diagnostic(const address& address, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool push_routed_by_us_diagnostic(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool push_address_set_diagnostic(const internal_vector_t<address>& packet_addresses, size_t set_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool push_address_unset_diagnostic(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_set_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool push_address_replaced_diagnostic(const internal_vector_t<address>& packet_addresses, size_t set_address_index, std::string_view new_address, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool push_address_decremented_diagnostic(address& address, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool push_address_inserted_diagnostic(const internal_vector_t<address>& packet_addresses, size_t insert_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool push_address_removed_diagnostic(const internal_vector_t<address>& packet_addresses, size_t remove_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool create_address_move_diagnostic(const internal_vector_t<address>& packet_addresses, size_t from_index, size_t to_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);
bool create_truncate_address_range_diagnostic(const internal_vector_t<address>& packet_addresses, size_t from_index, size_t to_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d);

std::string create_display_name_diagnostic(const routing_diagnostic_display_entry& line);
routing_diagnostic_display_entry create_diagnostic_print_line(const routing_diagnostic& diag, const APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& routed_packet);

bool operator==(const address& lhs, const address& rhs);
bool operator!=(const address& lhs, const address& rhs);
std::string to_string(const struct address& address);
bool equal_addresses_ignore_mark(const struct address& lhs, const struct address& rhs);
q_construct parse_q_construct(std::string_view input);
address_kind parse_address_kind(std::string_view text);
bool try_parse_address(std::string_view address_string, address& result);
bool try_parse_n_N_address(std::string_view address_string, struct address& address);
bool try_parse_address_with_ssid(std::string_view address_string, struct address& address);
bool try_parse_address(std::string_view address, std::string& address_no_ssid, int& ssid);
bool try_parse_address_with_used_flag(std::string_view address, std::string& address_no_ssid, int& ssid);
bool try_parse_address_with_used_flag(std::string_view address, std::string& address_no_ssid, int& ssid, bool& mark);
bool try_parse_int(std::string_view str, int& result);

void init_addresses(route_state& state);
void unset_all_used_addresses(internal_vector_t<address>& packet_addresses, size_t offset, size_t count);
void unset_all_used_addresses(internal_vector_t<address>& packet_addresses, size_t offset, size_t count, std::optional<size_t> maybe_ignore_index);
void set_address_as_used(internal_vector_t<address>& packet_addresses, size_t index);
void set_address_as_used(internal_vector_t<address>& packet_addresses, address& address);
void update_addresses_index(internal_vector_t<address>& addresses);
void set_addresses_offset(std::string_view packet_from_address, std::string_view packet_to_address, internal_vector_t<address>& addresses);
void update_addresses_offset(internal_vector_t<address>& addresses, size_t initial_offset);
void update_addresses_offset(internal_vector_t<address>& addresses);
bool try_insert_address(internal_vector_t<address>& packet_addresses, size_t index, std::string_view inserted_address_string);
void replace_address_with_router_address(struct address& address, const struct address& router_address);
bool try_move_address_to_position(internal_vector_t<address>& packet_addresses, size_t from_index, size_t to_index);
bool try_truncate_address_range(internal_vector_t<address>& packet_addresses, size_t from_index, size_t to_index);
bool try_truncate_empty_addresses(route_state& state);
bool try_substitute_complete_n_N_address(route_state& state, size_t packet_n_N_address_index);
bool try_decrement_n_N_address(address& s);
bool try_decrement_n_N_address(route_state& state, address& s);

std::optional<std::pair<size_t, size_t>> find_first_unused_n_N_address_index(const internal_vector_t<address>& packet_addresses, const internal_vector_t<address>& router_addresses, routing_option options);
std::optional<size_t> find_last_used_address_index(const internal_vector_t<address>& packet_addresses, const internal_vector_t<address>& router_n_N_addresses, routing_option options);
std::optional<size_t> find_router_address_index(const internal_vector_t<address>& packet_addresses, size_t offset, const address& router_address, const internal_vector_t<address>& router_addresses);
std::optional<size_t> find_unused_router_address_index(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, const address& router_address, const internal_vector_t<address>& router_explicit_addresses);
void find_used_addresses(route_state& state);
bool has_address(const internal_vector_t<address>& addresses, size_t offset, struct address address);

bool is_packet_valid(std::string_view packet_from_address, std::string_view packet_to_address, const internal_vector_t<std::string>& packet_path, routing_option options);
bool is_packet_valid(const route_state& state);
bool is_valid_router_address_and_packet(const route_state& state);
bool is_packet_from_us(std::string_view packet_from_address, std::string_view router_address);
bool is_packet_from_us(const route_state& state);
bool is_packet_sent_to_us(std::string_view packet_to_address, std::string_view router_address);
bool is_packet_sent_to_us(const route_state& state);
bool has_packet_routing_ended(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_last_used_address_index);
bool has_packet_routing_ended(const route_state& state);
bool has_packet_been_routed_by_us(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, const address& router_address);
bool has_packet_been_routed_by_us(route_state& state);

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END

// **************************************************************** //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
// PUBLIC DEFINITIONS                                               //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
// **************************************************************** //

// **************************************************************** //
//                                                                  //
//                                                                  //
// PACKET                                                           //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_PACKET_NAMESPACE_BEGIN

#if APRS_ROUTER_ENABLE_PACKET_SUPPORT

#ifndef APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY

APRS_ROUTER_INLINE packet::packet(const std::string& from, const std::string& to, const std::vector<std::string>& path, const std::string& data) : from(from), to(to), path(path), data(data)
{
}

APRS_ROUTER_INLINE packet::packet(const char* packet_string)
{
    bool result = try_decode_packet(packet_string, *this);
    (void)result;
    assert(result);
}

APRS_ROUTER_INLINE packet::packet(const std::string& packet_string)
{
    bool result = try_decode_packet(packet_string, *this);
    (void)result;
    assert(result);
}

APRS_ROUTER_INLINE packet::operator std::string() const
{
    return to_string(*this);
}

APRS_ROUTER_INLINE size_t hash(const struct packet& packet)
{
    size_t result = 17; // Start with a prime number
    result = result * 31 + std::hash<std::string>()(packet.from);
    result = result * 31 + std::hash<std::string>()(packet.to);
    result = result * 31 + std::hash<std::string>()(packet.data);
    return result;
}

APRS_ROUTER_INLINE std::string to_string(const struct packet& packet)
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

APRS_ROUTER_INLINE bool operator==(const packet& lhs, const packet& rhs)
{
    return lhs.from == rhs.from &&
           lhs.to == rhs.to &&
           lhs.path == rhs.path &&
           lhs.data == rhs.data;
}

APRS_ROUTER_INLINE bool operator!=(const packet& lhs, const packet& rhs)
{
    return !(lhs == rhs);
}

APRS_ROUTER_INLINE bool try_decode_packet(std::string_view packet_string, packet& result)
{
    // Parse a packet: N0CALL>APRS,CALLA,CALLB*,CALLC,CALLD,CALLE,CALLF,CALLG:data
    //                 ~~~~~~ ~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~
    //                 from   to   path                                       data
    //
    // This function does the minimum required to parse a packet string.
    // If packet string is invalid, filling of the the packet fields is not guaranteed,
    // e.g. missing data separator ":", or missig "path"

    result.path.clear();

    // Find the from address, and the end of the packet header
    //
    // N0CALL>APRS,CALLA,CALLB*,CALLC,CALLD,CALLE,CALLF,CALLG:data
    //       ~                                               ~
    //       from_end_pos                                    colon_pos
    //
    // If we cannot find the from position, or the end of the header, we fail the parsing

    size_t from_end_pos = packet_string.find('>');

    if (from_end_pos == std::string_view::npos)
    {
        return false;
    }

    size_t colon_pos = packet_string.find(':', from_end_pos);

    if (colon_pos == std::string_view::npos)
    {
        return false;
    }

    result.from = packet_string.substr(0, from_end_pos);

    // Find the 'to' address, and the 'path'
    //
    // N0CALL>APRS,CALLA,CALLB*,CALLC,CALLD,CALLE,CALLF,CALLG:data
    //        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //        to_and_path

    std::string_view to_and_path = packet_string.substr(from_end_pos + 1, colon_pos - from_end_pos - 1);

    size_t comma_pos = to_and_path.find(',');

    // If we cannot find the comma, comma_pos will be set to the largest positive unsigned integer
    // with 'to' containing the remaining string, and with path being empty
    //
    // comma_pos = 18446744073709551615
    // 
    // N0CALL>APRS:data
    //        ~~~~
    //        to

    result.to = to_and_path.substr(0, comma_pos);

    if (comma_pos != std::string_view::npos)
    {
        std::string_view path = to_and_path.substr(comma_pos + 1);

        // Keep consuming the path until we reach the end of the header (colon_pos)
        // We use remove_prefix, which just changes the beginning of the string_view
        // It does not modify the string, nor copy it
        //
        // 1st iteration: CALLA,CALLB*,CALLC,CALLD,CALLE,CALLF,CALLG
        //                ~~~~~
        // 2nd iteration: CALLB*,CALLC,CALLD,CALLE,CALLF,CALLG
        //                ~~~~~~
        // 3rd iteration: CALLC,CALLD,CALLE,CALLF,CALLG
        //                ~~~~~
        // 4th iteration: CALLD,CALLE,CALLF,CALLG
        //                ~~~~~
        // 5th iteration: CALLE,CALLF,CALLG
        //                ~~~~~
        // 6th iteration: CALLF,CALLG
        //                ~~~~~
        // 7th iteration: CALLG
        //                ~~~~~

        while (!path.empty())
        {
            comma_pos = path.find(',');

            std::string_view address = path.substr(0, comma_pos);

            result.path.emplace_back(address);

            if (comma_pos == std::string_view::npos)
            {
                break;
            }

            // No copy or string modification, just update the string_view beginning
            path.remove_prefix(comma_pos + 1);
        }
    }

    // The remaining string after the colon_pos is the data

    result.data = packet_string.substr(colon_pos + 1);

    return true;
}

#endif // APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY

#endif // APRS_ROUTER_ENABLE_PACKET_SUPPORT

// **************************************************************** //
//                                                                  //
//                                                                  //
// OPERATORS, ENUM, FORMATTING                                      //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_PACKET_NAMESPACE_END

APRS_ROUTER_NAMESPACE_BEGIN

#ifndef APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY

APRS_ROUTER_INLINE routing_option operator|(routing_option lhs, routing_option rhs)
{
    return static_cast<routing_option>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

APRS_ROUTER_INLINE bool try_parse_routing_option(std::string_view text, routing_option& result)
{
    if (text == "none")
    {
        result = routing_option::none;
    }
    else if (text == "route_self")
    {
        result = routing_option::route_self;
    }
    else if (text == "preempt_front")
    {
        result = routing_option::preempt_front;
    }
    else if (text == "preempt_truncate")
    {
        result = routing_option::preempt_truncate;
    }
    else if (text == "preempt_drop")
    {
        result = routing_option::preempt_drop;
    }
    else if (text == "preempt_mark")
    {
        result = routing_option::preempt_mark;
    }
    else if (text == "substitute_complete_n_N_address")
    {
        result = routing_option::substitute_complete_n_N_address;
    }
    else if (text == "substitute_explicit_address")
    {
        result = routing_option::substitute_explicit_address;
    }
    else if (text == "trap_limit_exceeding_n_N_address")
    {
        result = routing_option::trap_limit_exceeding_n_N_address;
    }
    else if (text == "reject_limit_exceeding_n_N_address")
    {
        result = routing_option::reject_limit_exceeding_n_N_address;
    }
    else if (text == "skip_complete_n_N_address")
    {
        result = routing_option::skip_complete_n_N_address;
    }
    else if (text == "preempt_n_N")
    {
        result = routing_option::preempt_n_N;
    }
    else if (text == "strict")
    {
        result = routing_option::strict;
    }
    else if (text == "recommended")
    {
        result = routing_option::recommended;
    }
    else
    {
        assert(false);
        return false;
    }

    return true;
}

APRS_ROUTER_INLINE bool enum_has_flag(routing_option value, routing_option flag)
{
    return (static_cast<int>(value) & static_cast<int>(flag)) != 0;
}

APRS_ROUTER_INLINE std::string to_string(const routing_result& result)
{
APRS_ROUTER_DETAIL_NAMESPACE_USE

    routing_diagnostic_display diag = format(result);

    std::string diag_string;

    for (const auto& e : diag.entries)
    {
        diag_string += create_display_name_diagnostic(e);
    }

    return diag_string;
}

APRS_ROUTER_INLINE std::string to_string(routing_action action)
{
    switch (action)
    {
        case routing_action::none: return "none";
        case routing_action::insert: return "insert";
        case routing_action::remove: return "remove";
        case routing_action::replace: return "replace";
        case routing_action::unset: return "unset";
        case routing_action::set: return "set";
        case routing_action::decrement: return "decrement";
        case routing_action::error: return "error";
        case routing_action::warn: return "warn";
        case routing_action::message: return "message";
    }

    assert(false);

    return "";
}

APRS_ROUTER_INLINE std::string to_string(applies_to target)
{
    switch (target)
    {
        case applies_to::none: return "none";
        case applies_to::from: return "from";
        case applies_to::to: return "to";
        case applies_to::path: return "path";
        case applies_to::data: return "data";
    }

    assert(false);

    return "";
}

APRS_ROUTER_INLINE routing_diagnostic_display format(const routing_result& result)
{
APRS_ROUTER_DETAIL_NAMESPACE_USE

    routing_diagnostic_display diag_format;

    packet routed_packet = result.original_packet;

    for (const auto& a : result.actions)
    {
        if (a.type == routing_action::remove)
        {
            diag_format.entries.push_back(create_diagnostic_print_line(a, routed_packet));
            routed_packet.path.erase(routed_packet.path.begin() + a.index);
        }
        else if (a.type == routing_action::insert)
        {
            routed_packet.path.insert(routed_packet.path.begin() + a.index, a.address);
            diag_format.entries.push_back(create_diagnostic_print_line(a, routed_packet));
        }
        else if (a.type == routing_action::set)
        {
            routed_packet.path[a.index].append("*");
            diag_format.entries.push_back(create_diagnostic_print_line(a, routed_packet));
        }
        else if (a.type == routing_action::unset)
        {
            diag_format.entries.push_back(create_diagnostic_print_line(a, routed_packet));
            routed_packet.path[a.index] = a.address;
        }
        else if (a.type == routing_action::replace ||
                 a.type == routing_action::decrement)
        {
            routed_packet.path[a.index] = a.address;
            diag_format.entries.push_back(create_diagnostic_print_line(a, routed_packet));
        }
    }

    return diag_format;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// ROUTING                                                          //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE bool try_route_packet(const struct APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& packet, const router_settings& settings, routing_result& result)
{
APRS_ROUTER_DETAIL_NAMESPACE_USE

    init_routing_result(packet, result);

    try_route_packet(packet.from, packet.to, packet.path, settings, result.routed_packet.path, result.state, result.actions);

    result.routed = (result.state == routing_state::routed);

    if (!result.routed)
    {
        result.routed_packet.path = packet.path;
    }

    return result.routed;
}

APRS_ROUTER_INLINE bool try_route_packet(std::string_view original_packet_from, std::string_view original_packet_to, const std::vector<std::string>& original_packet_path, const router_settings& settings, std::vector<std::string>& routed_packet_path, enum routing_state& routing_state, std::vector<routing_diagnostic>& routing_actions)
{
    return try_route_packet(original_packet_from, original_packet_to, original_packet_path.begin(), original_packet_path.end(), settings, std::back_inserter(routed_packet_path), routing_state, std::back_inserter(routing_actions));
}

template<class InputIterator, class OutputIterator1, class OutputIterator2>
APRS_ROUTER_INLINE bool try_route_packet(std::string_view original_packet_from, std::string_view original_packet_to, InputIterator original_packet_path_begin, InputIterator original_packet_path_end, const router_settings& settings, OutputIterator1 routed_packet_path_out, enum routing_state& routing_state, OutputIterator2 routing_actions_out, std::pmr::memory_resource* memory_resource)
{
APRS_ROUTER_DETAIL_NAMESPACE_USE

#if APRS_ROUTER_USE_PMR
    assert(memory_resource != nullptr);

    route_state state(memory_resource);
#else
    route_state state;

    (void)memory_resource;
#endif    

    state.packet_from_address = original_packet_from;
    state.packet_to_address = original_packet_to;
    state.packet_path.assign(original_packet_path_begin, original_packet_path_end);
    state.settings.emplace(std::ref(settings));
#if APRS_ROUTER_USE_PMR
    state.memory_resource = memory_resource;
#endif

    init_addresses(state);

    if (is_valid_router_address_and_packet(state))
    {
        routing_state = routing_state::not_routed;
        return false;
    }

    find_used_addresses(state);

    // Packet has finished routing: N0CALL>APRS,CALL,WIDE1,DIGI*:data
    //                                                     ~~~~~
    if (has_packet_routing_ended(state))
    {
        return create_routing_ended_routing(state, routing_state, routing_actions_out);
    }

    // Packet has already been routing by us: N0CALL>APRS,CALL,DIGI*,WIDE1-1,WIDE2-2:data
    //                                                         ~~~~~
    if (has_packet_been_routed_by_us(state))
    {
        return create_routed_by_us_routing(state, routing_state, routing_actions_out);
    }

    // Packet has been sent to us: N0CALL>DIGI,CALL,WIDE1-1,WIDE2-2:data
    //                                    ~~~~
    if (is_packet_sent_to_us(state))
    {
        routing_state = routing_state::already_routed;
        return false;
    }

    if (try_explicit_or_n_N_route(state, routing_state))
    {
        return create_routed_routing(state, routed_packet_path_out, routing_actions_out);
    }

    return false;
}

#endif // APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY

APRS_ROUTER_NAMESPACE_END

// **************************************************************** //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
// PRIVATE DEFINITIONS                                              //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_NAMESPACE_BEGIN

APRS_ROUTER_DETAIL_NAMESPACE_BEGIN

#ifndef APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY

// **************************************************************** //
//                                                                  //
//                                                                  //
// ROUTING                                                          //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE bool try_explicit_or_n_N_route(route_state& state, enum routing_state& routing_state)
{
    routing_state = routing_state::not_routed;

    // Packet has ben sent by us: DIGI>APRS,CALL,WIDE1-1,WIDE2-2:data
    //                            ~~~~
    bool is_routing_self = is_packet_from_us(state);

    if (is_explicit_routing(is_routing_self, state))
    {
        if (try_explicit_route(state))
        {
            routing_state = routing_state::routed;
            return true;
        }
        else
        {
            routing_state = routing_state::not_routed;
            return false;
        }
    }

    // Self routing is only allowed in explicit routing mode
    if (is_routing_self)
    {
        routing_state = routing_state::cannot_route_self;
        return false;
    }

    if (try_n_N_route(state))
    {
        routing_state = routing_state::routed;
        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool is_explicit_routing(bool is_routing_self, std::optional<size_t> maybe_router_address_index, routing_option options)
{
    // Explicit routing is enabled if the packet has the router's address
    //
    // Example:
    //
    // Router's address: DIGI
    //
    // Packet: N0CALL>APRS,CALLA,CALLB*,DIGI,CALLC:data
    //                                  ~~~~

    if (maybe_router_address_index)
    {
        return (!is_routing_self || enum_has_flag(options, routing_option::route_self));
    }

    return false;
}

APRS_ROUTER_INLINE bool is_explicit_routing(bool is_routing_self, const route_state& state)
{
    return is_explicit_routing(is_routing_self, state.maybe_router_address_index, state.settings->get().options);
}

APRS_ROUTER_INLINE bool try_explicit_route(route_state& state)
{
    // If explicitly routing a packet through the router
    // find the router's address in the packet and mark it as used (*)
    // Also unmark all the previously used addresses.
    //
    // preemptive routing allows us to ignore other packets in front of
    // us and proceed on the routing. There are two strategies that
    // can be used with preemptive routing, one which prioritizes our address
    // and what that eliminates unused addresses from the packet.

    const std::optional<size_t> maybe_router_address_index = state.maybe_router_address_index;
#ifndef NDEBUG
    const internal_vector_t<address>& packet_addresses = state.packet_addresses;
#endif
    const size_t unused_address_index = state.unused_address_index;
    const routing_option options = state.settings->get().options;

    if (!maybe_router_address_index)
    {
        // We did not find router's address
        // or a explicit router address in the packet path
        return false;
    }

    size_t router_address_index = maybe_router_address_index.value();

    assert(router_address_index < packet_addresses.size());
    assert(unused_address_index < packet_addresses.size());

    // is_path_based_routing - this means that the matching address is not the router's address, and comes from the router's path
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

    bool have_other_unused_addresses_ahead = router_address_index != unused_address_index;

    bool preempt_drop = enum_has_flag(options, routing_option::preempt_drop);

    // If we don't have any unused addresses ahead of us, then proceed
    // If preempt_drop mode is enabled, different processing of the packet is required
    if (!have_other_unused_addresses_ahead && !preempt_drop)
    {
        try_explicit_basic_route(state, router_address_index);
        return true;
    }
    else
    {
        if (try_preempt_explicit_route(state))
        {
            return true;
        }
    }

    return false;
}

APRS_ROUTER_INLINE bool try_explicit_basic_route(route_state& state, size_t set_address_index)
{
    // Route a packet using non-preemptive explicit routing.
    //
    // In the simplest case, find a matching address and set it as 'used'.
    //
    //   1) Unmark any 'used' addresses in front of the matched address:
    //
    //      Input: N0CALL>APRS,CALL*,DIGI:data
    //                               ~~~~
    //                               matched address based on router's address
    //
    //      Output: N0CALL>APRS,CALL,DIGI*:data
    //                          ~~~~ ~~~~~
    //                          ^    matched address marked as 'used' 
    //                          |
    //                          address marked as unused
    //
    //   2) Path based matching.
    //   
    //      If the address matched is not the router's address, but matches the router's path addresses
    //      we also need to insert the router's address in front:
    //
    //      2.a) Simplest case:
    //
    //           Router's address: DIGI
    //
    //           Router's path: CALLB
    //
    //           Input: N0CALL>APRS,CALLA*,CALLB:data
    //                                     ~~~~~
    //                                     matched address based on path
    //
    //           Output: N0CALL>APRS,CALLA,DIGI,CALLB*:data
    //                               ~~~~~ ~~~~ ~~~~~~
    //                               ^     ^    matched address marked as 'used' 
    //                               |     inserted router address
    //                               address marked as unused
    //
    //      2.b) If we cannot insert an address because the packet has 8 addresses in the path
    //           then replace the matched address with the router's address:
    //
    //           Router's address: DIGI
    //
    //           Router's path: CALLB
    //
    //           Input: N0CALL>APRS,A,B,C,D,E,F,G*,CALLB:data
    //                                             ~~~~~
    //                                             matched address based on path
    //
    //           Output: N0CALL>APRS,A,B,C,D,E,F,G,DIGI*:data
    //                                           ~ ~~~~~
    //                                           ^ address replaced with router's address and marked as 'used' 
    //                                           |
    //                                           address marked as unused
    //
    //   3) A variation of 2.b when substitute_explicit_address is set. 
    //      The matched address will be replaced by the route's address
    //      even if we have space to insert the address
    //
    //      Router's address: DIGI
    //
    //      Router's path: CALLB
    //
    //      Input: N0CALL>APRS,CALLA*,CALLB:data
    //                                ~~~~~
    //                                matched address based on path
    //
    //      Output: N0CALL>APRS,CALLA,DIGI*:data
    //                          ~~~~~ ~~~~~
    //                          ^     address replaced with router's address and marked as 'used' 
    //                          |
    //                          address marked as unused

    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    const bool is_path_based_routing = state.is_path_based_routing;
    const size_t unused_address_index = state.unused_address_index;
    const address& router_address = state.router_address;
    const std::string_view router_address_string = state.settings->get().address;
    const routing_option options = state.settings->get().options;
    const bool enable_diagnostics = state.settings->get().enable_diagnostics;
    internal_vector_t<routing_diagnostic>& actions = state.actions;

    assert(set_address_index < packet_addresses.size());
    assert(unused_address_index < packet_addresses.size());

    bool substitute_explicit_address = enum_has_flag(options, routing_option::substitute_explicit_address);

    if (substitute_explicit_address)
    {
        push_address_replaced_diagnostic(packet_addresses, set_address_index, router_address_string, enable_diagnostics, actions);
        replace_address_with_router_address(packet_addresses[set_address_index], router_address);
        push_address_unset_diagnostic(packet_addresses, set_address_index, enable_diagnostics, actions);
        set_address_as_used(packet_addresses, set_address_index);
        push_address_set_diagnostic(packet_addresses, set_address_index, enable_diagnostics, actions);
        return true;
    }

    if (is_path_based_routing)
    {
        if (try_insert_address(packet_addresses, unused_address_index, router_address_string))
        {
            push_address_unset_diagnostic(packet_addresses, set_address_index, enable_diagnostics, actions);
            set_address_as_used(packet_addresses, set_address_index + 1);
            push_address_inserted_diagnostic(packet_addresses, unused_address_index, enable_diagnostics, actions);
            push_address_set_diagnostic(packet_addresses, set_address_index + 1, enable_diagnostics, actions);
        }
        else
        {
            push_address_replaced_diagnostic(packet_addresses, set_address_index, router_address_string, enable_diagnostics, actions);
            replace_address_with_router_address(packet_addresses[set_address_index], router_address);
            push_address_unset_diagnostic(packet_addresses, set_address_index, enable_diagnostics, actions);
            set_address_as_used(packet_addresses, set_address_index);
            push_address_set_diagnostic(packet_addresses, set_address_index, enable_diagnostics, actions);
        }
    }
    else
    {
        push_address_unset_diagnostic(packet_addresses, set_address_index, enable_diagnostics, actions);
        set_address_as_used(packet_addresses, set_address_index);
        push_address_set_diagnostic(packet_addresses, set_address_index, enable_diagnostics, actions);
    }

    return true;
}

APRS_ROUTER_INLINE bool try_preempt_explicit_route(route_state& state)
{
    if (try_preempt_transform_explicit_route(state))
    {
        try_explicit_basic_route(state, state.unused_address_index);
        return true;
    }
    return false;
}

APRS_ROUTER_INLINE bool try_preempt_transform_explicit_route(route_state& state)
{
    const bool enable_diagnostics = state.settings->get().enable_diagnostics;
    const routing_option options = state.settings->get().options;
    const size_t router_address_index = state.maybe_router_address_index.value();
    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    size_t& unused_address_index = state.unused_address_index;
    internal_vector_t<routing_diagnostic>& actions = state.actions;
#if APRS_ROUTER_USE_PMR
    std::pmr::memory_resource* memory_resource = state.memory_resource;
#endif

    assert(router_address_index < packet_addresses.size());
    assert(unused_address_index < packet_addresses.size());

    if (enum_has_flag(options, routing_option::preempt_front))
    {
        // Diagnostics are calculated before the move.
        // Store diagnostics in a temporary vector, and if the move is not successful
        // then we will not add the diagnostics to the actions.
#if APRS_ROUTER_USE_PMR
        internal_vector_t<routing_diagnostic> temp_d(memory_resource);
#else
        internal_vector_t<routing_diagnostic> temp_d;
#endif
        create_address_move_diagnostic(packet_addresses, router_address_index, unused_address_index, enable_diagnostics, temp_d);
        if (try_move_address_to_position(packet_addresses, router_address_index, unused_address_index))
        {
            std::copy(temp_d.begin(), temp_d.end(), std::back_inserter(actions));
        }
        return true;
    }
    else if (enum_has_flag(options, routing_option::preempt_truncate))
    {
        // Diagnostics are calculated before the move.
        // Store diagnostics in a temporary vector, and if the move is not successful
        // then we will not add the diagnostics to the actions.
#if APRS_ROUTER_USE_PMR
        internal_vector_t<routing_diagnostic> temp_d(memory_resource);
#else
        internal_vector_t<routing_diagnostic> temp_d;
#endif
        create_truncate_address_range_diagnostic(packet_addresses, unused_address_index, router_address_index, enable_diagnostics, temp_d);
        if (try_truncate_address_range(packet_addresses, unused_address_index, router_address_index))
        {
            std::copy(temp_d.begin(), temp_d.end(), std::back_inserter(actions));
        }
        return true;
    }
    else if (enum_has_flag(options, routing_option::preempt_drop))
    {
        // Diagnostics are calculated before the move.
        // Store diagnostics in a temporary vector, and if the move is not successful
        // then we will not add the diagnostics to the actions.
#if APRS_ROUTER_USE_PMR
        internal_vector_t<routing_diagnostic> temp_d(memory_resource);
#else
        internal_vector_t<routing_diagnostic> temp_d;
#endif
        create_truncate_address_range_diagnostic(packet_addresses, 0, router_address_index, enable_diagnostics, temp_d);
        if (try_truncate_address_range(packet_addresses, 0, router_address_index))
        {
            std::copy(temp_d.begin(), temp_d.end(), std::back_inserter(actions));
        }

        // Reset the unused address index to 0 as we are dropping all the addresses
        // in front of the router's matched address
        //
        // Example:
        //
        // Router address: DIGI
        // Router path: E
        //
        // Original packet: N0CALL>APRS,A,B*,C,D,E,F:data
        //                                   ~
        //                                   unused_address_index = 2
        // 
        // Truncated packet: N0CALL>APRS,E,F:data
        //                               ~
        //                               unused_address_index = 0

        unused_address_index = 0;

        return true;
    }
    else if (enum_has_flag(options, routing_option::preempt_mark))
    {
        // Reset the unused address index to the index of the router's matched address
        //
        // Example:
        //
        // Router address: DIGI
        // Router path: E
        //
        // Original packet: N0CALL>APRS,A,B*,C,D,E,F:data
        //                                   ~
        //                                   unused_address_index = 2
        // 
        // Original packet: N0CALL>APRS,A,B*,C,D,E,F:data
        //                                       ~
        //                                       unused_address_index = 4

        unused_address_index = router_address_index;

        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool try_n_N_route(route_state& state)
{
    // n-N routing function. Routes a packet using the n-N routing algorithm.
    //
    // If the first unused address in the packet is an n-N address, apply the n-N routing algorithm:
    //
    // Example:
    //
    //   Router address: DIGI
    //
    //   Router path: WIDE2-2,WIDE1-2
    //
    //   Packet: N0CALL>APRS,CALL*,WIDE1-2,WIDE2-2:data
    //                           ~ ~~~~~~~
    //
    //   Routed Packet: N0CALL>APRS,CALL,DIGI*,WIDE1-1,WIDE2-2:data
    //                                 ~ ~~~~~~~~~~~~~
    //
    // If an n-N packet address exceeds the maximum number of hops, trap it if "trap_limit_exceeding_n_N_address" is set:
    //
    // Example:
    // 
    //   Router path: WIDE2-2
    //
    //   Packet: N0CALL>APRS,CALL*,WIDE2-3:data
    //                             ~~~~~~~
    //
    //   Routed Packet: N0CALL>APRS,CALL,DIGI*:data
    //                                   ~~~~~

    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    const internal_vector_t<address>& router_n_N_addresses = state.router_n_N_addresses;
    const routing_option options = state.settings->get().options;
    const size_t unused_address_index = state.unused_address_index;
    const address& unused_address = state.packet_addresses[unused_address_index];

    auto unused_address_index_pair = find_first_unused_n_N_address_index(packet_addresses, router_n_N_addresses, options);

    if (!unused_address_index_pair)
    {
        return false;
    }

    auto [address_n_N_index, router_n_N_index] = unused_address_index_pair.value();

    // We should not route the packet if we have other unused addresses in front of us.
    // If we have unused addresses, then we should stop the routing process:
    //
    // N0CALL>APRS,CALL,WIDE1-1,WIDE2-1:data
    //             ~~~~
    //             unused address
    //
    //  But we should still allow empty addresses to be routed so they can be removed

    if (address_n_N_index > unused_address_index)
    {
        if (!unused_address.text.empty())
        {
            return false;
        }
    }

    assert(address_n_N_index < packet_addresses.size());
    assert(router_n_N_index < router_n_N_addresses.size());

    if (try_trap_n_N_route(state, packet_addresses[address_n_N_index], router_n_N_addresses[router_n_N_index]))
    {
        return true;
    }

    try_n_N_route_no_trap(state, address_n_N_index);

    return true;
}

APRS_ROUTER_INLINE bool try_n_N_route_no_trap(route_state& state, size_t packet_n_N_address_index)
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
    //      3.a. If "substitute_complete_n_N_address" is set, remove the ADDRESSn-N where N is 0
    //
    //      3.b. If "substitute_complete_n_N_address" is not set, mark the ADDRESSn-N as "used"
    //
    //   4. Create and update the routing result to reflect these changes.
    //
    // Example:
    //
    //   Router address:  DIGI
    //
    //   Router path:     WIDE1-1,WIDE2-1
    //                    ~~~~~~~~~~~~~~~
    //                    router_n_N_addresses
    //
    //   Original packet: N0CALL>APRS,CALL*,WIDE1-1,WIDE2-1:data
    //                                ~~~~~~~~~~~~~~~~~~~~~
    //                                packet_addresses
    //
    //   After step 1:    N0CALL>APRS,CALL*,WIDE1,WIDE2-1:data
    //   After step 2:    N0CALL>APRS,CALL*,DIGI,WIDE1,WIDE2-1:data
    //   After step 2.a:  N0CALL>APRS,CALL*,DIGI,WIDE1,WIDE2-1:data
    //   After step 3.a:  N0CALL>APRS,CALL,DIGI*,WIDE2-1:data
    //   After step 3.b:  N0CALL>APRS,CALL,DIGI,WIDE1*,WIDE2-1:data

    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    const routing_option options = state.settings->get().options;
    const bool enable_diagnostics = state.settings->get().enable_diagnostics;
    internal_vector_t<routing_diagnostic>& actions = state.actions;

    assert(packet_n_N_address_index < packet_addresses.size());

    bool substitute_zero_hops = enum_has_flag(options, routing_option::substitute_complete_n_N_address);

    address& n_N_address = packet_addresses[packet_n_N_address_index];

    assert(n_N_address.n > 0);

    if (try_decrement_n_N_address(state, n_N_address))
    {
        push_address_decremented_diagnostic(n_N_address, enable_diagnostics, actions);
    }

    // If we are in a position which will require us to insert more than 8 addresses
    // just return, the only thing we can do is increment the counter

    if (try_complete_n_N_route(state, n_N_address, substitute_zero_hops))
    {
        return true;
    }

    if (substitute_zero_hops && n_N_address.N == 0)
    {
        try_substitute_complete_n_N_address(state, packet_n_N_address_index);
        return true;
    }

    try_insert_n_N_route(state, packet_n_N_address_index);

    return true;
}

APRS_ROUTER_INLINE bool try_complete_n_N_route(route_state& state, address& n_N_address, bool substitute_zero_hops)
{
    // If we are in a position which will require us to insert more than 8 addresses
    // just return, the only thing we can do is increment the n-N counter
    //
    // Example:
    //
    // Input: FROM>TO,A,B,C,D,E,F,G,WIDE2-2:data
    //                ~ ~ ~ ~ ~ ~ ~ ~~~~~~~
    //
    // Output: FROM>TO,A,B,C,D,E,F,G,WIDE2-1:data
    //                               ~~~~~~~
    //
    // There are a few circumstances where we can still continue with the 
    // insertion even when the address count is 8. For example if we will
    // shortly shrink the packet path (substitute_complete_n_N_address)

    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    const bool enable_diagnostics = state.settings->get().enable_diagnostics;
    internal_vector_t<routing_diagnostic>& actions = state.actions;

    if (packet_addresses.size() >= 8)
    {
        // The n-N address has no remaining hops, but we cannot substitute it
        // with our router's address because "substitute_zero_hops" is unset
        // we also have more than 8 addresses, so just mark the completed address as "set"
        if (!substitute_zero_hops && n_N_address.N == 0)
        {
            push_address_unset_diagnostic(packet_addresses, n_N_address.index, enable_diagnostics, actions);
            set_address_as_used(packet_addresses, n_N_address);
            push_address_set_diagnostic(packet_addresses, n_N_address.index, enable_diagnostics, actions);
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

APRS_ROUTER_INLINE bool try_insert_n_N_route(route_state& state, size_t& packet_n_N_address_index)
{
    // Insert the router's address in front of the n-N address:
    //
    // Example:
    //
    // Input:  FROM>TO,CALL,WIDE2-1:data
    //                      ~~~~~~~
    //
    // Output: FROM>TO,CALL,DIGI*,WIDE2-1:data
    //                      ~~~~~
    //                      Insert and mark the router's address as used
    //                      n-N address is left unchanged by this function

    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    const std::string_view router_address = state.settings->get().address;
    const bool enable_diagnostics = state.settings->get().enable_diagnostics;
    internal_vector_t<routing_diagnostic>& actions = state.actions;
    const bool substitute_zero_hops = enum_has_flag(state.settings->get().options, routing_option::substitute_complete_n_N_address);

    assert(packet_n_N_address_index < packet_addresses.size());
    assert(packet_addresses.size() < 8);

    struct address& n_N_address = packet_addresses[packet_n_N_address_index];

    struct address new_address;
    new_address.text = router_address;
    new_address.kind = address_kind::other;
    new_address.length = router_address.size();

    bool set_new_address_as_used = false;

    if (substitute_zero_hops || n_N_address.N > 0)
    {
        set_new_address_as_used = true;
    }
    else
    {
        push_address_unset_diagnostic(packet_addresses, n_N_address.index, enable_diagnostics, actions);
        set_address_as_used(packet_addresses, n_N_address);
        push_address_set_diagnostic(packet_addresses, n_N_address.index, enable_diagnostics, actions);
    }

    size_t initial_offset = packet_addresses[0].offset;

    packet_addresses.insert(packet_addresses.begin() + packet_n_N_address_index, new_address);

    update_addresses_index(packet_addresses);
    update_addresses_offset(packet_addresses, initial_offset);

    push_address_inserted_diagnostic(packet_addresses, packet_n_N_address_index, enable_diagnostics, actions);

    if (set_new_address_as_used)
    {
        push_address_unset_diagnostic(packet_addresses, std::nullopt, enable_diagnostics, actions);
        set_address_as_used(packet_addresses, packet_n_N_address_index);
        push_address_set_diagnostic(packet_addresses, packet_n_N_address_index, enable_diagnostics, actions);
    }

    packet_n_N_address_index++;

    return true;
}

APRS_ROUTER_INLINE bool try_trap_n_N_route(route_state& state, address& packet_n_N_address, const address& router_n_N_address)
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
    //                             7 2
    //
    // Input:  FROM>TO,WIDE7-7:data
    //                 ~~~~~~~
    //                 packet_n_N_address
    //
    // Output: FROM>TO,DIGI*:data
    //                 ~~~~~
    //                 router_address
    //
    // Returns 'true' if the route is trapped, 'false' otherwise

    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    const std::string_view router_address = state.settings->get().address;
    const routing_option options = state.settings->get().options;
    const bool enable_diagnostics = state.settings->get().enable_diagnostics;
    internal_vector_t<routing_diagnostic>& actions = state.actions;

    bool trap_limit_exceeding_n_N_address = enum_has_flag(options, routing_option::trap_limit_exceeding_n_N_address);

    if (trap_limit_exceeding_n_N_address)
    {
        if (router_n_N_address.N > 0 && packet_n_N_address.N > router_n_N_address.N)
        {
            push_address_replaced_diagnostic(packet_addresses, packet_n_N_address.index, router_address, enable_diagnostics, actions);

            packet_n_N_address.text = router_address;
            packet_n_N_address.length = router_address.size();
            packet_n_N_address.n = 0;
            packet_n_N_address.N = 0;

            push_address_unset_diagnostic(packet_addresses, packet_n_N_address.index, enable_diagnostics, actions);
            set_address_as_used(packet_addresses, packet_n_N_address);
            push_address_set_diagnostic(packet_addresses, packet_n_N_address.index, enable_diagnostics, actions);

            return true;
        }
    }

    return false;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// TEST ROUTING                                                     //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE bool try_route_packet_by_index(const struct routing_result& routing_result, APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& result)
{
    if (routing_result.state != routing_state::routed)
    {
        return false;
    }

    assert(routing_result.actions.size() > 0);

    result = routing_result.original_packet;

    for (const auto& a : routing_result.actions)
    {
        if (a.type == routing_action::remove)
        {
            result.path.erase(result.path.begin() + a.index);
        }
        else if (a.type == routing_action::insert)
        {
            result.path.insert(result.path.begin() + a.index, a.address);
        }
        else if (a.type == routing_action::set)
        {
            result.path[a.index].append("*");
        }
        else if (a.type == routing_action::unset || a.type == routing_action::replace ||
            a.type == routing_action::decrement)
        {
            result.path[a.index] = a.address;
        }
        else
        {
            return false;
        }
    }

    return true;
}

APRS_ROUTER_INLINE bool try_route_packet_by_start_end(const struct routing_result& routing_result, APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& result)
{
    if (routing_result.state != routing_state::routed)
    {
        return false;
    }

    assert(routing_result.actions.size() > 0);

    std::string routed_packet = to_string(routing_result.original_packet);

    for (const auto& a : routing_result.actions)
    {
        size_t start = a.start;
        size_t end = a.end;
        size_t count = a.end - a.start;

        if (a.type == routing_action::remove)
        {
            if (routed_packet[end + 1] == ':' || a.index == 0)
            {
                count++;
            }
            else if (a.index != 0)
            {
                start--;
                count++;
            }
            routed_packet.erase(start, count);
        }
        else if (a.type == routing_action::insert)
        {
            routed_packet.insert(start, a.address);
            if (routed_packet[end + 1] != ',' || a.index == 0)
            {
                routed_packet.insert(end, ",");
            }
        }
        else if (a.type == routing_action::set)
        {
            routed_packet.insert(end, "*");
        }
        else if (a.type == routing_action::unset || a.type == routing_action::replace ||
            a.type == routing_action::decrement)
        {
            routed_packet.erase(start, count);
            routed_packet.insert(start, a.address);
        }
        else
        {
            return false;
        }
    }

    if (!try_decode_packet(routed_packet, result))
    {
        return false;
    }

    return true;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// ROUTING RESULT                                                   //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE void init_routing_result(const struct APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& packet, routing_result& result)
{
    result.routed = false;
    result.success = true;
    result.original_packet = packet;
    result.routed_packet.from = packet.from;
    result.routed_packet.to = packet.to;
    result.routed_packet.data = packet.data;
    result.routed_packet.path.clear();
    result.actions.clear();
}

template <class OutputIterator>
APRS_ROUTER_INLINE bool create_routing_ended_routing(const route_state& state, enum routing_state& routing_state, OutputIterator routing_actions_out)
{
#if APRS_ROUTER_USE_PMR
    internal_vector_t<routing_diagnostic> routing_actions(state.memory_resource);
#else
    internal_vector_t<routing_diagnostic> routing_actions;
#endif
    bool result = create_routing_ended_routing(state, routing_state, routing_actions);
    for (auto& diag : routing_actions)
    {
        *routing_actions_out++ = std::move(diag);
    }
    return result;
}

APRS_ROUTER_INLINE bool create_routing_ended_routing(const route_state& state, enum routing_state& routing_state, internal_vector_t<routing_diagnostic>& routing_actions)
{
    push_routing_ended_diagnostic(state.packet_addresses.back(), state.settings->get().enable_diagnostics, routing_actions);
    routing_state = routing_state::not_routed;
    return false;
}

template <class OutputIterator>
APRS_ROUTER_INLINE bool create_routed_by_us_routing(const route_state& state, enum routing_state& routing_state, OutputIterator routing_actions_out)
{
#if APRS_ROUTER_USE_PMR
    internal_vector_t<routing_diagnostic> routing_actions(state.memory_resource);
#else
    internal_vector_t<routing_diagnostic> routing_actions;
#endif
    bool result = create_routed_by_us_routing(state, routing_state, routing_actions);
    for (auto& diag : routing_actions)
    {
        *routing_actions_out++ = std::move(diag);
    }
    return result;
}

APRS_ROUTER_INLINE bool create_routed_by_us_routing(const route_state& state, enum routing_state& routing_state, internal_vector_t<routing_diagnostic>& routing_actions)
{
    push_routed_by_us_diagnostic(state.packet_addresses, state.maybe_last_used_address_index, state.settings->get().enable_diagnostics, routing_actions);
    routing_state = routing_state::already_routed;
    return false;
}

template <class OutputIterator1, class OutputIterator2>
APRS_ROUTER_INLINE bool create_routed_routing(route_state& state, OutputIterator1 routed_packet_path_out, OutputIterator2 routing_actions_out)
{
#if APRS_ROUTER_USE_PMR
    internal_vector_t<routing_diagnostic> routing_actions(state.memory_resource);
    internal_vector_t<std::string> routed_packet_path(state.memory_resource);
#else
    internal_vector_t<routing_diagnostic> routing_actions;
    internal_vector_t<std::string> routed_packet_path;
#endif

    bool result = create_routed_routing(state, routed_packet_path, routing_actions);

    for (auto& address : routed_packet_path)
    {
        *routed_packet_path_out++ = std::move(address);
    }

    for (auto& diag : routing_actions)
    {
        *routing_actions_out++ = std::move(diag);
    }

    return result;
}

APRS_ROUTER_INLINE bool create_routed_routing(route_state& state, internal_vector_t<std::string>& routed_packet_path, internal_vector_t<routing_diagnostic>& routing_actions)
{
    try_truncate_empty_addresses(state);

    routed_packet_path.clear();

    for (const auto& address : state.packet_addresses)
    {
        if (!address.text.empty())
        {
            routed_packet_path.push_back(to_string(address));
        }
    }

    routing_actions = state.actions;

    return true;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// ROUTING DIAGNOSTICS                                              //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE bool push_routing_ended_diagnostic(const address& address, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    if (enable_diagnostics)
    {
        routing_diagnostic diag;

        diag.target = applies_to::path;
        diag.type = routing_action::warn;
        diag.message = "Packet has finished routing";
        diag.address = address.text;
        diag.start = address.offset;
        diag.end = diag.start + address.length;
        diag.index = address.index;

        d.push_back(diag);
    }
    return true;
}

APRS_ROUTER_INLINE bool push_routed_by_us_diagnostic(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    if (enable_diagnostics && maybe_last_used_address_index)
    {
        routing_diagnostic diag;

        const address& address = packet_addresses[maybe_last_used_address_index.value()];

        diag.target = applies_to::path;
        diag.type = routing_action::warn;
        diag.message = "Packet has already been routed";
        diag.address = address.text;
        diag.start = address.offset;
        diag.end = diag.start + address.length;
        diag.index = address.index;

        d.push_back(diag);
    }
    return true;
}

APRS_ROUTER_INLINE bool push_address_set_diagnostic(const internal_vector_t<address>& packet_addresses, size_t set_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    assert(set_address_index < packet_addresses.size());

    if (enable_diagnostics)
    {
        routing_diagnostic diag;

        const address& address = packet_addresses[set_address_index];

        diag.target = applies_to::path;
        diag.type = routing_action::set;
        diag.message = "Packet address marked as 'set'";
        diag.address = to_string(address);
        diag.start = address.offset;
        diag.end = diag.start + address.length;
        diag.index = set_address_index;

        if (address.mark && !diag.address.empty())
        {
            // Remove the '*' marker
            diag.address.pop_back();
            diag.end--;
        }

        d.push_back(diag);
    }
    return true;
}

APRS_ROUTER_INLINE bool push_address_unset_diagnostic(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_set_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    // Called before unsetting addresses, before calling 'set_address_as_used'

    size_t set_address_index = maybe_set_address_index.value_or(packet_addresses.size()); // intentionally out of bounds if not set

    assert(!maybe_set_address_index || set_address_index < packet_addresses.size());
    assert(packet_addresses.size() > 0);

    if (enable_diagnostics)
    {
        size_t i = 0;
        size_t offset = packet_addresses[0].offset;
        for (const auto& address : packet_addresses)
        {
            size_t length = address.length;

            if (address.mark && address.index != set_address_index)
            {
                routing_diagnostic diag;

                diag.target = applies_to::path;
                diag.type = routing_action::unset;
                diag.message = "Packet address marked as 'unset'";
                diag.address = to_string(address);
                diag.start = offset;
                diag.end = diag.start + length;
                diag.index = i;

                if (address.mark && !diag.address.empty())
                {
                    // Remove the '*' marker
                    diag.address.pop_back();
                }

                d.push_back(diag);
            }

            if (address.mark)
            {
                length--;
            }

            offset += length + 1;

            i++;
        }
    }
    return true;
}

APRS_ROUTER_INLINE bool push_address_replaced_diagnostic(const internal_vector_t<address>& packet_addresses, size_t set_address_index, std::string_view new_address, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    assert(set_address_index < packet_addresses.size());

    if (enable_diagnostics)
    {
        routing_diagnostic diag;

        const address& address = packet_addresses[set_address_index];

        diag.target = applies_to::path;
        diag.type = routing_action::replace;
        diag.message = "Packet address replaced";
        diag.address = new_address;
        diag.start = address.offset;
        diag.end = diag.start + address.length;
        diag.index = set_address_index;

        d.push_back(diag);
    }
    return true;
}

APRS_ROUTER_INLINE bool push_address_decremented_diagnostic(address& address, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    if (enable_diagnostics)
    {
        routing_diagnostic diag;

        diag.target = applies_to::path;
        diag.type = routing_action::decrement;
        diag.message = "Packet address decremented";
        diag.address = to_string(address);
        diag.start = address.offset;
        diag.end = diag.start + address.length;
        diag.index = address.index;

        // +2 as push_address_decremented_diagnostic is called after the address was decremented
        // with the length decremented by 2 if N is 0
        if (address.N == 0)
        {
            diag.end += 2;
        }

        d.push_back(diag);
    }
    return true;
}

APRS_ROUTER_INLINE bool push_address_inserted_diagnostic(const internal_vector_t<address>& packet_addresses, size_t insert_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    assert(insert_address_index < packet_addresses.size());

    if (enable_diagnostics)
    {
        routing_diagnostic diag;

        const address& address = packet_addresses[insert_address_index];

        diag.target = applies_to::path;
        diag.type = routing_action::insert;
        diag.message = "Packet address inserted";
        diag.address = address.text;
        diag.start = address.offset;
        diag.end = diag.start + address.text.size();
        diag.index = insert_address_index;

        d.push_back(diag);
    }
    return true;
}

APRS_ROUTER_INLINE bool push_address_removed_diagnostic(const internal_vector_t<address>& packet_addresses, size_t remove_address_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    assert(remove_address_index < packet_addresses.size());

    if (enable_diagnostics)
    {
        routing_diagnostic diag;

        const address& address = packet_addresses[remove_address_index];

        diag.target = applies_to::path;
        diag.type = routing_action::remove;
        diag.message = "Packet address removed";
        diag.address = address.text;
        diag.start = address.offset;
        diag.end = diag.start + address.text.size();
        diag.index = remove_address_index;

        d.push_back(diag);
    }
    return true;
}

APRS_ROUTER_INLINE bool create_address_move_diagnostic(const internal_vector_t<address>& packet_addresses, size_t from_index, size_t to_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    assert(from_index < packet_addresses.size());
    assert(to_index < packet_addresses.size());

    if (enable_diagnostics)
    {
        routing_diagnostic remove_diag;

        const address& removed_address = packet_addresses[from_index];

        remove_diag.target = applies_to::path;
        remove_diag.type = routing_action::remove;
        remove_diag.message = "Packet address removed";
        remove_diag.address = to_string(removed_address);
        remove_diag.start = removed_address.offset;
        remove_diag.end = remove_diag.start + removed_address.length;
        remove_diag.index = from_index;

        d.push_back(remove_diag);

        routing_diagnostic insert_diag;

        const address& inserted_address = packet_addresses[to_index];

        insert_diag.target = applies_to::path;
        insert_diag.type = routing_action::insert;
        insert_diag.message = "Packet address inserted";
        insert_diag.address = to_string(removed_address);
        insert_diag.start = inserted_address.offset;
        insert_diag.end = insert_diag.start + removed_address.length;
        insert_diag.index = to_index;

        d.push_back(insert_diag);
    }
    return true;
}

APRS_ROUTER_INLINE bool create_truncate_address_range_diagnostic(const internal_vector_t<address>& packet_addresses, size_t from_index, size_t to_index, bool enable_diagnostics, internal_vector_t<routing_diagnostic>& d)
{
    assert(from_index < packet_addresses.size());
    assert(to_index < packet_addresses.size());

    if (!enable_diagnostics)
    {
        return false;
    }

    assert(from_index < packet_addresses.size());

    size_t initial_offset = packet_addresses[from_index].offset;

    for (size_t i = from_index; i < packet_addresses.size() && i < to_index; i++)
    {
        routing_diagnostic diag;

        const address& address = packet_addresses[i];

        diag.target = applies_to::path;
        diag.type = routing_action::remove;
        diag.message = "Packet address removed";
        diag.address = address.text;
        diag.start = initial_offset;
        diag.end = diag.start + address.length;
        diag.index = from_index;

        d.push_back(diag);
    }
    return true;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// ROUTING DIAGNOSTICS FORMAT                                       //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE std::string create_display_name_diagnostic(const routing_diagnostic_display_entry& line)
{
    // Creates a diagnostic message.
    //
    // Beginning of example:
    //
    // Packet address removed:
    //
    // N0CALL>APRS,CALLA,CALLB,CALLC,CALLD:data
    //                               ~~~~~
    //
    // End of example

    std::string diag_string;

    diag_string.append(line.message);
    diag_string.append(":");
    diag_string.append("\n");
    diag_string.append("\n");
    diag_string.append(line.packet_string);
    diag_string.append("\n");
    diag_string.append(line.highlight_string);
    diag_string.append("\n");
    diag_string.append("\n");

    return diag_string;
}

APRS_ROUTER_INLINE routing_diagnostic_display_entry create_diagnostic_print_line(const routing_diagnostic& diag, const APRS_ROUTER_PACKET_NAMESPACE_REFERENCE packet& routed_packet)
{
    // Creates a diagnostic entry.
    //
    // Packet string    - entry.packet_string:     "N0CALL>APRS,CALLA,CALLB,CALLC,CALLD:data"
    // Highlight string - entry.highlight_string:  "                              ~~~~~"
    // Message          - entry.message:           "Packet address removed"

    routing_diagnostic_display_entry entry;

    entry.message = diag.message;
    entry.packet_string = to_string(routed_packet);
    entry.highlight_string.append(std::string(diag.start, ' '));
    entry.highlight_string.append(std::string(diag.end - diag.start, '~'));
    if (diag.type == routing_action::set)
    {
        entry.highlight_string.append("~");
    }

    return entry;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// ADDRESS                                                          //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE bool operator==(const address& lhs, const address& rhs)
{
    return lhs.text == rhs.text &&
           lhs.n == rhs.n &&
           lhs.N == rhs.N &&
           lhs.ssid == rhs.ssid &&
           lhs.mark == rhs.mark;
}

APRS_ROUTER_INLINE bool operator!=(const address& lhs, const address& rhs)
{
    return !(lhs == rhs);
}

APRS_ROUTER_INLINE std::string to_string(const struct address& address)
{
    if (address.text.empty())
    {
        return "";
    }

    std::string result = address.text;

    if (address.n > 0)
    {
        result += std::to_string(address.n);
    }

    if (address.N > 0)
    {
        result += "-" + std::to_string(address.N);
    }

    if (address.ssid > 0)
    {
        result += "-" + std::to_string(address.ssid);
    }

    if (address.mark)
    {
        result += "*";
    }

    return result;
}

APRS_ROUTER_INLINE bool equal_addresses_ignore_mark(const struct address& lhs, const struct address& rhs)
{
    // Normally we simply use to_string(lhs) == to_string(rhs) to compare addresses.
    // But "to_string" requires us to make a copy of an address to unset the "mark" fields (we typically ignore '*' in address comparisons).
    // Additionally, to_string allocates heap memory in debug builds.
    //
    // This implementation is a fast address comparison function. This function makes no heap memory allocations, nor requires object copies.
    // It trades a bit of speed speed for memory usage.
    // As part of the comparison, it ignores the "mark" field of the address.

    // Ensure the addresses are valid.
    // An address cannot have both the n and N fields and ssid set.
    // If the N number is set, the n number must also be set.

    assert(lhs.n >= 0 && lhs.n <= 7 && lhs.N >= 0 && lhs.N <= 7);
    assert(rhs.n >= 0 && rhs.n <= 7 && rhs.N >= 0 && rhs.N <= 7);
    assert(lhs.ssid >= 0 && lhs.ssid <= 15);
    assert(rhs.ssid >= 0 && rhs.ssid <= 15);
    assert(lhs.n == 0 || lhs.ssid == 0);
    assert(rhs.n == 0 || rhs.ssid == 0);
    assert(lhs.N == 0 || (lhs.N > 0 && lhs.n > 0));
    assert(rhs.N == 0 || (rhs.N > 0 && rhs.n > 0));

    // Various combinations of n, N numbers and ssid can make addresses be equal to each other.
    // 
    //      address      address text     n     -    N   ssid                 
    //  --------------------------------------------------------
    //      WIDE1-2          WIDE         1     -    2    0
    //      WIDE1-2          WIDE1        0     -    0    2
    //      WIDE1            WIDE         1     -    0    0
    //      WIDE1            WIDE1        0     -    0    0
    //      WIDE1            WIDE         1     -    0    0
    //      WIDE2            WIDE         2     -    0    0
    //      WIDE1-1          WIDE         1     -    1    0
    //      WIDE2-2          WIDE         1     -    2    0
    //      WIDE1-1          WIDE         1     -    1    0
    //      WIDE1-1          WIDE1        0     -    0    1
    //      WIDE11-1         WIDE1        1     -    1    0
    //      WIDE11-1         WIDE11       0     -    0    1
    //      WIDE1            WIDE         1     -    0    0
    //      WIDE1            WIDE1        0     -    0    0

    bool equal_address_text = (lhs.text == rhs.text);

    // If all address fields are equal, then we can immediately return true.

    if (equal_address_text && lhs.n == rhs.n &&
        lhs.N == rhs.N && lhs.ssid == rhs.ssid)
    {
        return true;
    }

    // If all the address fields are equal, except for the the address text,
    // we can immediately return false, as there is no way for the addresses to be equal.

    if (lhs.n == rhs.n && lhs.N == rhs.N &&
        lhs.ssid == rhs.ssid && !equal_address_text)
    {
        return false;
    }

    // If both addresses have an n, N number or ssid, and they are not equal,
    // then we can immediately return false, as the addresses cannot be equal.
    //
    // Example:
    //
    // WIDE1   !=  WIDE2
    // WIDE1-1 !=  WIDE1-2
    // CALL-1  !=  CALL-2

    if ((lhs.n > 0 && rhs.n > 0 && lhs.n != rhs.n) ||
        (lhs.N > 0 && rhs.N > 0 && lhs.N != rhs.N) ||
        (lhs.ssid > 0 && rhs.ssid > 0 && lhs.ssid != rhs.ssid))
    {
        return false;
    }

    // If the address text is equal, but the n, N numbers or ssid are different,
    // we can return false, as the addresses cannot be equal.

    if (equal_address_text && (lhs.n != rhs.n || lhs.N != rhs.N || lhs.ssid != rhs.ssid))
    {
        return false;
    }

    // Compare ADDRESSn-N with ADDRESS-SSID
    //
    // Example: "WIDE1-1" can be represented as either n-N (WIDE1-1) or SSID (WIDE1-1)
    // 
    // Check if the base text part is the same ("WIDE" == "WIDE") and the numeric parts match, numerically and by text comparison

    if (!equal_address_text && std::abs(static_cast<int>(lhs.text.size()) - static_cast<int>(rhs.text.size())) == 1)
    {
        if (lhs.n > 0)
        {
            if (lhs.text.size() >= rhs.text.size())
            {
                return false;
            }

            if (!std::equal(lhs.text.begin(), lhs.text.end(), rhs.text.begin()))
            {
                return false;
            }

            if ((rhs.text.back() - '0') == lhs.n)
            {
                if (rhs.ssid > 0)
                {
                    return lhs.N == rhs.ssid;
                }
                return true;
            }
        }
        else if (rhs.n > 0)
        {
            if (rhs.text.size() >= lhs.text.size())
            {
                return false;
            }

            if (!std::equal(rhs.text.begin(), rhs.text.end(), lhs.text.begin()))
            {
                return false;
            }

            if ((lhs.text.back() - '0') == rhs.n)
            {
                if (lhs.ssid > 0)
                {
                    return rhs.N == lhs.ssid;
                }
                return true;
            }
        }
    }

    return false;
}

APRS_ROUTER_INLINE q_construct parse_q_construct(std::string_view text)
{
    using pair_t = std::pair<std::string_view, q_construct>;

    // NOTE: lookup_table is sorted by the first element of the pair

    static constexpr std::array<pair_t, 10> lookup_table =
    {{
        { "qAC", q_construct::qAC },
        { "qAI", q_construct::qAI },
        { "qAO", q_construct::qAO },
        { "qAR", q_construct::qAR },
        { "qAS", q_construct::qAS },
        { "qAU", q_construct::qAU },
        { "qAX", q_construct::qAX },
        { "qAZ", q_construct::qAZ },
        { "qAo", q_construct::qAo },
        { "qAr", q_construct::qAr }
    }};

    auto it = std::lower_bound(lookup_table.begin(), lookup_table.end(), text, [](const pair_t& lhs, std::string_view rhs) {
        return lhs.first < rhs;
    });

    if (it != lookup_table.end() && it->first == text)
    {
        return it->second;
    }

    return q_construct::none;
}

APRS_ROUTER_INLINE address_kind parse_address_kind(std::string_view text)
{
    using pair_t = std::pair<std::string_view, address_kind>;

    // NOTE: lookup_table is sorted by the first element of the pair

    static constexpr std::array<pair_t, 13> lookup_table =
    {{
        { "ECHO", address_kind::echo },
        { "GATE", address_kind::gate },
        { "IGATECALL", address_kind::igatecall },
        { "NOGATE", address_kind::nogate },
        { "OPNTRC", address_kind::opntrc },
        { "OPNTRK", address_kind::opntrk },
        { "RELAY", address_kind::relay },
        { "RFONLY", address_kind::rfonly },
        { "TEMP", address_kind::temp },
        { "TCPIP", address_kind::tcpip },
        { "TCPXX", address_kind::tcpxx },
        { "TRACE", address_kind::trace },
        { "WIDE", address_kind::wide }
    }};

    auto it = std::lower_bound(lookup_table.begin(), lookup_table.end(), text, [](const pair_t& lhs, std::string_view rhs) {
        return lhs.first < rhs;
    });

    if (it != lookup_table.end() && it->first == text)
    {
        return it->second;
    }

    return address_kind::other;
}

APRS_ROUTER_INLINE bool try_parse_address(std::string_view address_string, struct address& address)
{
    std::string_view address_text = address_string;

    address.text = address_text;
    address.mark = false;
    address.ssid = 0;
    address.length = address_text.size();
    address.n = 0;
    address.N = 0;
    address.q = parse_q_construct(address.text);
    address.kind = address_kind::other;

    // Parse Q construct first
    if (address.q != q_construct::none)
    {
        address.kind = address_kind::q;
        return true; // Early return for Q constructs as there is nothing else to parse
    }

    // Check to see if the address is used (ending with *)
    if (!address_text.empty() && address_text.back() == '*')
    {
        address.mark = true;
        address_text.remove_suffix(1); // remove the *
        address.text = address_text; // set the text to the address without the *
    }

    auto sep_position = address_text.find("-");

    // No separator found
    if (sep_position == std::string_view::npos)
    {
        if (!address_text.empty() && isdigit(address_text.back()))
        {
            address.n = address_text.back() - '0'; // get the last character as a number
            address_text.remove_suffix(1); // remove the digit from the address text

            // Validate the n is in the range 1-7
            if (address.n > 0 && address.n <= 7)
            {
                address.text = address_text;
                address.kind = parse_address_kind(address.text);
            }
            else
            {
                address.n = 0;
            }
        }
        else
        {
            address.kind = parse_address_kind(address.text);
        }

        return true;
    }

    // Separator found, check if we have exactly one digit on both sides of the separator, ex WIDE1-1
    // If the address does not match the n-N format, we will treat it as a regular address ex address with SSID
    if (sep_position != std::string_view::npos &&
        std::isdigit(static_cast<int>(address_text[sep_position - 1])) &&
        (sep_position + 1) < address_text.size() && std::isdigit(static_cast<int>(address_text[sep_position + 1])) &&
        (sep_position + 2 == address_text.size()))
    {
        address.n = address_text[sep_position - 1] - '0';
        address.N = address_text[sep_position + 1] - '0';

        if (address.N >= 0 && address.N <= 7 && address.n > 0 && address.n <= 7)
        {
            address.text = address_text.substr(0, sep_position - 1); // remove the separator and both digits from the address text
            address.kind = parse_address_kind(address.text);
        }
        else
        {
            address.n = 0;
            address.N = 0;
        }

        return true;
    }

    // Handle SSID parsing
    // Expecting the separator to be followed by a digit, ex: CALL-1
    if ((sep_position + 1) < address_text.size() && std::isdigit(static_cast<int>(address_text[sep_position + 1])))
    {
        std::string ssid_str = std::string(address_text.substr(sep_position + 1));

        // Check for a single digit or two digits, ex: CALL-1 or CALL-12
        if (ssid_str.size() == 1 || (ssid_str.size() == 2 && std::isdigit(static_cast<int>(ssid_str[1]))))
        {
            int ssid;
            if (!try_parse_int(ssid_str, ssid))
            {
                return true;
            }

            if (ssid >= 0 && ssid <= 15)
            {
                address.ssid = ssid;
                address.text = address_text.substr(0, sep_position);
            }
        }
    }

    return true;
}

APRS_ROUTER_INLINE bool try_parse_n_N_address(std::string_view address_string, struct address& address)
{
    std::string_view address_text = address_string;

    address.text = address_text;
    address.mark = false;
    address.length = address_text.size();
    address.n = 0;
    address.N = 0;
    address.kind = address_kind::other;

    // Check to see if the address is used (ending with *)
    if (!address_text.empty() && address_text.back() == '*')
    {
        address.mark = true;
        address_text.remove_suffix(1); // remove the *
        address.text = address_text; // set the text to the address without the *
    }

    auto sep_position = address_text.find("-");

    // No separator found
    if (sep_position == std::string_view::npos)
    {
        if (!address_text.empty() && isdigit(address_text.back()))
        {
            address.n = address_text.back() - '0'; // get the last character as a number
            address_text.remove_suffix(1); // remove the digit from the address text

            // Validate the n is in the range 1-7
            if (address.n > 0 && address.n <= 7)
            {
                address.text = address_text;
                address.kind = parse_address_kind(address.text);
            }
            else
            {
                address.n = 0;
            }
        }
        else
        {
            address.kind = parse_address_kind(address.text);
        }

        return true;
    }

    // Separator found, check if we have exactly one digit on both sides of the separator, ex WIDE1-1
    // If the address does not match the n-N format, we will treat it as a regular address ex address with SSID
    if (sep_position != std::string_view::npos &&
        std::isdigit(static_cast<int>(address_text[sep_position - 1])) &&
        (sep_position + 1) < address_text.size() && std::isdigit(static_cast<int>(address_text[sep_position + 1])) &&
        (sep_position + 2 == address_text.size()))
    {
        address.n = address_text[sep_position - 1] - '0';
        address.N = address_text[sep_position + 1] - '0';

        if (address.N >= 0 && address.N <= 7 && address.n > 0 && address.n <= 7)
        {
            address.text = address_text.substr(0, sep_position - 1); // remove the separator and both digits from the address text
            address.kind = parse_address_kind(address.text);
        }
        else
        {
            address.n = 0;
            address.N = 0;
        }

        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool try_parse_address_with_ssid(std::string_view address_string, struct address& address)
{
    // This function is a wrapper around try_parse_address
    // it will parse the address and ssid from the address_string
    // and create an address type.

    std::string address_no_ssid;
    int ssid = 0;
    bool mark = false;

    if (!try_parse_address_with_used_flag(address_string, address_no_ssid, ssid, mark))
    {
        return false;
    }

    address.text = address_no_ssid;
    address.ssid = ssid;
    address.length = address_string.size();
    address.mark = mark;

    return true;
}

APRS_ROUTER_INLINE bool try_parse_address(std::string_view address, std::string& address_no_ssid, int& ssid)
{
    // Try parse an address like: ADDRESS[-SSID]
    //
    // Example:
    //
    // CALL1-10
    // ~~~~~ ~~
    // ^     ssid = 10
    // |
    // address_no_ssid = CALL1
    // 
    // This functions expects a valid AX.25 address,
    // and will return false if the address is not valid.
    // An address with a non numeric ssid will be rejected, ex: CALL-AB

    ssid = 0;

    if (address.empty() || address.size() > 9)
    {
        return false;
    }

    auto sep_position = address.find("-");

    if (sep_position != std::string_view::npos)
    {
        // Check few error conditions
        // If packet ends with a separator but no ssid, ex: "CALL-"
        // If there are more than 2 character after the separator, ex: CALL-123
        if ((sep_position == (address.size() - 1)) || ((sep_position + 3) < address.size()))
        {
            return false;
        }

        address_no_ssid = address.substr(0, sep_position);

        std::string ssid_string = std::string(address.substr(sep_position + 1));

        if (ssid_string[0] == '0')
        {
            return false;
        }

        // Ensure the ssid is a number
        if (!std::isdigit(static_cast<unsigned char>(ssid_string[0])) ||
            (ssid_string.size() > 1 && !std::isdigit(static_cast<unsigned char>(ssid_string[1]))))
        {
            return false;
        }

        if (!try_parse_int(ssid_string, ssid))
        {
            return false;
        }

        if (ssid < 0 || ssid > 15)
        {
            ssid = 0;
            return false;
        }
    }
    else
    {
        address_no_ssid = address;
        ssid = 0;
    }

    if (address_no_ssid.size() > 6)
    {
        return false;
    }

    for (char c : address_no_ssid)
    {
        // The address has to be alphanumeric and uppercase, or a digit
        if ((!std::isalnum(static_cast<unsigned char>(c)) || !std::isdigit(static_cast<unsigned char>(c))) &&
            !std::isupper(static_cast<unsigned char>(c)))
        {
            return false;
        }
    }

    return true;
}

APRS_ROUTER_INLINE bool try_parse_address_with_used_flag(std::string_view address, std::string& address_no_ssid, int& ssid)
{
    bool mark = false;
    return try_parse_address_with_used_flag(address, address_no_ssid, ssid, mark);
}

APRS_ROUTER_INLINE bool try_parse_address_with_used_flag(std::string_view address, std::string& address_no_ssid, int& ssid, bool& mark)
{
    ssid = 0;
    mark = false;

    if (address.empty())
    {
        return false;
    }

    if (address.back() == '*')
    {
        mark = true;
        address.remove_suffix(1);
    }

    return try_parse_address(address, address_no_ssid, ssid);
}

APRS_ROUTER_INLINE bool try_parse_int(std::string_view str, int& value)
{
    // Attempt to parse an integer from the given string_view.
    // Returns true if parsing is successful, false otherwise.
    // If parsing fails, the value is set to 0.

    auto result = std::from_chars(str.data(), str.data() + str.size(), value);

    // Check if the parsing was successful and if the entire string was consumed
    // The result.ec should be std::errc() and result.ptr should point to the end of the string    
    bool success = (result.ec == std::errc()) && (result.ptr == (str.data() + str.size()));

    // If parsing fails, set value to 0
    if (!success)
    {
        value = 0;
    }

    return success;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// ADDRESSES                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE void init_addresses(route_state& state)
{
    // Initialize addresses
    //
    // Addresses: N0CALL>APRS,CALLA,CALLB,WIDE1-1,WIDE2-2:data
    //                        ~~~~~ ~~~~~ ~~~~~~~ ~~~~~~~
    //
    // n-N addresses: N0CALL>APRS,CALLA,CALLB,WIDE1-1,WIDE2-2:data
    //                                        ~~~~~~~ ~~~~~~~
    //
    // Explicit addresses: N0CALL>APRS,CALLA,CALLB,WIDE1-1,WIDE2-2:data
    //                                 ~~~~~ ~~~~~
    //
    // Parse the router's address and path
    //
    // Router address:            DIGI                               state.settings.address   state.router_address
    // Router path:               WIDE1-1,WIDE2-2,CALLA,CALLB        state.settings.path      router_addresses
    // Router n-N addresses:      WIDE1-1,WIDE2-2                                             state.router_n_N_addresses
    // Router explicit addresses: CALLA,CALLB                                                 state.router_explicit_addresses
    // Packet:                    N0CALL>APRS,WIDE1,WIDE2-2:data                              state.packet_from_address, state.packet_to_address, state.packet_path
    // Packet addresses:          WIDE1,WIDE2-2                                               state.packet_addresses

    const std::string_view packet_from_address = state.packet_from_address;
    const std::string_view packet_to_address = state.packet_to_address;
    const internal_vector_t<std::string>& packet_path = state.packet_path;
    const std::string& router_address_string = state.settings->get().address;
    struct address& router_address = state.router_address;
    const std::vector<std::string>& explicit_addresses = state.settings->get().explicit_addresses;
    internal_vector_t<struct address>& router_explicit_addresses = state.router_explicit_addresses;
    const std::vector<std::string>& n_N_addresses = state.settings->get().n_N_addresses;
    internal_vector_t<struct address>& router_n_N_addresses = state.router_n_N_addresses;
    internal_vector_t<struct address>& packet_addresses = state.packet_addresses;

    try_parse_address_with_ssid(router_address_string, router_address);

    size_t index = 0;

    // Parse explicit addresses, ex: CALLA,CALLB,CALLC
    // Use try_parse_address_with_ssid to parse the address as we expect it to be in the format ADDRESS[-N]

    for (const auto& address_string : explicit_addresses)
    {
        struct address address;
        if (try_parse_address_with_ssid(address_string, address))
        {
            address.index = index;
            router_explicit_addresses.push_back(address);
        }
        index++;
    }

    // Parse n-N addresses, ex: WIDE1-1,WIDE2-2,WIDE3
    // Use try_parse_n_N_address to parse the address as we expect it to be in the format ADDRESSn[-N]

    index = 0;

    for (const auto& address_n_N_string : n_N_addresses)
    {
        struct address address_n_N;
        if (try_parse_n_N_address(address_n_N_string, address_n_N))
        {
            address_n_N.index = index;
            router_n_N_addresses.push_back(address_n_N);
        }
        index++;
    }

    // Parse the packet addresses
    // Based on whether an address appears in the router's explicit or n-N addresses list
    // decide whether we use try_parse_address_with_ssid or try_parse_n_N_address to parse the packed address
    //
    // Router address: DIGI
    //                 ~~~~
    // Router explicit addresses: CALLA,CALLB,CALLC
    //                            ~~~~~ ~~~~~ ~~~~~
    // Router n-N addresses: WIDE2,WIDE3
    //                       ----- -----
    // Packet: N0CALL>APRS,CALLA,CALLB*,WIDE2-1,WIDE3-2,DIGI,CALLC:data
    //                     ~~~~~ ~~~~~  ------- ------- ~~~~ ~~~~~
    // Addresses parsed with try_parse_address_with_ssid: CALLA,CALLB,DIGI,CALLC
    //                                                    ~~~~~ ~~~~~ ~~~~ ~~~~~
    // Addresses parsed with try_parse_n_N_address: WIDE2-1,WIDE3-2
    //                                              ------- -------  

    packet_addresses.reserve(packet_path.size());

    index = 0;

    for (const auto& packet_address_string : packet_path)
    {
        bool found = false;

        for (const auto& router_explicit_address : router_explicit_addresses)
        {
            struct address packet_explicit_address;

            if (try_parse_address_with_ssid(packet_address_string, packet_explicit_address))
            {
                if ((packet_explicit_address.ssid == router_explicit_address.ssid && packet_explicit_address.text == router_explicit_address.text) ||
                    (router_address.ssid == router_explicit_address.ssid && router_address.text == router_explicit_address.text))
                {
                    packet_explicit_address.index = index;
                    packet_addresses.push_back(packet_explicit_address);
                    found = true;
                    break;
                }
            }
        }

        if (found)
        {
            index++;
            continue;
        }

        for (const auto& router_n_N_address : router_n_N_addresses)
        {
            struct address packet_n_N_adress;

            if (try_parse_n_N_address(packet_address_string, packet_n_N_adress))
            {
                if (packet_n_N_adress.n == router_n_N_address.n && packet_n_N_adress.text == router_n_N_address.text)
                {
                    packet_n_N_adress.index = index;
                    packet_addresses.push_back(packet_n_N_adress);
                    found = true;
                    break;
                }
            }
        }

        if (found)
        {
            index++;
            continue;
        }

        struct address packet_address;

        if (try_parse_address(packet_address_string, packet_address))
        {
            packet_address.index = index;
            packet_addresses.push_back(packet_address);
        }

        index++;
    }

    set_addresses_offset(packet_from_address, packet_to_address, packet_addresses);
}

APRS_ROUTER_INLINE void unset_all_used_addresses(internal_vector_t<address>& packet_addresses, size_t offset, size_t count)
{
    unset_all_used_addresses(packet_addresses, offset, count, std::nullopt);
}

APRS_ROUTER_INLINE void unset_all_used_addresses(internal_vector_t<address>& packet_addresses, size_t offset, size_t count, std::optional<size_t> maybe_ignore_index)
{
    // Unset all addresses marked as used inside a packet path
    //
    // Packet: N0CALL>APRS,CALLA*,CALLB,CALLC*,CALLD,WIDE1-2:data
    //                     ~~~~~~       ~~~~~~
    // Will be updated to: N0CALL>APRS,CALLA,CALLB,CALLC,CALLD,WIDE1-2:data

    assert(offset < packet_addresses.size());
    assert(count <= packet_addresses.size());

    size_t ignore_index = maybe_ignore_index.value_or(packet_addresses.size()); // intentionally outside bounds if not set

    size_t address_offset = packet_addresses[offset].offset;

    for (size_t i = offset, n = 0; i < packet_addresses.size() && n < count; i++, n++)
    {
        if (packet_addresses[i].mark && i != ignore_index)
        {
            packet_addresses[i].mark = false;
            packet_addresses[i].length--;
        }
        packet_addresses[i].offset = address_offset;
        address_offset += packet_addresses[i].length + 1; // +1 for the comma address separator ','
    }
}

APRS_ROUTER_INLINE void set_address_as_used(internal_vector_t<address>& packet_addresses, size_t index)
{
    // Mark an address at "index" as used: N0CALL>APRS,CALLA,CALLB,CALLC,CALLD,WIDE1-2:data
    //
    // If index is "2" packet will be updated to: N0CALL>APRS,CALLA,CALLB,CALLC*,CALLD,WIDE1-2:data
    //                                                                    ~~~~~~

    assert(index < packet_addresses.size());
    set_address_as_used(packet_addresses, packet_addresses[index]);
}

APRS_ROUTER_INLINE void set_address_as_used(internal_vector_t<address>& packet_addresses, address& packet_address)
{
    unset_all_used_addresses(packet_addresses, 0, packet_addresses.size());

    if (packet_address.mark)
    {
        return;
    }

    packet_address.mark = true;
    packet_address.length++;

    if (packet_addresses.size() > 1)
    {
        for (size_t i = packet_address.index, address_offset = packet_address.offset; i < packet_addresses.size(); i++)
        {
            packet_addresses[i].offset = address_offset;
            address_offset += packet_addresses[i].length + 1; // +1 for the comma address separator ','
        }
    }
}

APRS_ROUTER_INLINE void update_addresses_index(internal_vector_t<address>& addresses)
{
    for (size_t i = 0; i < addresses.size(); i++)
    {
        addresses[i].index = i;
    }
}

APRS_ROUTER_INLINE void set_addresses_offset(std::string_view packet_from_address, std::string_view packet_to_address, internal_vector_t<address>& addresses)
{
    // +1 to account for the path separator ',', +1 to account for '>' separator
    //
    // Example:
    //
    // N0CALL>APRS,WIDE2-1:data
    //       ~    ~
    //      +1   +1
    // ~~~~~~~~~~~~ - offset

    size_t offset = packet_from_address.size() + packet_to_address.size() + 2;
    update_addresses_offset(addresses, offset);
}

APRS_ROUTER_INLINE void update_addresses_offset(internal_vector_t<address>& addresses, size_t initial_offset)
{
    // Updates addresses offsets, useful after an address was inserted, replaced or removed
    // This function does not update the length of addresses, and assumes they are up to date
    // This function should not be used if an address is updated in a way that will change the length ex: marked as set
    // because the length will be innacurate and the offsets will not be correctly calculated
    //
    // Before:
    //
    // N0CALL>APRS,AB,ABC,ABCD:data
    // ~~~~~~~~~~~~
    // 0         12 - initial_offset
    //
    // N0CALL>APRS,AB,ABC,ABCD:data
    //             ~~ ~~~ ~~~~
    //             12 15  19  - Offsets
    //
    // After:
    //
    // N0CALL>APRS,ABCDE,AB,ABC,ABCD:data
    //             ~~~~~ ~~ ~~~ ~~~~
    //             12    18 21  25  - Offsets
    //               ^
    //             Packet address inserted
    //
    // Offsets updated for addresses: "AB", "ABC", and "ABCD"
    // due to the inserted address "ABCDE"

    for (size_t i = 0, offset = initial_offset; i < addresses.size(); i++)
    {
        address& address = addresses[i];
        address.offset = offset;
        offset += address.length + 1; // +1 for the comma address separator ','
    }
}

APRS_ROUTER_INLINE void update_addresses_offset(internal_vector_t<address>& addresses)
{
    assert(addresses.size() > 0);
    size_t initial_offset = addresses[0].offset;
    update_addresses_offset(addresses, initial_offset);
}

APRS_ROUTER_INLINE bool try_insert_address(internal_vector_t<address>& packet_addresses, size_t index, std::string_view inserted_address_string)
{
    // Insert an address at "index" in the packet path
    //
    // Packet: N0CALL>APRS,CALLA,CALLB,CALLC,CALLD,CALLE:data
    //                                 ^
    //                                 |
    //                                 index = 2
    // 
    // inserted_address_string: "DIGI"
    //
    // Updated packet: N0CALL>APRS,CALLA,CALLB,DIGI,CALLC,CALLD,CALLE:data
    //                                         ~~~~

    assert(index < packet_addresses.size());

    if (packet_addresses.size() >= 8)
    {
        return false;
    }

    address new_address;
    new_address.text = inserted_address_string;
    new_address.length = inserted_address_string.size();

    assert(packet_addresses.size() < 8);

    size_t initial_offset = packet_addresses[0].offset;

    packet_addresses.insert(packet_addresses.begin() + index, new_address);

    update_addresses_index(packet_addresses);
    update_addresses_offset(packet_addresses, initial_offset);

    return true;
}

APRS_ROUTER_INLINE void replace_address_with_router_address(struct address& address, const struct address& router_address)
{
    // Replace an address with the router's address
    //
    // If the router's address is "DIGI", and the address is "WIDE2", the address will be updated to "DIGI"
    //
    // A packet "N0CALL>APRS,CALLA*,CALLB,CALLC,WIDE2:data"
    //                                          ~~~~~
    // 
    // will be updated to "N0CALL>APRS,CALLA*,CALLB,CALLC,DIGI:data"
    //                                                    ~~~~

    address.text = router_address.text;
    address.length = router_address.length;
    address.ssid = router_address.ssid;
    address.n = 0;
    address.N = 0;
}

APRS_ROUTER_INLINE bool try_move_address_to_position(internal_vector_t<address>& packet_addresses, size_t from_index, size_t to_index)
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
    assert(from_index != to_index);

    if (from_index >= packet_addresses.size() ||
        to_index >= packet_addresses.size())
    {
        return false;
    }

    if (from_index == to_index)
    {
        return false;
    }

    address address = packet_addresses[from_index];

    size_t initial_offset = packet_addresses[0].offset;

    packet_addresses.erase(packet_addresses.begin() + from_index);

    assert(packet_addresses.size() < 8);

    packet_addresses.insert(packet_addresses.begin() + to_index, address);

    update_addresses_index(packet_addresses);
    update_addresses_offset(packet_addresses, initial_offset);

    return true;
}

APRS_ROUTER_INLINE bool try_truncate_address_range(internal_vector_t<address>& packet_addresses, size_t start_index, size_t end_index)
{
    // Truncate a range of addresses, typically used for "preempt_truncate"
    //
    // If the router's address is "CALLD", a packet "N0CALL>APRS,CALLA*,CALLB,CALLC,CALLD,CALLE:data"
    //                                                                  ~~~~~~~~~~~~~~~~~
    //
    // will be truncated of the "CALLB,CALLC,CALLD" addresses in the path and become:
    //
    // "N0CALL>APRS,CALLA,CALLD*,CALLE:data"
    //                    ~~~~~~
    //
    // The digipeated marker and re-insertion of "CALLD" isn't done by this function

    assert(start_index < packet_addresses.size());
    assert(end_index < packet_addresses.size());

    if (start_index >= packet_addresses.size() ||
        end_index >= packet_addresses.size() ||
        start_index >= end_index)
    {
        return false;
    }

    struct address address = packet_addresses[end_index];

    size_t initial_offset = packet_addresses[0].offset;

    packet_addresses.erase(packet_addresses.begin() + start_index, packet_addresses.begin() + end_index + 1);

    assert(packet_addresses.size() < 8);

    packet_addresses.insert(packet_addresses.begin() + start_index, address);

    update_addresses_index(packet_addresses);
    update_addresses_offset(packet_addresses, initial_offset);

    return true;
}

APRS_ROUTER_INLINE bool try_truncate_empty_addresses(route_state& state)
{
    // Truncate all empty addresses in the packet path
    //
    // Packet: N0CALL>APRS,,CALLA*,CALLB,,CALLD,CALLE:data
    //                     ~             ~
    // Will be updated to: N0CALL>APRS,CALLA*,CALLB,CALLD,CALLE:data

    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    const bool enable_diagnostics = state.settings->get().enable_diagnostics;
    internal_vector_t<routing_diagnostic>& actions = state.actions;

    for (size_t i = 0; i < packet_addresses.size();)
    {
        if (packet_addresses[i].text.empty())
        {
            push_address_removed_diagnostic(packet_addresses, i, enable_diagnostics, actions);
            packet_addresses.erase(packet_addresses.begin() + i);
        }
        else
        {
            i++;
        }
    }

    update_addresses_index(packet_addresses);
    update_addresses_offset(packet_addresses);

    return true;
}

APRS_ROUTER_INLINE bool try_substitute_complete_n_N_address(route_state& state, size_t packet_n_N_address_index)
{
    // If the last n-N hop has been exhausted, replace the hop with the router's address
    //
    // Example:
    //
    // Input:  FROM>TO,WIDE1-1,WIDE2-1:data
    // Output: FROM>TO,DIGI*,WIDE2-1:data - replace WIDE1 (after decrementing) with DIGI

    internal_vector_t<address>& packet_addresses = state.packet_addresses;
    const std::string_view router_address = state.settings->get().address;
    const bool enable_diagnostics = state.settings->get().enable_diagnostics;
    internal_vector_t<routing_diagnostic>& actions = state.actions;

    assert(packet_n_N_address_index < packet_addresses.size());

    address& address = packet_addresses[packet_n_N_address_index];

    if (address.N == 0)
    {
        push_address_replaced_diagnostic(packet_addresses, address.index, router_address, enable_diagnostics, actions);

        address.text = router_address;
        address.length = router_address.size();
        address.N = 0;
        address.n = 0;
        address.kind = address_kind::other;

        push_address_unset_diagnostic(packet_addresses, address.index, enable_diagnostics, actions);
        set_address_as_used(packet_addresses, address);
        push_address_set_diagnostic(packet_addresses, address.index, enable_diagnostics, actions);

        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool try_decrement_n_N_address(address& n_N_address)
{
    // Decrements an n-N address
    //
    // Example: WIDE2-2 becomes WIDE2-1
    //              ~~~             ~~~

    assert(n_N_address.N > 0);

    if (n_N_address.N > 0)
    {
        n_N_address.N--;
        if (n_N_address.N == 0)
        {
            n_N_address.length -= 2; // as '-N' is absent
        }
        return true;
    }

    return false;
}

APRS_ROUTER_INLINE bool try_decrement_n_N_address(route_state& state, struct address& address)
{
    // Decrements an n-N address, while updating the offsets of the addresses

    bool result = try_decrement_n_N_address(address);
    if (result)
    {
        update_addresses_offset(state.packet_addresses);
    }
    return result;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// ADDRESS SEARCH                                                   //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE std::optional<std::pair<size_t, size_t>> find_first_unused_n_N_address_index(const internal_vector_t<address>& packet_addresses, const internal_vector_t<address>& router_n_N_addresses, routing_option options)
{
    // Find the first unused n-N address inside the packet
    // using the router's matching addresses
    //
    // Packet: N0CALL>APRS,WIDE1,WIDE2-2:data
    //                           ~~~~~~~
    //
    // Router addresses: WIDE1,WIDE2,WIDE3-2
    //                         ~~~~~
    //
    // Found addresses in packet: WIDE2-2
    //
    // If "reject_limit_exceeding_n_N_address" is set, n-N addresses with excessive hops with
    // be ignored.
    //
    // Router addresses: WIDE1,WIDE2,WIDE3-2
    //                               ~~~~~~~
    //                               j = 2
    //
    // Packet: N0CALL>APRS,CALL*,WIDE3-3:data
    //                           ~~~~~~~
    //                           i = 1
    //
    // Found addresses pair: { 1, 2 }

    bool reject_limit_exceeding_n_N_address = enum_has_flag(options, routing_option::reject_limit_exceeding_n_N_address);

    size_t i = 0;
    for (const auto& address : packet_addresses)
    {
        size_t j = 0;
        for (const auto& p : router_n_N_addresses)
        {
            if (address.n == p.n && address.N > 0 && address.text == p.text)
            {
                if (reject_limit_exceeding_n_N_address && p.N > 0 && address.N > p.N)
                {
                    j++;
                    continue;
                }
                return std::make_pair(i, j);
            }
            j++;
        }
        i++;
    }

    return {};
}

APRS_ROUTER_INLINE std::optional<size_t> find_last_used_address_index(const internal_vector_t<address>& packet_addresses, const internal_vector_t<address>& router_n_N_addresses, routing_option options)
{
    // Find the last address that has been marked as "used" in the packet path.
    // For example, if the packet is: FROM>TO,CALL*,TEST,ADDRESS*,WIDE1-2:data
    //                                                   ~~~~~~~~
    // the last used address will be "ADDRESS".

    std::optional<size_t> last_used_address_index;

    for (int i = static_cast<int>(packet_addresses.size()) - 1; i >= 0; i--)
    {
        if (packet_addresses[i].mark)
        {
            last_used_address_index = static_cast<size_t>(i);
            break;
        }
    }

    // Special handling for skip_complete_n_N_address
    //
    // There might be packets with unset but completed n-N addresses in the path.
    // We'd like to be able to route them if the skip_complete_n_N_address condition is set.
    // To do so, we consider them as "used" and progress in finding the last used address.
    //
    // Example:
    // 
    // Router address: DIGI
    // 
    // Router n-N addresses: WIDE1,WIDE2
    //
    // Packet: N0CALL>APRS,WIDE1,WIDE2-2:data
    //
    // Routed packet: N0CALL>APRS,WIDE1,DIGI*,WIDE2-1:data

    bool skip_complete_n_N_address = enum_has_flag(options, routing_option::skip_complete_n_N_address);

    if (skip_complete_n_N_address)
    {
        size_t offset = last_used_address_index.value_or(0);

        for (size_t i = offset; i < packet_addresses.size(); i++)
        {
            const auto& address = packet_addresses[i];

            for (const auto& p : router_n_N_addresses)
            {
                if (address.n == p.n && address.N == 0 && address.text == p.text)
                {
                    last_used_address_index = i;
                    break;
                }
            }
        }
    }

    return last_used_address_index;
}

APRS_ROUTER_INLINE std::optional<size_t> find_router_address_index(const internal_vector_t<address>& packet_addresses, size_t offset, const address& router_address, const internal_vector_t<address>& router_explicit_addresses)
{
    // Find an address in the packet, matching the router's address or an address in the router's path.
    //
    // Examples:
    //
    // Router address: DIGI
    //                 ~~~~
    // Packet: FROM>TO,CALL,ROUTE,DIGI,WIDE2-1:data
    //                            ~~~~
    //
    // Offset: 1
    // Router addresses: CALL,ROUTE 
    //                        ~~~~~
    // Packet: FROM>TO,CALL*,ROUTE,DIGI,WIDE2-1:data
    //                       ~~~~~
    //
    // Because we are comparing the addresses in the packet with the router's address and the router's path,
    // the packet addresses cannot be an n-N address (n > 0).

    assert(offset < packet_addresses.size());

    for (size_t i = offset; i < packet_addresses.size(); i++)
    {
        if (equal_addresses_ignore_mark(packet_addresses[i], router_address))
        {
            return i;
        }

        for (size_t j = 0; j < router_explicit_addresses.size(); j++)
        {
            if (equal_addresses_ignore_mark(packet_addresses[i], router_explicit_addresses[j]))
            {
                return i;
            }
        }
    }

    return {};
}

APRS_ROUTER_INLINE std::optional<size_t> find_unused_router_address_index(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, const address& router_address, const internal_vector_t<address>& router_explicit_addresses)
{
    // Find unused address mathing router's address or an address in the router's path.
    // Start the search from the last used address.
    //
    // FROM>TO,CALL,ROUTE*,DIGI,WIDE2-1:data - if explicit routing by router address
    //                     ~~~~
    //
    // FROM>TO,CALL,ROUTE*,WIDE,DIGI,WIDE2-1:data - if explicit routing by router explicit addresses
    //                     ~~~~
    //
    // FROM>TO,CALL,ROUTE*,DIGI,WIDE2-1:data - if n-N routing
    //                          ~~~~~~~

    size_t start_search_address_index = maybe_last_used_address_index.value_or(0);

    assert(start_search_address_index < packet_addresses.size());

    std::optional<size_t> maybe_address_index = find_router_address_index(
        packet_addresses,
        start_search_address_index,
        router_address,
        router_explicit_addresses);

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

APRS_ROUTER_INLINE void find_used_addresses(route_state& state)
{
    // Router addresses: DIGI,WIDE1
    //
    // Last used address: N0CALL>APRS,CALL*,DIGI,WIDE1,ROUTE,WIDE2-2:data 
    //                                ~~~~~
    //
    // Router address: N0CALL>APRS,CALL*,DIGI,WIDE1,ROUTE,WIDE2-2:data
    //                                   ~~~~
    //
    // Unused address: N0CALL>APRS,CALL*,DIGI,WIDE1,ROUTE,WIDE2-2:data
    //                                   ~~~~

    state.maybe_last_used_address_index = find_last_used_address_index(state.packet_addresses, state.router_n_N_addresses, state.settings->get().options);
    state.maybe_router_address_index = find_unused_router_address_index(state.packet_addresses, state.maybe_last_used_address_index, state.router_address, state.router_explicit_addresses);
    state.unused_address_index = state.maybe_last_used_address_index.value_or(-1) + 1;

    if (state.maybe_router_address_index)
    {
        // Compare the two addresses to determine if the packet is being routed by the router's address
        state.is_path_based_routing = !equal_addresses_ignore_mark(state.packet_addresses[state.maybe_router_address_index.value()], state.router_address);
    }
}

APRS_ROUTER_INLINE bool has_address(const internal_vector_t<address>& addresses, size_t offset, struct address address)
{
    // Check if the address is present in the list of 'addresses'
    // starting from the 'offset'

    address.mark = false;
    for (size_t i = offset; i < addresses.size(); i++)
    {
        if (equal_addresses_ignore_mark(addresses[i], address))
        {
            return true;
        }
    }
    return false;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// PACKET QUERIES                                                   //
//                                                                  //
//                                                                  //
// **************************************************************** //

APRS_ROUTER_INLINE bool is_packet_valid(std::string_view packet_from_address, std::string_view packet_to_address, const internal_vector_t<std::string>& packet_path, routing_option options)
{
    // Performs various checks on the packet.
    //
    // The packet is valid if:
    //
    //   - the 'from' address is not empty
    //   - the 'to' address is not empty
    //   - the 'path' is not empty and has less than 8 addresses
    //   - each address ('from', 'to', 'path') is valid if:
    //     - the address is alphanumeric
    //     - the address is uppercase
    //     - the address is not longer than 9 characters with ssid
    //     - the address is not longer than 6 characters without ssid
    //     - the ssid if present us a number in the range of 1-15
    //     - the address might containd the used '*' flag at the end
    //
    // NOTE: we are not checking whether data is valid, as it is not the router's job
    // we only operate on the 'from', 'to' and 'path' addresses

    if (packet_from_address.empty() || packet_to_address.empty())
    {
        return false;
    }

    if (packet_path.empty() || packet_path.size() > 8)
    {
        return false;
    }

    if (!enum_has_flag(options, routing_option::strict))
    {
        return true;
    }

    std::string callsign;
    int ssid;

    if (!try_parse_address_with_used_flag(packet_from_address, callsign, ssid) ||
        !try_parse_address_with_used_flag(packet_to_address, callsign, ssid))
    {
        return false;
    }

    for (const auto& p : packet_path)
    {
        if (!try_parse_address_with_used_flag(p, callsign, ssid))
        {
            return false;
        }
    }

    return true;
}

APRS_ROUTER_INLINE bool is_packet_valid(const route_state& state)
{
    return is_packet_valid(state.packet_from_address, state.packet_to_address, state.packet_path, state.settings->get().options);
}

APRS_ROUTER_INLINE bool is_valid_router_address_and_packet(const route_state& state)
{
    return state.settings->get().address.empty() || !is_packet_valid(state);
}

APRS_ROUTER_INLINE bool is_packet_from_us(std::string_view packet_from_address, std::string_view router_address)
{
    // Router address: DIGI
    //
    // Packet has ben sent by us: DIGI>APRS,CALL,WIDE1-1,WIDE2-2:data
    //                            ~~~~
    return packet_from_address == router_address;
}

APRS_ROUTER_INLINE bool is_packet_from_us(const route_state& state)
{
    // Router address: DIGI
    //
    // Packet has ben sent by us: DIGI>APRS,CALL,WIDE1-1,WIDE2-2:data
    //                            ~~~~
    return is_packet_from_us(state.packet_from_address, state.settings->get().address);
}

APRS_ROUTER_INLINE bool is_packet_sent_to_us(std::string_view packet_to_address, std::string_view router_address)
{
    // Router address: DIGI
    //
    // Packet: N0CALL>DIGI,CALL,WIDE1-1,WIDE2-2:data
    //                ~~~~
    return packet_to_address == router_address;
}

APRS_ROUTER_INLINE bool is_packet_sent_to_us(const route_state& state)
{
    // Router address: DIGI
    //
    // Packet: N0CALL>DIGI,CALL,WIDE1-1,WIDE2-2:data
    //                ~~~~
    return is_packet_sent_to_us(state.packet_to_address, state.settings->get().address);
}

APRS_ROUTER_INLINE bool has_packet_routing_ended(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_last_used_address_index)
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

APRS_ROUTER_INLINE bool has_packet_routing_ended(const route_state& state)
{
    // Packet has finished routing: N0CALL>APRS,CALL,WIDE1,DIGI*:data
    //                                                     ~~~~~
    return has_packet_routing_ended(state.packet_addresses, state.maybe_last_used_address_index);
}

APRS_ROUTER_INLINE bool has_packet_been_routed_by_us(const internal_vector_t<address>& packet_addresses, std::optional<size_t> maybe_last_used_address_index, const address& router_address)
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

    const address& last_used_address = packet_addresses[last_used_address_index];

    return ((last_used_address.text == router_address.text && last_used_address.ssid == router_address.ssid) &&
            last_used_address.mark);
}

APRS_ROUTER_INLINE bool has_packet_been_routed_by_us(route_state& state)
{
    // Packet has already been routing by us: N0CALL>APRS,CALL,DIGI*,WIDE1-1,WIDE2-2:data
    //                                                         ~~~~~
    return has_packet_been_routed_by_us(state.packet_addresses, state.maybe_last_used_address_index, state.router_address);
}

#endif // APRS_ROUTER_PUBLIC_FORWARD_DECLARATIONS_ONLY

APRS_ROUTER_NAMESPACE_END

APRS_ROUTER_NAMESPACE_END