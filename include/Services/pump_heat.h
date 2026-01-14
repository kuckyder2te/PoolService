#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "dosingPumps.h"
namespace Services
{

    class Pump_heat : public DosingPumps
    {
    public:
        Pump_heat(uint8_t pin,
                  uint8_t monpin) // not exist
            : DosingPumps(pin, monpin, "pool/pump/heat", 0, 0)
        {
        }
    };
} // End namespace Services