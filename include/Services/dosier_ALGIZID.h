#pragma once
#include "dosingPumps.h"

#ifndef ALGIZID_PUMP
#error "ALGIZID_PUMP not defined"
#endif
#ifndef ALGIZID_MON
#define ALGIZID_MON 255
#endif
#ifndef TIMEOUT_ALGIZID_PUMP
#define TIMEOUT_ALGIZID_PUMP 0
#endif

namespace Services {

class dosierAlgzid : public dosingPumps {
public:
    explicit dosierAlgzid(const String& taskName)
        : dosingPumps(taskName, ALGIZID_PUMP, ALGIZID_MON, "pool/pump/algizid", TIMEOUT_ALGIZID_PUMP) {}
};

} // namespace Services