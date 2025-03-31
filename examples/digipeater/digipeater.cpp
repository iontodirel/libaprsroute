#include "digipeater.h"
#include "log.h"

#include <fmt/format.h>

// **************************************************************** //
//                                                                  //
//                                                                  //
// to_string                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

std::string to_string(digipeater_reject_reason reason)
{
    switch (reason)
    {
        case digipeater_reject_reason::none:
            return "none";
        case digipeater_reject_reason::duplicate:
            return "duplicate";
        case digipeater_reject_reason::age:
            return "age";
        case digipeater_reject_reason::direct_only:
            return "direct_only";
        case digipeater_reject_reason::non_routed:
            return "non_routed";
        case digipeater_reject_reason::other:
            return "other";
    }
    return "unknown";
}

// **************************************************************** //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
// digipeater                                                       //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
// **************************************************************** //

// **************************************************************** //
//                                                                  //
//                                                                  //
// public implementation                                            //
//                                                                  //
//                                                                  //
// **************************************************************** //

void digipeater::initialize(digipeater_settings settings)
{
    settings_ = settings;

    router_settings.address = settings.address;
    router_settings.n_N_addresses = settings.n_N_addresses;
    router_settings.explicit_addresses = settings.explicit_addresses;
    router_settings.options = settings.options;
    router_settings.enable_diagnostics = true;
}

void digipeater::add_event_handler(digipeater_events& handler)
{
    event_handlers_.push_back(std::ref(handler));
}

void digipeater::route_packet(const aprs::router::packet& p)
{
    log(log_type::message, log_verbosity::verbose, "digipeater::route_packet", "Processing packet.", p);

    on_start_route(p);

    // Ensure the packet is meant for RF digipeating and is valid.
    // The packet should have no Q constructs in the packet path.
    // We should not see TCPIP or TCPXX addresses in the packet path.

    if (!validate_packet(p))
    {
        on_rejected_packet(p, false, 0);
        return;
    }

    // Route the packet and store it in the queue

    on_start_router(p);

    aprs::router::routing_result result;
    aprs::router::try_route_packet(p, router_settings, result);

    on_end_router(result);

    packet_entry entry = create_packet_entry(p, result);

    queue_packet(entry);

    log(log_type::message, log_verbosity::verbose, "digipeater::route_packet", "Packet added to queue", p, false, entry);

    on_end_route(p, packet_queue.size());

    update();
}

void digipeater::update()
{
    // Update entry elapsed times.
    // Remove old entries as configured by max_age_ms.

    update_elapsed_time();
    remove_old_entries();

    for (auto& entry : packet_queue) // reverse, process from the end
    {
        // If the packet has already been routed, skip it.
        // If the packet has been previously rejected or accepted, ignore it.
        // If the packet has been marked as removed, ignore it.

        if (!entry.pending || entry.rejected || entry.accepted || entry.removed)
        {
            continue;
        }

        // If the packet has failed to pre-route in the first place, ignore it.

        if (!entry.successful)
        {
            reject_packet(entry, "Packet failed to route", false, digipeater_reject_reason::non_routed, std::nullopt, "digipeater::update");
            continue;
        }

        // If the packet has been unconditionally accepted, skip it.

        if (handle_unconditional_accept_packet(entry))
        {
            continue;
        }

        // If the packet has a delay set, wait for the delay to expire first.

        if (entry.elapsed_ms <= settings_.hold_time_ms && settings_.hold_time_ms > 0)
        {
            continue;
        }

        // Ensure that this is not an old packet.
        // Even if a packet passes all the other checks,
        // we should not route packets that are older than max_accept_age_ms.

        if (entry.elapsed_ms >= settings_.max_accept_age_ms)
        {
            reject_packet(entry, "Packet is too old", false, digipeater_reject_reason::age, std::nullopt, "digipeater::update");
            continue;
        }

        // Ignore packets that have been routed by another station, if the direct_only option is set.
        // 
        // Ex: N0CALL>APRS,DIGI*,WIDE1-2:data
        //                 ~~~~~

        if (entry.has_used_addresses && settings_.direct_only)
        {
            reject_packet(entry, "Packet has already been routed by another station (direct only mode enabled)", false, digipeater_reject_reason::direct_only, std::nullopt, "digipeater::update");
            continue;
        }

        // From the time we added the packet to the queue,
        // we might have received the same packet routed by another station.
        // If so, we should consider it a duplicate and reject it.
        //
        // Example:
        //
        // A hold of 6 seconds is set for the digipeater.
        // Received packet 0: N0CALL>APRS,WIDE1-3:data
        // Put packet in the queue and wait for the delay to expire.
        // <1 second passes>
        // Received packet 1: N0CALL>APRS,OTHER*,WIDE1-2:data
        // <1 second passes>
        // Received packet 2: N0CALL>APRS,OTHER*,CALL*,WIDE1-1:data
        // <4 seconds passes>
        // Delay expired for packet 0.
        // Duplicates found for packet 0: packet 1 and packet 2.
        // Reject packet 0.

        if (handle_duplicate_packet(entry) || handle_ignore_packet(entry))
        {
            continue;
        }

        // Packet is ready to be routed and sent for TX.

        handle_accept_packet(entry);
    }
}

void digipeater::clear_all_packets()
{
    packet_queue.clear();
}

void digipeater::clear_routed_packets()
{
    for (auto it = packet_queue.begin(); it != packet_queue.end();)
    {
        if (it->accepted)
        {
            it = packet_queue.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::vector<aprs::router::routing_result> digipeater::routed_packets()
{
    return routed_packets(false);
}

std::vector<aprs::router::routing_result> digipeater::routed_packets(bool remove_routed_packets)
{
    // Returns all packets that have been routed and accepted.
    // If remove_routed_packets is true, the routed packets are marked as removed from the queue.
    // We don't remove them from the queue, because we need to keep track of the packets that have been routed,
    // for deduplication purposes.

    std::vector<aprs::router::routing_result> routed_packets;

    for (auto& entry : packet_queue)
    {
        if (entry.accepted)
        {
            if (entry.removed)
            {
                // If the packet has been marked as removed, skip it.
                continue;
            }

            routed_packets.push_back(entry.routing_result);

            if (remove_routed_packets)
            {
                entry.removed = true;
            }
        }
    }

    return routed_packets;
}

std::vector<aprs::router::routing_result> digipeater::non_routed_packets()
{
    // Returns all packets that have not been accepted yet. Whether rejected or pending.

    std::vector<aprs::router::routing_result> non_routed_packets;
    for (const auto& entry : packet_queue)
    {
        if (!entry.accepted && !entry.removed)
        {
            non_routed_packets.push_back(entry.routing_result);
        }
    }
    return non_routed_packets;
}

void digipeater::reset_simulated_time()
{
    simulated_time_ = false;
    log(log_type::message, log_verbosity::debug, "digipeater::reset_simulated_time", "Simulated time reset");
}

// **************************************************************** //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
// private implementation                                           //
//                                                                  //
//                                                                  //
//                                                                  //
//                                                                  //
// **************************************************************** //

// **************************************************************** //
//                                                                  //
//                                                                  //
// packet query and validation                                      //
//                                                                  //
//                                                                  //
// **************************************************************** //

std::vector<aprs::router::detail::address> digipeater::packet_addresses(const aprs::router::packet& p)
{
    std::vector<aprs::router::detail::address> addresses;
    for (const auto& address_string : p.path)
    {
        aprs::router::detail::address address;
        aprs::router::detail::try_parse_address(address_string, address);
        addresses.push_back(address);
    }
    return addresses;
}

bool digipeater::validate_packet(const aprs::router::packet& p)
{
    // Check if the packet is valid for digipeating.
    //
    // A packet is considered valid if:
    //
    //   - The packet does not have any of the following addresses in the "from" or "to" fields:
    //     - N0CALL
    //     - MYCALL
    //     - TCPIP
    //     - TCPXX
    //     - WIDE
    //     - RELAY
    //     - TRACE
    //     - NOCALL
    //     - A Q construct
    //   - The packet does not have an empty "path".
    //   - The packet does not have an empty "data" field.
    //   - For each of the packet path addresses:
    //     - The packet path address is not a Q construct.
    //     - The packet path address is not a TCPIP or TCPXX address.
    //     - The packet path address is not an igatecall address.
    //   - The packet does not have a "data" field larger than 256 bytes.

    using namespace aprs::router::detail;

    address from_address;

    if (!try_parse_address_with_ssid(p.from, from_address))
    {
        log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet from address is invalid", p);
        return false;
    }

    if (from_address.text == "N0CALL" || from_address.text == "MYCALL" ||
        from_address.text == "TCPIP" || from_address.text == "TCPXX" ||
        from_address.text == "WIDE" || from_address.text == "RELAY" ||
        from_address.text == "TRACE" || from_address.text == "NOCALL" ||
        from_address.q != q_construct::none)
    {
        log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet from address is invalid", p);
        return false;
    }

    address to_address;

    if (!try_parse_address_with_ssid(p.to, to_address))
    {
        log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet to address is invalid", p);
        return false;
    }

    if (to_address.text == "N0CALL" || to_address.text == "MYCALL" ||
        to_address.text == "TCPIP" || to_address.text == "TCPXX" ||
        to_address.text == "WIDE" || to_address.text == "RELAY" ||
        to_address.text == "TRACE" || to_address.text == "NOCALL" ||
        to_address.q != q_construct::none)
    {
        log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet to address is invalid", p);
        return false;
    }

    if (p.path.empty())
    {
        log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet path is empty", p);
        return false;
    }

    if (p.data.empty())
    {
        log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet data is empty", p);
        return false;
    }

    for (const auto& address_string : p.path)
    {
        address path_address;

        if (!try_parse_address(address_string, path_address))
        {
            log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet path address is invalid", p);
            return false;
        }

        if (path_address.kind == aprs::router::detail::address_kind::q)
        {
            log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet path address is a Q construct", p);
            return false;
        }

        if (path_address.kind == aprs::router::detail::address_kind::tcpip ||
            path_address.kind == aprs::router::detail::address_kind::tcpxx)
        {
            log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet path address is a TCPIP or TCPXX address", p);
            return false;
        }

        if (path_address.kind == aprs::router::detail::address_kind::igatecall)
        {
            log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet path address is an igatecall address", p);
            return false;
        }
    }

    if (p.data.size() > 256)
    {
        log(log_type::warning, log_verbosity::normal, "digipeater::validate_packet", "Packet data is too large", p);
        return false;
    }

    return true;
}

bool digipeater::has_used_addresses(const std::vector<aprs::router::detail::address>& addresses)
{
    // Look backwards in the list of addresses, to optimize for the most common cases
    // where the last address is the one that is marked as used.

    for (int i = static_cast<int>(addresses.size()) - 1; i >= 0; i--)
    {
        if (addresses[i].mark)
        {
            return true;
        }
    }

    return false;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// create entry, queue, book keeping                                //
//                                                                  //
//                                                                  //
// **************************************************************** //

packet_entry digipeater::create_packet_entry(const aprs::router::packet& p, const aprs::router::routing_result& result)
{
    packet_entry entry;

    std::vector<aprs::router::detail::address> addresses = packet_addresses(p);

    entry.routing_result = result;
    entry.successful = result.routed;
    entry.has_used_addresses = this->has_used_addresses(addresses);
    entry.date_time = get_local_time();
    entry.hash = aprs::router::hash(p);
    entry.timestamp = std::chrono::high_resolution_clock::now();
    entry.id = count_;

    count_++;

    return entry;
}

packet_entry& digipeater::queue_packet(packet_entry& entry)
{
    packet_queue.push_back(entry); // push_front
    return packet_queue.back();
}

void digipeater::remove_old_entries()
{
    for (auto it = packet_queue.begin(); it != packet_queue.end(); )
    {
        if (it->elapsed_ms >= settings_.max_keep_age_ms)
        {
            log(log_type::message, log_verbosity::verbose, "digipeater::remove_old_entries", fmt::format("Removing old entry (max_age_ms: {})", settings_.max_keep_age_ms), it->routing_result.original_packet, false, *it);
            it = packet_queue.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// try_find_duplicate                                               //
//                                                                  //
//                                                                  //
// **************************************************************** //

bool digipeater::try_find_duplicate(const packet_entry& entry, packet_entry& result)
{
    for (int i = static_cast<int>(packet_queue.size()) - 1; i >= 0; i--)
    {
        const packet_entry& e = packet_queue[i];

        // Find a packet that we might have received recently from another station.
        // Having failed routing is ok, if the reason is completed routing.

        if (e.hash == entry.hash && e.id != entry.id && !e.rejected && e.elapsed_ms < settings_.dedupe_window_ms)
        {
            result = packet_queue[i];
            return true;
        }
    }

    return false;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// accept/reject/ignore                                             //
//                                                                  //
//                                                                  //
// **************************************************************** //

void digipeater::reject_packet(packet_entry& entry, const std::string& message, bool is_duplicate, digipeater_reject_reason reason, std::optional<packet_entry> duplicate_packet, std::string function_name)
{
    entry.rejected = true;
    entry.pending = false;
    entry.reject_reason = reason;

    log(log_type::warning, log_verbosity::verbose, function_name, message, entry.routing_result.original_packet, false, entry, duplicate_packet);

    on_rejected_packet(entry.routing_result.original_packet, is_duplicate, entry.elapsed_ms);
}

void digipeater::accept_packet(packet_entry& entry, std::string function_name)
{
    entry.accepted = true;
    entry.pending = false;

    log(log_type::message, log_verbosity::normal, function_name, "Packet routing completed", entry.routing_result.original_packet, true, entry);

    on_accepted_packet(entry.routing_result.original_packet, entry.elapsed_ms);
}

void digipeater::ignore_packet(packet_entry& entry, std::string function_name)
{
    log(log_type::message, log_verbosity::verbose, function_name, "Packet was filtered out", entry.routing_result.original_packet, false, entry);
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// high level handling                                              //
//                                                                  //
//                                                                  //
// **************************************************************** //

bool digipeater::handle_duplicate_packet(packet_entry& entry)
{
    packet_entry duplicate_entry;
    if (try_find_duplicate(entry, duplicate_entry))
    {
        // Packet is a duplicate. Normally, such packets are rejected.
        // Run the filter_packet function to check if a handler wants to accept the duplicate packet.
        // This allows for custom routing logic, or allowing some dupes, while rejecting others.

        bool accept_duplicate_entry = false;
        on_accept_duplicate_packet(entry.routing_result.original_packet, accept_duplicate_entry);
        if (!accept_duplicate_entry)
        {
            reject_packet(entry, "Packet is a duplicate", true, digipeater_reject_reason::duplicate, duplicate_entry, "digipeater::handle_duplicate_packet");
            return true;
        }
    }
    return false;
}

bool digipeater::handle_ignore_packet(packet_entry& entry)
{
    // Packet passed all the checks and was accepted.
    // Run the filter_packet function to check if a handler wants to ignore it.
    // This allows for custom routing logic, like rate limiting.

    bool ignore_entry = false;
    on_ignore_packet(entry.routing_result.original_packet, ignore_entry);
    if (ignore_entry)
    {
        ignore_packet(entry, "digipeater::handle_ignore_packet");
        return true;
    }
    return false;
}

bool digipeater::handle_unconditional_accept_packet(packet_entry& entry)
{
    bool force_accept_entry = false;
    on_unconditionally_accept_packet(entry.routing_result.original_packet, force_accept_entry);
    if (force_accept_entry)
    {
        log(log_type::message, log_verbosity::debug, "digipeater::handle_unconditional_accept_packet", "Packet was unconditionally accepted", entry.routing_result.original_packet, false, entry);
        handle_accept_packet(entry);
        return true;
    }
    return false;
}

void digipeater::handle_accept_packet(packet_entry& entry)
{
    // Handle packet transcoding.
    // This allows transforming the packet into a different format.

    handle_transcode_packet(entry);

    // Packet is ready to be routed and sent for TX.

    accept_packet(entry, "digipeater::handle_accept_packet");
}

bool digipeater::handle_transcode_packet(packet_entry& entry)
{
    bool transcode = false;

    aprs::router::packet transcoded_packet;
    on_transcode_packet(entry.routing_result.original_packet, transcode, transcoded_packet);

    if (transcode)
    {
        entry.routing_result.routed_packet = transcoded_packet;

        // Create a new packet entry for the transcoded packet.
        // We do this to block a packet like the transcoded packet from being routed again, due to the duplicate check.

        packet_entry transcoded_entry = create_packet_entry(transcoded_packet, entry.routing_result);
        transcoded_entry.routing_result.original_packet = entry.routing_result.routed_packet;
        transcoded_entry.accepted = true;
        transcoded_entry.pending = false;

        packet_queue.push_back(transcoded_entry);

        return true;
    }

    return false;
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// on events                                                        //
//                                                                  //
//                                                                  //
// **************************************************************** //

void digipeater::on_ignore_packet(const aprs::router::packet& p, bool& ignore)
{
    bool original_ignore_value = ignore;
    for (auto& handler : event_handlers_)
    {
        handler.get().ignore_packet(p, ignore);
        if (original_ignore_value != ignore)
        {
            break;
        }
    }
}

void digipeater::on_unconditionally_accept_packet(const aprs::router::packet& p, bool& accept)
{
    bool original_accept_value = accept;
    for (auto& handler : event_handlers_)
    {
        handler.get().unconditionally_accept_packet(p, accept);
        if (original_accept_value != accept)
        {
            break;
        }
    }
}

void digipeater::on_accept_duplicate_packet(const aprs::router::packet& p, bool& accept)
{
    bool original_accept_value = accept;
    for (auto& handler : event_handlers_)
    {
        handler.get().accept_duplicate_packet(p, accept);
        if (original_accept_value != accept)
        {
            break;
        }
    }
}

void digipeater::on_start_router(const aprs::router::packet& p)
{
    // Call all event handlers to notify them of the start of the router process.

    for (auto& handler : event_handlers_)
    {
        handler.get().start_router(p);
    }
}

void digipeater::on_end_router(const aprs::router::routing_result& result)
{
    // Call all event handlers to notify them of the end of the router process.

    for (auto& handler : event_handlers_)
    {
        handler.get().end_router(result);
    }
}

void digipeater::on_start_route(const aprs::router::packet& p)
{
    // Call all event handlers to notify them of the start of the routing process.

    for (auto& handler : event_handlers_)
    {
        handler.get().start_route(p);
    }
}

void digipeater::on_end_route(const aprs::router::packet& p, size_t total_count)
{
    // Call all event handlers to notify them of the end of the routing process.

    for (auto& handler : event_handlers_)
    {
        handler.get().end_route(p, total_count);
    }
}

void digipeater::on_accepted_packet(const aprs::router::packet& p, unsigned long long elapsed_ms)
{
    // Call all event handlers to notify them of the accepted packet.

    for (auto& handler : event_handlers_)
    {
        handler.get().accepted_packet(p, elapsed_ms);
    }
}

void digipeater::on_rejected_packet(const aprs::router::packet& p, bool duplicate, unsigned long long elapsed_ms)
{
    // Call all event handlers to notify them of the rejected packet.

    for (auto& handler : event_handlers_)
    {
        handler.get().rejected_packet(p, duplicate, elapsed_ms);
    }
}

void digipeater::on_transcode_packet(const aprs::router::packet& input, bool& transcode, aprs::router::packet& output)
{
    bool original_transcode_value = transcode;
    for (auto& handler : event_handlers_)
    {
        handler.get().transcode_packet(input, transcode, output);
        if (original_transcode_value != transcode)
        {
            break;
        }
    }
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// simulated time                                                   //
//                                                                  //
//                                                                  //
// **************************************************************** //

void digipeater::simulate_elapsed_time(unsigned long long offset_ms)
{
    unsigned long long increment_ms = 100;
    unsigned long long steps = offset_ms / increment_ms;

    simulated_time_ = true;

    for (int i = 0; i < steps; ++i)
    {
        for (auto& entry : packet_queue)
        {
            entry.elapsed_ms += increment_ms;
        }

        update();
    }

    auto remaining_ms = offset_ms % increment_ms;
    if (remaining_ms > 0)
    {
        for (auto& entry : packet_queue)
        {
            entry.elapsed_ms += remaining_ms;
        }

        update();
    }

    log(log_type::message, log_verbosity::debug, "digipeater::simulate_elapsed_time", fmt::format("Simulated time advanced by {} ms.", offset_ms));
}

void digipeater::update_elapsed_time()
{
    if (simulated_time_)
    {
        // If we are simulating time, we don't need to update the elapsed time.
        // The elapsed time is already updated in the simulate_elapsed_time function.
        return;
    }

    auto now = std::chrono::high_resolution_clock::now();

    for (auto& entry : packet_queue)
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - entry.timestamp);
        entry.elapsed_ms = elapsed.count();
    }
}

// **************************************************************** //
//                                                                  //
//                                                                  //
// log                                                              //
//                                                                  //
//                                                                  //
// **************************************************************** //

void digipeater::log(const log_entry& entry)
{
    for (auto& logger : loggers_)
    {
        logger.get().log(entry);
    }
}

void digipeater::log(log_type type, log_verbosity verbosity, const std::string& function_name, const std::string& message)
{
    struct log_entry log_entry;
    log_entry.verbosity = verbosity;
    log_entry.type = type;
    log_entry.function_name = function_name;
    log_entry.date_time = get_local_time();
    log_entry.message = message;
    log(log_entry);
}

void digipeater::log(log_type type, log_verbosity verbosity, const std::string& function_name, const std::string& message, const aprs::router::packet& packet, bool diagnostics, std::optional<packet_entry> entry, std::optional<packet_entry> duplicate_entry)
{
    struct log_entry log_entry;
    log_entry.type = type;
    log_entry.verbosity = verbosity;
    log_entry.function_name = function_name;
    log_entry.date_time = get_local_time();
    log_entry.message = message;
    log_entry.packet = packet;
    log_entry.diagnostics = diagnostics;
    if (entry)
    {
        log_entry.entry = std::make_unique<packet_entry>(entry.value());
    }
    if (duplicate_entry)
    {
        log_entry.duplicate_entry = std::make_unique<packet_entry>(*duplicate_entry);
    }
    log(log_entry);
}
