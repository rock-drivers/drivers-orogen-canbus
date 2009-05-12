#ifndef CANBUS_TYPES_HH
#define CANBUS_TYPES_HH

namespace can
{
    struct Statistics
    {
        float    load; //! the bus load on the last 10 seconds
        int tx; //! the amount of bytes sent (total)
        int rx; //! the amount of bytes received (total)
    };
}
#endif

