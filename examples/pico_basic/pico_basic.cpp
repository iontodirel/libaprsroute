#include <stdio.h>
#include "pico/stdlib.h"

#include "external/aprsroute.hpp"

int main()
{
    stdio_init_all();

    while (true) {
        printf("Hello, world!\n");

        // Routing example
        aprs::router::router_settings settings { "DIGI", {}, { "WIDE1" }, aprs::router::routing_option::none, true };
        aprs::router::routing_result result;        
        aprs::router::try_route_packet("N0CALL>APRS,WIDE1-1,WIDE2-2:data", settings, result);

        sleep_ms(1000);
    }
}
