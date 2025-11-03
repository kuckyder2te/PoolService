#pragma once
#include "../include/Services/pump_base.h"

namespace Services {
class Pump_algizid : public PumpBase {
public:
    Pump_algizid(uint8_t pin, uint8_t monitorPin = 255, bool monitorActiveLow = true)
      : PumpBase(pin, "algizid_pump", 200, 0, monitorPin, monitorActiveLow, 500) {}
};
} // namespace Services
