#pragma once

#include "common.h"
#include "log.h"

#include <string>
#include <chrono>
#include <map>
#include <vector>

#include "external/aprsroute.hpp"

#include <fmt/format.h>

// **************************************************************** //
//                                                                  //
//                                                                  //
// digipeater_settings                                              //
//                                                                  //
//                                                                  //
// **************************************************************** //

struct digipeater_settings
{
    std::string address;                          // router configuration
    std::vector<std::string> explicit_addresses;  // router configuration
    std::vector<std::string> n_N_addresses;       // router configuration
    aprs::router::routing_option options = aprs::router::routing_option::none; // router configuration
    bool debug = true;                            // router configuration
    long long hold_time_ms = 0;                   // how long to wait before routing the packet, this can be used for viscuous digipeating
    long long dedupe_window_ms = 30000; // 30s    // packets with the same hash are considered duplicates within this window
    long long max_keep_age_ms = 60000; // 1min    // packets older than this are removed from the queue; packets might be kept in the queue for longer for diagnostics purposes
    long long max_accept_age_ms = 10000; // 10s   // packets older than this are rejected
    bool direct_only = false;                     // if true, packets that have been routed by another station are rejected
};

// **************************************************************** //
//                                                                  //
//                                                                  //
// digipeater_events                                                //
//                                                                  //
//                                                                  //
// **************************************************************** //

struct digipeater_events
{
    // Called at the beginning of a packet's routing process.
    // Always called.
    virtual void start_route(const aprs::router::packet& p) = 0;

    // Called at the end of a packet's routing process, after end_router.
    // Always called.
    virtual void end_route(const aprs::router::packet& p, size_t total_count) = 0;

    // Called before the router starts processing a packet.
    // Always called.
    virtual void start_router(const aprs::router::packet& p) = 0;

    // Called after the router has finished processing a packet.
    // Always called.
    virtual void end_router(const aprs::router::routing_result&) = 0;
    
    // Called after a packet is processed by the router. It can be used to bypass all hold and duplicate checks.
    // If accept is set to true, the packet is accepted and routed.
    virtual void unconditionally_accept_packet(const aprs::router::packet& p, bool& accept) = 0;

    // Called when a packet is a duplicate.
    // If accept is set to true, the packet is accepted and routed.
    virtual void accept_duplicate_packet(const aprs::router::packet& p, bool& accept) = 0;

    // Called after a packet is accepted, to control whether a client wants to ignore it.
    // Can be useful for implementing custom routing logic or rate limiting.
    // If ignore is set to true, the packet is kept in pending state and will not be routed.
    // This can be also used to additionally delay the routing of a packet.
    virtual void ignore_packet(const aprs::router::packet& p, bool& ignore) = 0;

    // Called when a packet is accepted.
    virtual void accepted_packet(const aprs::router::packet&, unsigned long long elapsed_ms) = 0;
    
    // Called when a packet is rejected.
    virtual void rejected_packet(const aprs::router::packet&, bool duplicate, unsigned long long elapsed_ms) = 0;

    // Called after a packed is accepted.
    // Can be used to transcode a packet to a different format.
    // For example, a position packet can be transcoded to mic-e format.
    virtual void transcode_packet(const aprs::router::packet& input, bool& transcode, aprs::router::packet& output) = 0;
};

// **************************************************************** //
//                                                                  //
//                                                                  //
// digipeater_reject_reason                                         //
//                                                                  //
//                                                                  //
// **************************************************************** //

enum class digipeater_reject_reason
{
    none,
    duplicate,
    age,
    direct_only,
    non_routed,
    other
};

std::string to_string(digipeater_reject_reason);

// **************************************************************** //
//                                                                  //
//                                                                  //
// packet_entry                                                     //
//                                                                  //
//                                                                  //
// **************************************************************** //

struct packet_entry
{
    unsigned long long id;
    size_t hash;
    aprs::router::routing_result routing_result;
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    struct date_time date_time;
    unsigned long long elapsed_ms = 0; // elapsed time in milliseconds
    bool has_used_addresses = false; // Packet has at least one "used" address
    bool successful = false; // Whether the packet has successfully been "routed"
    bool pending = true; // Whether the packet is still pending for routing
    bool rejected = false; // Packet has been rejected
    bool accepted = false; // Packet has been accepted
    bool removed = false; // Packet has been marked as removed
    digipeater_reject_reason reject_reason = digipeater_reject_reason::none;
};

// **************************************************************** //
//                                                                  //
//                                                                  //
// digipeater                                                       //
//                                                                  //
//                                                                  //
// **************************************************************** //

class digipeater
{
public:
    void initialize(digipeater_settings settings);

    void add_event_handler(digipeater_events& handler);

    template<class T>
    void add_logger(T& logger);

    void route_packet(const aprs::router::packet& p);

    void update();

    void clear_all_packets();
    void clear_routed_packets();

    std::vector<aprs::router::routing_result> routed_packets();
    std::vector<aprs::router::routing_result> routed_packets(bool remove_routed_packets);

    std::vector<aprs::router::routing_result> non_routed_packets();

    template<typename Rep, typename Period>
    void simulate_elapsed_time(std::chrono::duration<Rep, Period> offset);

    void reset_simulated_time();

private:
    std::vector<aprs::router::detail::address> packet_addresses(const aprs::router::packet& p);
    bool validate_packet(const aprs::router::packet& p);
    bool has_used_addresses(const std::vector<aprs::router::detail::address>& addresses);
    packet_entry create_packet_entry(const aprs::router::packet& p, const aprs::router::routing_result& result);
    packet_entry& queue_packet(packet_entry& entry);
    void remove_old_entries();
    bool try_find_duplicate(const packet_entry& entry, struct packet_entry& result);
    void reject_packet(packet_entry& entry, const std::string& message, bool is_duplicate, digipeater_reject_reason reason, std::optional<packet_entry> duplicate_packet, std::string function_name);
    void accept_packet(packet_entry& entry, std::string function_name);
    void ignore_packet(packet_entry& entry, std::string function_name);

    bool handle_duplicate_packet(packet_entry& entry);
    bool handle_ignore_packet(packet_entry& entry);
    bool handle_unconditional_accept_packet(packet_entry& entry);
    void handle_accept_packet(packet_entry& entry);
    bool handle_transcode_packet(packet_entry& entry);

    void on_ignore_packet(const aprs::router::packet& p, bool& ignore);
    void on_unconditionally_accept_packet(const aprs::router::packet& p, bool& accept);
    void on_accept_duplicate_packet(const aprs::router::packet& p, bool& accept);
    void on_start_router(const aprs::router::packet& p);
    void on_end_router(const aprs::router::routing_result&);
    void on_start_route(const aprs::router::packet& p);
    void on_end_route(const aprs::router::packet& p, size_t total_count);    
    void on_accepted_packet(const aprs::router::packet&, unsigned long long elapsed_ms);
    void on_rejected_packet(const aprs::router::packet&, bool is_duplicate, unsigned long long elapsed_ms);
    void on_transcode_packet(const aprs::router::packet& input, bool& transcode, aprs::router::packet& output);

    void simulate_elapsed_time(unsigned long long offset_ms);
    void update_elapsed_time();

    void log(const log_entry& entry);
    void log(log_type type, log_verbosity verbosity, const std::string& function_name, const std::string& message);
    void log(log_type type, log_verbosity verbosity, const std::string& function_name, const std::string& message, const aprs::router::packet& packet, bool diagnostics = false, std::optional<packet_entry> entry = std::nullopt, std::optional<packet_entry> duplicate_entry = std::nullopt);

    unsigned long long count_ = 0;
    std::vector<packet_entry> packet_queue;
    aprs::router::router_settings router_settings;
    digipeater_settings settings_;
    std::vector<std::reference_wrapper<logger_base>> loggers_;
    bool simulated_time_ = false;
    std::vector<std::reference_wrapper<digipeater_events>> event_handlers_;
};

// **************************************************************** //
//                                                                  //
//                                                                  //
// digipeater implementation                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

template<class T>
inline void digipeater::add_logger(T& logger)
{
    loggers_.push_back(std::ref(logger));
}

template <typename Rep, typename Period>
inline void digipeater::simulate_elapsed_time(std::chrono::duration<Rep, Period> offset)
{
    auto offset_ms = std::chrono::duration_cast<std::chrono::milliseconds>(offset);
    simulate_elapsed_time(offset_ms.count());
}