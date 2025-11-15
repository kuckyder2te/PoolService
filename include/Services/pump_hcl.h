#pragma once
#include "dosingPumps.h"

namespace Services {
class Pump_hcl : public DosingPumps {
public:
    Pump_hcl(uint8_t pin,
     uint8_t monpin)
        : DosingPumps(pin, monpin, "hcl_pump", 200) {}
};
}
