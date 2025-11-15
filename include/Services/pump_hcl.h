#pragma once
#include "dosingPumps.h"

namespace Services {
class Pump_hcl : public DosingPumps {
public:
    Pump_hcl(uint8_t pin)
        : DosingPumps(pin, "hcl_pump", 200) {}
};
}
