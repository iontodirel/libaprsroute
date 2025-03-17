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

#include "../aprsroute.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

using namespace aprs::router;
using namespace aprs::router::detail;

int main()
{
    auto start = std::chrono::high_resolution_clock::now();

    // Measurements made on 3/16/2025 (PST)
    // Commit: b0ea46f3deb4f5e07b6a7b35125ed40fcc22af34
    // 
    // Typically about 160K packets / second (Intel Celeron N5095, 8GB RAM) - Linux GCC
    // Typically about 168K packets / second (Intel Celeron N5095, 8GB RAM) - Linux Clang
    // Typically about 300K packets / second (Intel i9-14900HX, 97GB RAM) - Windows MSVC
    // Typically about 500K packets / second (Intel i9-14900HX, 97GB RAM) - Windows WSL GCC
    // Typically about 500K packets / second (Intel i9-14900HX, 97GB RAM) - Windows WSL Clang

    const size_t packet_count = 1000000;

    for (int i = 0; i < packet_count; i++)
    {
        routing_result result;
        router_settings digi;

        digi.address = "DIGI";
        digi.n_N_addresses = { "WIDE4" };

        packet p = { "N0CALL", "APRS", {"CALL", "WIDE1-3", "TEMP", "WIDE2-3", "ROUTE", "WIDE3-3", "ROUTER", "WIDE4-3"}, "The quick brown fox jumps over the lazy dog." };

        try_route_packet(p, digi, result);
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;

    double elapsed_ms = elapsed.count();
    double elapsed_seconds = elapsed_ms / 1000;
    double elapsed_minutes = elapsed_seconds / 60;
    double packets_per_ms = packet_count / elapsed_ms;
    double packets_per_second = packet_count / elapsed_seconds;

    std::cout << "Benchmark: " << elapsed_ms << " ms" << std::endl;
    std::cout << "Benchmark: " << std::fixed << std::setprecision(2) << elapsed_seconds << " seconds" << std::endl;
    std::cout << "Benchmark: " << std::fixed << std::setprecision(2) << elapsed_minutes << " minutes" << std::endl;
    std::cout << "Benchmark: " << packets_per_ms << " packets / ms" << std::endl;
    std::cout << "Benchmark: " << packets_per_second << " packets / second" << std::endl;

    return 0;
}