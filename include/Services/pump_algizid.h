#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "dosingPumps.h"

namespace Services
{
    class Pump_algizid : public DosingPumps
    {
    public:
        Pump_algizid(uint8_t pin,
                     uint8_t monpin)
            : DosingPumps(pin, monpin, "algizid_pump", 200, 0) {}
    };
} // end of namespace Services
