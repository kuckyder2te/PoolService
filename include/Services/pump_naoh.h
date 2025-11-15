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
    class Pump_naoh : public DosingPumps
    {
    public:
        Pump_naoh(uint8_t pin,
                  uint8_t monpin)
            : DosingPumps(pin, monpin, "naoh_pump", 200) {}
    };
}
