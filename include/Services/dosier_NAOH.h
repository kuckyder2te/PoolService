#pragma once
#include "dosingPumps.h"

#ifndef NAOH_PUMP
#error "NAOH_PUMP not defined"
#endif
#ifndef NAOH_MON
#define NAOH_MON 255
#endif
#ifndef TIMEOUT_NAOH_PUMP
#define TIMEOUT_NAOH_PUMP 0
#endif

namespace Services {

class dosierNaoh : public dosingPumps {
public:
    explicit dosierNaoh(const String& taskName)
        : dosingPumps(taskName, NAOH_PUMP, NAOH_MON, "pool/pump/naoh", TIMEOUT_NAOH_PUMP) {}
};

} // namespace Services