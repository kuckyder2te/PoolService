#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "dosingPumps.h"

namespace Services
{
    class Pump_pont : public DosingPumps
    {
        public:
        Pump_pont(uint8_t pin,
                  uint8_t monpin) // not exist
            : DosingPumps(pin, monpin, "pont/pump", 0, 0) {}
    };

} // end of namespace Services