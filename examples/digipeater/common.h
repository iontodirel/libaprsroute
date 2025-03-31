#pragma once

#include <string>
#include <chrono>
#include <exception>

#include "external/aprsroute.hpp"

// **************************************************************** //
//                                                                  //
//                                                                  //
// to_string                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

std::string to_string(bool b);

// **************************************************************** //
//                                                                  //
//                                                                  //
// date_time                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

struct date_time
{
    int year = -1;
    int month = -1;
    int day = -1;
    int hour = -1;
    int minute = -1;
    int second = -1;
};

date_time get_local_time();
date_time get_utc_time();

std::string to_string(date_time time);

// **************************************************************** //
//                                                                  //
//                                                                  //
// exception                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

enum class error_code
{
    connectivity,
    io,
    login,
    other,
    gnss,
    argument,
    file_not_found,
    port_not_found,
    parsing,
    library,
    none
};

struct exception : public std::exception
{
public:
    exception();
    exception(enum error_code e);
    exception(enum error_code code, const std::string& message);
    exception(const std::string& message);

    enum error_code code() const;
    const char* what() const noexcept override;

private:
    enum error_code code_ = error_code::other;
    std::string message_;
};

// **************************************************************** //
//                                                                  //
//                                                                  //
// stopwatch                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

struct stopwatch
{
    void start();
    void stop();
    unsigned long long elapsed_ms() const;

    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time_;
};

// **************************************************************** //
//                                                                  //
//                                                                  //
// packet_size_bytes                                                //
//                                                                  //
//                                                                  //
// **************************************************************** //

size_t packet_size_bytes(const aprs::router::packet& p);

// **************************************************************** //
//                                                                  //
//                                                                  //
// generate_random_number                                           //
//                                                                  //
//                                                                  //
// **************************************************************** //

size_t generate_random_number(size_t min, size_t max);
