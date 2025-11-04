#pragma once
#include "pump_base_simple.h"

namespace Services {
class Pump_pont : public PumpBaseSimple {
public:
    Pump_pont(uint8_t pin, unsigned long debounceMs = 200, unsigned long timeoutMs = 0)
        : PumpBaseSimple(pin, "pont_pump", debounceMs, timeoutMs) {}
};
} // namespace Services
