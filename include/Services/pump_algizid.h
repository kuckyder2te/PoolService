#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "dosingPumps.h"

namespace Services {
class Pump_algizid : public DosingPumps {
public:
    Pump_algizid(uint8_t pin)
        : DosingPumps(pin, "algizid_pump") {}
};
}
