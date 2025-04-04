#include <string>
#include <vector>
#include <sstream>

#include "external/aprsroute.hpp"

using namespace aprs::router;
using namespace aprs::router::detail;

bool try_parse_addresses(const std::vector<std::string>& addresses, std::vector<address>& result);
std::vector<address> get_router_n_N_addresses(const std::vector<address>& router_addresses);
std::vector<address> get_router_explicit_addresses(const std::vector<address>& router_addresses);
std::vector<std::string> to_vector_of_string(const std::vector<address>& addresses);
void init_router_addresses(const packet& p, const std::vector<std::string>& path, router_settings& settings);
std::vector<std::string> split_comma_separated_values(const std::string& str);

#ifdef _WIN32
extern "C" __declspec(dllexport) bool try_route_packet(const char* packet_string, const char* router_callsign_string, const char* router_path_string, char* router_packet_string, size_t* buffer_size)
#else
extern "C" __attribute__((visibility("default"))) bool try_route_packet(const char* packet_string, const char* router_callsign_string, const char* router_path_string, char* router_packet_string, size_t* buffer_size)
#endif
{    
    packet packet = packet_string;

    router_settings settings;
    settings.explicit_addresses = {};
    settings.n_N_addresses = {};
    settings.address = router_callsign_string;
    settings.enable_diagnostics = true;

    ::init_router_addresses(packet, split_comma_separated_values(router_path_string), settings);

    routing_result result;
    try_route_packet(packet, settings, result);

    std::string routed_packet_string = to_string(result.routed_packet);

    if (routed_packet_string.size() >= *buffer_size)
    {
        return false;
    }

    *buffer_size = routed_packet_string.size();

    std::copy(routed_packet_string.begin(), routed_packet_string.end(), router_packet_string);

    return result.routed;
}

bool try_parse_addresses(const std::vector<std::string>& addresses, std::vector<address>& result)
{
    result.clear();
    size_t index = 0;
    for (const auto& a : addresses)
    {
        address s;
        try_parse_address(a, s);
        s.index = index;
        index++;
        result.push_back(s);
    }
    return true;
}

std::vector<address> get_router_n_N_addresses(const std::vector<address>& router_addresses)
{
    std::vector<address> result;
    for (const auto& p : router_addresses)
    {
        if (p.n > 0)
        {
            result.push_back(p);
        }
    }
    return result;
}

std::vector<address> get_router_explicit_addresses(const std::vector<address>& router_addresses)
{
    std::vector<address> result;
    for (const auto& p : router_addresses)
    {
        if (p.n == 0)
        {
            result.push_back(p);
        }
    }
    return result;
}

std::vector<std::string> to_vector_of_string(const std::vector<address>& addresses)
{
    std::vector<std::string> result;
    for (const auto& p : addresses)
    {
        result.push_back(to_string(p));
    }
    return result;
}

void init_router_addresses(const packet& p, const std::vector<std::string>& path, router_settings& settings)
{
    if (path.empty())
    {
        return;
    }

    std::vector<address> router_addresses;
    try_parse_addresses(path, router_addresses);

    route_state state;
    state.router_n_N_addresses = get_router_n_N_addresses(router_addresses);
    state.router_explicit_addresses = get_router_explicit_addresses(router_addresses);
    state.packet_from_address = p.from;
    state.packet_to_address = p.to;
    state.packet_path = p.path;
    state.settings = settings;
    init_addresses(state);

    settings.explicit_addresses = to_vector_of_string(state.router_explicit_addresses);
    settings.n_N_addresses = to_vector_of_string(state.router_n_N_addresses);
}

std::vector<std::string> split_comma_separated_values(const std::string& str)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        result.push_back(item);
    }

    return result;
}