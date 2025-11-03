#pragma once
#include "../include/Services/pumpBase.h"

namespace Services {
class Pump_hcl : public PumpBase {
public:
    // Optional: monitorPin default 255 = none
    Pump_hcl(uint8_t pin, uint8_t monitorPin, bool monitorActiveLow = true)
      : PumpBase(pin, "hcl_pump", 200, 0, monitorPin, monitorActiveLow, 500) {}
};
} // namespace Services
