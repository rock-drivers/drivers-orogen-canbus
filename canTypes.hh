#ifndef CANBUS_TYPES_HH
#define CANBUS_TYPES_HH

#include <base/Time.hpp>

namespace canbus
{
    struct Statistics
    {
        base::Time time;
        uint64_t msg_tx; //! the number of messages sent (total)
        uint64_t msg_rx; //! the number of messages received (total)
        uint64_t tx; //! the amount of bytes sent (total)
        uint64_t rx; //! the amount of bytes received (total)
        uint64_t error_count; 

        Statistics()
            : msg_tx(0), msg_rx(0), tx(0), rx(0), error_count(0) {}
    };
}
#endif

