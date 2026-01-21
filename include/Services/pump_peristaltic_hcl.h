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
    /// @brief Klasse für die Peristaltik-Pumpe zur Dosierung von Salzsäure (HCL)
    /// @details Diese Klasse erbt von der PeristalticPumps-Basisklasse und ist speziell 
    ///          für die Steuerung einer Salzsäure-Pumpe konfiguriert.
    class PumpPeristalticHCL : public PeristalticPumps
    {
    public:
        /// @brief Konstruktor für die HCL-Pumpe
        /// @details Initialisiert die Pumpe mit den für Salzsäure spezifischen Pins und Parametern
        PumpPeristalticHCL(const String& taskName)
            : PeristalticPumps(taskName) {}
    };
} // end of namespace Services

