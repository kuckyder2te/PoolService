#pragma once
#include "dosingPumps.h"

namespace Services
{
    class Pump_algizid : public DosingPumps
    {
    public:
        Pump_algizid(uint8_t pin,
                     uint8_t monpin)
            : DosingPumps(pin, monpin, "algizid_pump", 200) {}
    };
}
