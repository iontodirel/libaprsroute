#include "digipeater.h"

int main()
{
    digipeater digi;

    digipeater_settings settings;
    settings.address = "DIGI";
    settings.n_N_addresses = { "WIDE1" };
    settings.explicit_addresses = {};
    settings.options = aprs::router::routing_option::none;
    settings.debug = true;
    settings.hold_time_ms = 6000; // 6s
    settings.direct_only = false;
    settings.dedupe_window_ms = 30000; // 30s
    settings.max_keep_age_ms = 60000; // 1min
    settings.max_accept_age_ms = 30000; // 30s

    digi.initialize(settings);

    basic_stdout_logger logger;
    logger.verbosity = log_verbosity::debug;
    digi.add_logger(logger);

    for (int i = 0; i < 100; ++i)
    {
        digi.route_packet("CALL>APRS,WIDE1-3:data");
        size_t delay = generate_random_number(0, 40);
        digi.simulate_elapsed_time(std::chrono::seconds(delay));
    }

    return 0;
}