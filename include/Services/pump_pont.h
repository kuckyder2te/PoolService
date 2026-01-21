#pragma once
/// @cond
#include <Arduino.h>
//#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "Pumps_220V.h"

namespace Services
{
    class Pump_pont : public Pumps_220V
    {
        public:
        Pump_pont(const String &taskName)
            : Pumps_220V(taskName, 0, 0, "pont/pump", 0, 0) {}
    };

} // end of namespace Services
