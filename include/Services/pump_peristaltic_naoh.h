#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "peristalticPumps.h"

namespace Services
{
    /// @brief Klasse für die Peristaltik-Pumpe zur Dosierung von Natronlauge (NaOH)
    /// @details Diese Klasse erbt von der PeristalticPumps-Basisklasse und ist speziell 
    ///          für die Steuerung einer Natronlauge-Pumpe konfiguriert.
    class PumpPeristalticNAOH : public PeristalticPumps
    {
    public:
        /// @brief Konstruktor für die NaOH-Pumpe
        /// @details Initialisiert die Pumpe mit den für Natronlauge spezifischen Pins und Parametern
        PumpPeristalticNAOH(const String& taskName)
            : PeristalticPumps(taskName, NAOH_PUMP, NAOH_MON, "pump/naoh", TIMEOUT_NAOH_PUMP) {}
    };
} // end of namespace Services

