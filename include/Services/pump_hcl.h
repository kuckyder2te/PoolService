#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "dosingPumps.h"

namespace Services {
class Pump_hcl : public DosingPumps {
public:
    Pump_hcl(uint8_t pin)
        : DosingPumps(pin, "hcl_pump") {}
};
}
