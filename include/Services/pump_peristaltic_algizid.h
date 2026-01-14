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
    /// @brief Klasse für die Peristaltik-Pumpe zur Dosierung von Algizid
    /// @details Diese Klasse erbt von der PeristalticPumps-Basisklasse und ist speziell 
    ///          für die Steuerung einer Algizid-Pumpe konfiguriert.
    class PumpPeristalticAlgizid : public PeristalticPumps
    {
    public:
        /// @brief Konstruktor für die Algizid-Pumpe
        /// @details Initialisiert die Pumpe mit den für Algizid spezifischen Pins und Parametern
        PumpPeristalticAlgizid()
            : PeristalticPumps(ALGIZID_PUMP, ALGIZID_MON, "pool/pump/algizid", DEBOUNCE_TIME, TIMEOUT_ALGIZID_PUMP) {}
    };
} // end of namespace Services

