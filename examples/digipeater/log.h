#pragma once

#include "common.h"

#include <string>
#include <optional>
#include <memory>

#include "external/aprsroute.hpp"

enum class log_type
{
    error,
    warning,
    message,
};

std::string to_string(log_type type);

enum class log_verbosity : int
{
    quiet = 1,
    normal,
    verbose,
    debug
};

struct packet_entry; // forward declaration

enum class log_stage
{
    start_route,
    end_route,
    start_router,
    end_router,
    accept_packet,
    reject_packet,
    ignore_packet,
    transcode_packet,
    duplicate_packet,
    unconditional_accept_packet,
    update
};

struct log_entry
{
    log_verbosity verbosity = log_verbosity::normal;
    log_type type = log_type::message;
    std::string function_name;
    struct date_time date_time;
    std::string message;
    std::optional<aprs::router::packet> packet;
    std::unique_ptr<packet_entry> entry;
    std::unique_ptr<packet_entry> duplicate_entry;
    bool diagnostics = false;
};

struct logger_base
{
    virtual void log(const log_entry& entry) = 0;
};

struct basic_stdout_logger : public logger_base
{
    log_verbosity verbosity = log_verbosity::normal;

    void log(const log_entry& entry) override;
};
