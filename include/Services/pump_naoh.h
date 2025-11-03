#pragma once
#include "../include/Services/pump_naoh.h"

namespace Services {
class Pump_naoh : public PumpBase {
public:
    Pump_naoh(uint8_t pin, uint8_t monitorPin = 255, bool monitorActiveLow = true)
      : PumpBase(pin, "naoh_pump", 200, 0, monitorPin, monitorActiveLow, 500) {}
};
} // namespace Services
