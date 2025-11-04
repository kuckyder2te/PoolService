#pragma once
#include "../include/Services/dosing_pump_base.h"

namespace Services {
class Pump_algizid : public PumpBase {
public:
    Pump_algizid(uint8_t pin, uint8_t monitorPin, bool monitorActiveLow = true)
      : PumpBase(pin, "algizid_pump", 200, 0, monitorPin, monitorActiveLow, 500) {}
};
} // namespace Services
