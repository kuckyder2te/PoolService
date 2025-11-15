#pragma once
#include "dosingPumps.h"

namespace Services
{
    class Pump_naoh : public DosingPumps
    {
    public:
        Pump_naoh(uint8_t pin,
            uint8_t monpin)
        : DosingPumps(pin, monpin, "naoh_pump", 200) {}
    };
}
