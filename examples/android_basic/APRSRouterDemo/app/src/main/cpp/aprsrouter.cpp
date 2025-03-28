// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("aprsrouter");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("aprsrouter")
//      }
//    }

#include <jni.h>

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

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_aprsrouter_MainActivity_routePacket(
        JNIEnv* env,
        jobject /* this */,
        jstring callsign,
        jstring path,
        jstring original_packet) {

    // Convert jstrings to std::strings

    const char* callsign_cstr = env->GetStringUTFChars(callsign, nullptr);
    const char* path_cstr = env->GetStringUTFChars(path, nullptr);
    const char* original_packet_cstr = env->GetStringUTFChars(original_packet, nullptr);

    std::string callsign_str(callsign_cstr);
    std::string path_str(path_cstr);
    std::string original_packet_str(original_packet_cstr);

    // IMPORTANT: Release memory
    env->ReleaseStringUTFChars(callsign, callsign_cstr);
    env->ReleaseStringUTFChars(path, path_cstr);
    env->ReleaseStringUTFChars(original_packet, original_packet_cstr);

    packet p = original_packet_str;

    router_settings settings;
    settings.explicit_addresses = {};
    settings.n_N_addresses = {};
    settings.address = callsign_str;
    settings.enable_diagnostics = true;

    ::init_router_addresses(p, split_comma_separated_values(path_str), settings);

    routing_result result;
    try_route_packet(p, settings, result);

    std::string routed_packet_str = to_string(result.routed_packet);

    return env->NewStringUTF(routed_packet_str.c_str());
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