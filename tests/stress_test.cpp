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

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <algorithm>
#include <numeric>
#include <new>
#include <array>
#include <memory_resource>

#define APRSROUTE_USE_STACK_ALLOCATOR2
#define APRSROUTE_LOG_ALLOCATIONS

#ifdef APRSROUTE_USE_STACK_ALLOCATOR2

struct stack_buffer
{
    void reset() noexcept;

    std::byte* data;
    std::size_t size;
    std::size_t offset = 0;
};

inline void stack_buffer::reset() noexcept
{
    offset = 0;
}

inline stack_buffer* global_stack_buffer()
{
    static std::byte bytes[8192];
    static stack_buffer buffer { bytes, sizeof(bytes) };
    return &buffer;
}

template <typename T>
struct stack_allocator
{
    template <typename U> friend struct stack_allocator;

    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::false_type;

    stack_allocator() noexcept : buffer(global_stack_buffer())
    {
    }

    template <typename U>
    stack_allocator(const stack_allocator<U>& other) noexcept : buffer(other.buffer)
    {
    }

    pointer allocate(size_type n)
    {
        std::uintptr_t base = reinterpret_cast<std::uintptr_t>(buffer->data);
        std::uintptr_t curr = base + buffer->offset;
        std::uintptr_t aligned = (curr + alignof(T) - 1) & ~(alignof(T) - 1);
        std::size_t new_offset = aligned - base + n * sizeof(T);

        if (new_offset > buffer->size)
        {
            throw std::bad_alloc();
        }

        buffer->offset = new_offset;

        return reinterpret_cast<pointer>(buffer->data + (aligned - base));
    }

    void deallocate(pointer, size_type) noexcept
    {
    }

    template <typename U>
    bool operator==(const stack_allocator<U>& other) const noexcept
    {
        return buffer == other.buffer;
    }

    template <typename U>
    bool operator!=(const stack_allocator<U>& other) const noexcept
    {
        return !(*this == other);
    }

    stack_buffer* buffer;
};

#define APRS_ROUTER_USE_PMR 0
#define APRS_ROUTER_DEFINE_CUSTOM_TYPES

namespace aprs::router::detail
{
    template<class T>
    using internal_vector_t = std::vector<T, stack_allocator<T>>;

    template<class T>
    using internal_string_t = std::basic_string<T>;
}

#endif // APRSROUTE_USE_STACK_ALLOCATOR2

#ifdef APRSROUTE_LOG_ALLOCATIONS

void log_allocation(size_t size, void* ptr)
{
    std::cout << "Allocated " << size << " bytes at " << ptr << std::endl;
}

void* operator new(size_t size)
{
    void* ptr = malloc(size);
    if (!ptr)
    {
        throw std::bad_alloc();
    }
    log_allocation(size, ptr);
    return ptr;
}

void* operator new[](size_t size)
{
    void* ptr = malloc(size);
    if (!ptr)
    {
        throw std::bad_alloc();
    }
    log_allocation(size, ptr);
    return ptr;
}

#endif

#ifdef APRSROUTE_USE_STACK_ALLOCATOR1

#define APRS_ROUTER_USE_PMR 1

#endif

#include "../aprsroute.hpp"

using namespace aprs::router;
using namespace aprs::router::detail;

void run_try_route_packet_stress_test()
{
    constexpr size_t packet_count = 10'000'000;

#ifdef APRSROUTE_USE_STACK_ALLOCATOR1
    constexpr size_t pool_size = 4096;
    unsigned char buffer[pool_size];
    std::fill_n(buffer, pool_size, 0);
#endif

    aprs::router::router_settings settings{ "DIGI", {}, { "WIDE1-2", "WIDE2-3" }, aprs::router::routing_option::none, false };

    std::vector<std::string> original_packet_path{ "CALLA-10*", "CALLB-5*", "CALLC-15*", "WIDE1*", "WIDE2-1" };

    aprs::router::routing_state routing_state;

    std::vector<routing_diagnostic> routing_actions;

    std::vector<std::string> routed_packet_path;
    routed_packet_path.reserve(8);

    std::cout << "--- Begin routing loop ---" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < packet_count; i++)
    {
#ifdef APRSROUTE_USE_STACK_ALLOCATOR1
        std::pmr::monotonic_buffer_resource pool(buffer, pool_size, std::pmr::null_memory_resource());
        std::pmr::memory_resource* memory_resource = &pool;

        std::pmr::vector<std::string> routed_packet_path(memory_resource);

        aprs::router::try_route_packet("N0CALL-10", "CALL-5", original_packet_path.begin(), original_packet_path.end(), settings, std::back_inserter(routed_packet_path), routing_state, std::back_inserter(routing_actions), memory_resource);
#else
        routed_packet_path.clear();

        aprs::router::try_route_packet("N0CALL-10", "CALL-5", original_packet_path, settings, routed_packet_path, routing_state, routing_actions);
#endif

#ifdef APRSROUTE_USE_STACK_ALLOCATOR2
        global_stack_buffer()->reset();
#endif
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "--- End routing loop ---" << std::endl;

    std::chrono::duration<double, std::micro> elapsed_us = end - start;

    double elapsed_ms = elapsed_us.count() / 1000;
    double elapsed_seconds = elapsed_ms / 1000;
    double elapsed_minutes = elapsed_seconds / 60;
    double packets_per_ms = packet_count / elapsed_ms;
    double packets_per_second = packet_count / elapsed_seconds;

    double average_route_time = elapsed_us.count() / packet_count;

    std::cout << "Elapsed: " << elapsed_ms << " ms" << std::endl;
    std::cout << "Elapsed: " << std::fixed << std::setprecision(2) << elapsed_seconds << " seconds" << std::endl;
    std::cout << "Elapsed: " << std::fixed << std::setprecision(2) << elapsed_minutes << " minutes" << std::endl;
    std::cout << "Throughput: " << packets_per_ms << " packets / ms" << std::endl;
    std::cout << "Throughput: " << packets_per_second << " packets / second" << std::endl;
    std::cout << "Average route time: " << average_route_time << " us" << std::endl;
}

void run_address_equal_stress_test()
{
    constexpr size_t count = 100'000'000;

    volatile bool result = false;

    {
        address a1 { "WIDE1", 0, 0, 0 };
        address a2 { "WIDE", 1, 0, 0 };

        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < count; i++)
        {
            address a1_copy = a1;
            a1_copy.mark = false;
            result = to_string(a1_copy) == to_string(a2);
            assert(result);
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto result = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "to_string Elapsed: " << result.count() << " us" << std::endl;
        std::cout << "to_string Elapsed: " << result.count() / 1000.0 << " ms" << std::endl;
        std::cout << "to_string Elapsed: " << result.count() / 1000000.0 << " s" << std::endl;
    }

    {
        address a1 { "WIDE1", 0, 0, 0 };
        address a2 { "WIDE", 1, 0, 0 };

        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < count; i++)
        {
            result = equal_addresses_ignore_mark(a1, a2);
            assert(result);
        }

        auto end = std::chrono::high_resolution_clock::now();

        auto result = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "equal_addresses_ignore_mark Elapsed: " << result.count() << " us" << std::endl;
        std::cout << "equal_addresses_ignore_mark Elapsed: " << result.count() / 1000.0 << " ms" << std::endl;
        std::cout << "equal_addresses_ignore_mark Elapsed: " << result.count() / 1000000.0 << " s" << std::endl;
    }
}

int main()
{
    run_address_equal_stress_test();
    run_try_route_packet_stress_test();
    return 0;
}
