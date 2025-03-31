#include "common.h"

#include <ctime>
#include <random>

#include <fmt/format.h>

// **************************************************************** //
//                                                                  //
//                                                                  //
// to_string                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

std::string to_string(bool b)
{
    return b ? "true" : "false";
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// date_time                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

date_time get_local_time()
{
    std::time_t now = std::time(nullptr);
    std::tm local_tm = *std::localtime(&now);

    return {
        local_tm.tm_year + 1900,
        local_tm.tm_mon + 1,
        local_tm.tm_mday,
        local_tm.tm_hour,
        local_tm.tm_min,
        local_tm.tm_sec
    };
}

date_time get_utc_time()
{
    std::time_t now = std::time(nullptr);
    std::tm utc_tm = *std::gmtime(&now);

    return {
        utc_tm.tm_year + 1900,
        utc_tm.tm_mon + 1,
        utc_tm.tm_mday,
        utc_tm.tm_hour,
        utc_tm.tm_min,
        utc_tm.tm_sec
    };
}

std::string to_string(date_time time)
{
    // 2024-06-26 05:08:56
    return fmt::format("{}-{:02}-{:02} {:02}:{:02}:{:02}", time.year, time.month, time.day, time.hour, time.minute, time.second);
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// exception                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

exception::exception()
{
}

exception::exception(enum error_code e) : code_(e)
{
}

exception::exception(enum error_code code, const std::string& message) : code_(code), message_(message)
{
}

exception::exception(const std::string& message) : code_(error_code::other), message_(message)
{
}

enum error_code exception::code() const
{
    return code_;
}

const char* exception::what() const noexcept
{
    return message_.c_str();
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// stopwatch                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

void stopwatch::start()
{
    start_time_ = std::chrono::high_resolution_clock::now();
}

void stopwatch::stop()
{
    end_time_ = std::chrono::high_resolution_clock::now();
}

unsigned long long stopwatch::elapsed_ms() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_time_ - start_time_).count();
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// packet_size_bytes                                                //
//                                                                  //
//                                                                  //
// **************************************************************** //

size_t packet_size_bytes(const aprs::router::packet& p)
{
    // N0CALL>APRS,CALL,WIDE1-3:data
    size_t size = p.from.size() + p.to.size() + p.data.size() + 2;
    for (const auto& path : p.path)
    {
        size += 1 + path.size();
    }
    return size;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// generate_random_number                                           //
//                                                                  //
//                                                                  //
// **************************************************************** //

size_t generate_random_number(size_t min, size_t max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(min, max);
    return dis(gen);
}