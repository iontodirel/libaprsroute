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

#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <string_view>

bool tracking_enabled = false;
size_t allocation_count = 0;
size_t allocation_bytes = 0;

void* operator new(size_t requested_bytes)
{
    void* allocated_pointer = std::malloc(requested_bytes ? requested_bytes : 1);
    if (!allocated_pointer) throw std::bad_alloc();
    if (tracking_enabled)
    {
        allocation_count++;
        allocation_bytes += requested_bytes;
    }
    return allocated_pointer;
}

void* operator new[](size_t requested_bytes)
{
    return ::operator new(requested_bytes);
}

void operator delete(void* allocated_pointer) noexcept { std::free(allocated_pointer); }
void operator delete[](void* allocated_pointer) noexcept { std::free(allocated_pointer); }
void operator delete(void* allocated_pointer, size_t) noexcept { std::free(allocated_pointer); }
void operator delete[](void* allocated_pointer, size_t) noexcept { std::free(allocated_pointer); }

#if defined(_MSC_VER)
#  define APRSROUTE_FORCE_INLINE __forceinline
#else
#  define APRSROUTE_FORCE_INLINE inline __attribute__((always_inline))
#endif

// Anti-DCE primitives. Same pattern the Google Benchmark and Folly use.
//
// Clang/GCC: empty inline asm with an "r,m" input constraint and a "memory" clobber.
// Emits no instructions; tells the compiler the value is read into a register or memory,
// and that any memory may have been touched, which prevents elision of the producing computation and any reordering across it.
//
// MSVC (x64): Force a volatile read of the first byte through a char-pointer alias,
// then a compile-only signal fence.
// The volatile load can't be optimized away, and the fence prevents reordering across it.
template<class T>
APRSROUTE_FORCE_INLINE void do_not_optimize(T const& value)
{
#if defined(__clang__) || defined(__GNUC__)
    asm volatile("" : : "r,m"(value) : "memory");
#elif defined(_MSC_VER)
    char const volatile sink = *reinterpret_cast<char const volatile*>(&value);
    (void)sink;
    std::atomic_signal_fence(std::memory_order_acq_rel);
#endif
}

#include "../aprsroute.hpp"

#if defined(_MSC_VER)
constexpr const char* compiler_name = "MSVC";
#elif defined(__clang__)
constexpr const char* compiler_name = "Clang";
#elif defined(__GNUC__)
constexpr const char* compiler_name = "GCC";
#else
constexpr const char* compiler_name = "Unknown";
#endif

#if defined(_WIN32)
constexpr const char* os_name = "Windows";
#elif defined(__linux__)
constexpr const char* os_name = "Linux";
#elif defined(__APPLE__)
constexpr const char* os_name = "macOS";
#else
constexpr const char* os_name = "Unknown";
#endif

static std::string platform_label()
{
    return std::string(os_name) + " " + compiler_name;
}

static std::string format_throughput(double pkts_per_sec)
{
    std::ostringstream oss;
    oss << std::fixed;
    if (pkts_per_sec >= 1'000'000.0)
    {
        oss << std::setprecision(2) << (pkts_per_sec / 1'000'000.0) << "M pkts/s";
    }
    else if (pkts_per_sec >= 1'000.0)
    {
        oss << std::setprecision(0) << (pkts_per_sec / 1'000.0) << "K pkts/s";
    }
    else
    {
        oss << std::setprecision(0) << pkts_per_sec << " pkts/s";
    }
    return oss.str();
}

static std::string format_route_time(double microseconds)
{
    std::ostringstream oss;
    oss << std::fixed;
    if (microseconds < 10.0)
    {
        oss << std::setprecision(2) << microseconds << " us";
    }
    else if (microseconds < 100.0)
    {
        oss << std::setprecision(1) << microseconds << " us";
    }
    else
    {
        oss << std::setprecision(0) << microseconds << " us";
    }
    return oss.str();
}

static void run_throughput_test()
{
    constexpr size_t packet_count = 10'000'000;

    const std::string_view packet_from = "N0CALL-10";
    const std::string_view packet_to   = "CALL-5";
    const std::string_view router_address = "DIGI";

    const std::array<std::string_view, 5> packet_path{ "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };
    const std::array<std::string_view, 0> explicit_addresses{};
    const std::array<std::string_view, 2> n_N_addresses{ "WIDE1-1", "WIDE2-1" };

    std::array<std::array<char, 10>, 8> routed_packet_path{};
    std::array<size_t, 8> routed_packet_path_address_sizes{};

    aprs::router::routing_state routing_state;
    aprs::router::route_state route_state;

    aprs::router::init_router(router_address, explicit_addresses.begin(), explicit_addresses.end(), n_N_addresses.begin(), n_N_addresses.end(), aprs::router::routing_option::none, route_state);

    std::cout << "Platform:        " << platform_label() << std::endl;
    std::cout << "Packet:          N0CALL-10>CALL-5,CALLA-10*,CALLB-5*,CALLC-15*,WIDE1*,WIDE2-1:data" << std::endl;
    std::cout << "Router address:  " << router_address << std::endl;
    std::cout << "Router path:     WIDE1-2, WIDE2-3" << std::endl;
    std::cout << "Diagnostics:     disabled" << std::endl;
    std::cout << "Iterations:      " << packet_count << std::endl;
    std::cout << std::endl;
    std::cout << "--- Begin routing loop ---" << std::endl;

    allocation_count = 0;
    allocation_bytes = 0;
    tracking_enabled = true;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < packet_count; ++i)
    {
        auto [routed_packet_path_end, routed_packet_path_address_sizes_end, routing_succeeded] = aprs::router::try_route_packet(
            packet_from, packet_to,
            packet_path.begin(), packet_path.end(),
            routed_packet_path.begin(),
            routed_packet_path_address_sizes.begin(),
            routing_state, route_state);

        // Anti-DCE: each call's outputs must be treated as observed
        // so the optimizer cannot elide the call or hoist it out of the loop.
        do_not_optimize(routing_succeeded);
        do_not_optimize(routed_packet_path_end);
        do_not_optimize(routed_packet_path_address_sizes_end);
    }

    auto end = std::chrono::high_resolution_clock::now();

    tracking_enabled = false;

    // Force the cumulative state to be observable too.
    // Covers any cross-iteration optimization that proves the arrays/state aren't read after the loop.
    do_not_optimize(routed_packet_path);
    do_not_optimize(routed_packet_path_address_sizes);
    do_not_optimize(routing_state);
    do_not_optimize(route_state);

    std::cout << "--- End routing loop ---" << std::endl;
    std::cout << std::endl;

    const double elapsed_us = std::chrono::duration<double, std::micro>(end - start).count();
    const double elapsed_ms = elapsed_us / 1000.0;
    const double elapsed_s  = elapsed_ms / 1000.0;
    const double avg_route_us = elapsed_us / static_cast<double>(packet_count);
    const double pkts_per_sec = static_cast<double>(packet_count) / elapsed_s;

    std::ostringstream memory_text;
    memory_text << allocation_bytes << " bytes, " << allocation_count << " allocations";

    std::cout << "Elapsed:         " << std::fixed << std::setprecision(2) << elapsed_s << " s ("
              << std::setprecision(0) << elapsed_ms << " ms)" << std::endl;
    std::cout << "Throughput:      " << format_throughput(pkts_per_sec) << std::endl;
    std::cout << "Routing time:    " << format_route_time(avg_route_us) << std::endl;
    std::cout << "Routing memory:  " << memory_text.str() << std::endl;
    std::cout << std::endl;

    std::cout << "README perf table row (fill hardware column):" << std::endl;
    std::cout << std::endl;
    std::cout << "| Platform     | Hardware                     | Throughput  | Routing time  | Routing memory (heap)      |" << std::endl;
    std::cout << "|--------------|------------------------------|-------------|---------------|----------------------------|" << std::endl;
    std::cout << "| " << std::left << std::setw(13) << platform_label()
              << "| " << std::setw(29) << "<fill hardware>"
              << "| " << std::setw(12) << format_throughput(pkts_per_sec)
              << "| " << std::setw(14) << format_route_time(avg_route_us)
              << "| " << std::setw(27) << memory_text.str() << "|" << std::endl;
}

int main()
{
    run_throughput_test();
    return 0;
}
