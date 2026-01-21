#pragma once
/// @cond
#include <Arduino.h>

//#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "Pumps_220V.h"
namespace Services
{

    class Pump_heat : public Pumps_220V
    {
    public:
        Pump_heat(const String &taskName)
            : Pumps_220V(taskName, 0, 0, "pool/pump/heat", 0, 0)
        {
        }
    };
} // End namespace Services
