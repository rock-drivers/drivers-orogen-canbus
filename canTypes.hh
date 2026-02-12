#ifndef CANBUS_TYPES_HH
#define CANBUS_TYPES_HH

#include <base/Time.hpp>
#include <cstdint>

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

    struct CanOutputPort
    {
        std::string ports_name;
        std::uint32_t mask;
        std::uint32_t id;
        CanOutputPort()
            : mask(0), id(0) {}
    };
}
#endif

