#pragma once
#include "dosingPumps.h"

#ifndef HCL_PUMP
#error "HCL_PUMP not defined"
#endif
#ifndef HCL_MON
#define HCL_MON 255
#endif
#ifndef TIMEOUT_HCL_PUMP
#define TIMEOUT_HCL_PUMP 0
#endif

namespace Services {

class dosierHcl : public dosingPumps {
public:
    explicit dosierHcl(const String& taskName)
        : dosingPumps(taskName, HCL_PUMP, HCL_MON, "pool/pump/hcl", TIMEOUT_HCL_PUMP) {}
};

} // namespace Services