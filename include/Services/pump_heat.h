#pragma once
#include "pump_base_simple.h"

namespace Services {
class Pump_heat : public PumpBaseSimple {
public:
    Pump_heat(uint8_t pin, unsigned long debounceMs = 200, unsigned long timeoutMs = 0)
        : PumpBaseSimple(pin, "heat_pump", debounceMs, timeoutMs) {}
};
} // namespace Services
