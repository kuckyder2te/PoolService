#pragma once
#include "../include/Services/dosing_pump_base.h"

namespace Services {
class Pump_naoh : public PumpBase {
public:
    Pump_naoh(uint8_t pin, uint8_t monitorPin, bool monitorActiveLow = true)
      : PumpBase(pin, "naoh_pump", 200, 0, monitorPin, monitorActiveLow, 500) {}
};
} // namespace Services
