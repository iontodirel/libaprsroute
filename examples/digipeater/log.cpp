#include "log.h"

#include "digipeater.h"

#include <iostream>

#include <fmt/format.h>
#include <fmt/color.h>

// **************************************************************** //
//                                                                  //
//                                                                  //
// to_string                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

std::string to_string(log_type type)
{
    switch (type)
    {
        case log_type::error:   return "error";
        case log_type::warning: return "warning";
        case log_type::message: return "info";
    }
    return "unknown";
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// basic_stdout_logger                                              //
//                                                                  //
//                                                                  //
// **************************************************************** //

void basic_stdout_logger::log(const log_entry& entry)
{
    int entry_verbosity = static_cast<int>(entry.verbosity);
    int log_verbosity = static_cast<int>(verbosity);

    if (entry_verbosity > log_verbosity)
    {
        return;
    }

    fmt::print(fg(fmt::color::coral) | fmt::emphasis::bold, "{}\n", entry.message);

    fmt::print("\n");

    fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "type");

    switch (entry.type)
    {
        case log_type::error:   fmt::print(fg(fmt::color::red) | fmt::emphasis::italic, "{}\n", to_string(entry.type)); break;
        case log_type::warning: fmt::print(fg(fmt::color::orange) | fmt::emphasis::italic, "{}\n", to_string(entry.type)); break;
        case log_type::message: fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", to_string(entry.type)); break;
    }

    fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "function");
    fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", entry.function_name);

    fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "log time");
    fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", to_string(entry.date_time));

    if (entry.packet)
    {
        fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "original packet");
        fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", to_string(entry.packet.value()));
    }

    if (entry.entry)
    {
        if (entry.entry->successful)
        {
            fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "routed packet");
            fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", to_string(entry.entry->routing_result.routed_packet));
        }

        if (entry.duplicate_entry)
        {
            fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "duplicate packet");
            fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", to_string(entry.duplicate_entry->routing_result.routed_packet));
        }

        if (entry.entry->reject_reason != digipeater_reject_reason::none)
        {
            fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "reject reason");
            fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", to_string(entry.entry->reject_reason));
        }

        fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "id");
        fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", entry.entry->id);

        fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "hash");
        fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", entry.entry->hash);

        fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "packet time");
        fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", to_string(entry.entry->date_time));

        fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}: ", "elapsed_ms");
        fmt::print(fg(fmt::color::gray) | fmt::emphasis::italic, "{}\n", entry.entry->elapsed_ms);

        if (entry.entry->accepted && !entry.entry->pending && entry.diagnostics)
        {
            fmt::print(fg(fmt::color::rosy_brown) | fmt::emphasis::italic, "{:>18}:\n\n", "routing details");

            aprs::router::routing_diagnostic_display diag = aprs::router::format(entry.entry->routing_result);

            for (const auto& e : diag.entries)
            {
                fmt::print(fg(fmt::color::blue_violet), "{:>10}: ", "note");
                fmt::print("{}\n", e.message);
                fmt::print(fg(fmt::color::gray), "{:>10}{}\n", "", e.packet_string);
                fmt::print(fg(fmt::color::lime_green), "{:>10}{}\n", "", e.highlight_string);
            }
        }
    }

    fmt::print("\n");
}
