#pragma once
#include "dosingPumps.h"

namespace Services {
class Pump_algizid : public DosingPumps {
public:
    Pump_algizid(uint8_t pin)
        : DosingPumps(pin, "algizid_pump", 200) {}
};
}
