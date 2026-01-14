#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include <TaskManager.h>
#include "../network.h"
#include <Wire.h>
#include <DallasTemperature.h>

extern Network *_network;

namespace Services
{
    class PH_Placebo : public Task::Base
    {
        char _msg[30];

    public:
        PH_Placebo(const String &name) : Task::Base(name)
        {
            LOGGER_NOTICE("Create Temperature Task");
        }
        virtual void begin() override
        {
            LOGGER_NOTICE("Vor getäuschte pH Anzeige");
        }
        PH_Placebo *init(const uint8_t DallasPin)
        {
            LOGGER_VERBOSE("enter ...");

            LOGGER_VERBOSE("leave ...");
            return this;
        }

        virtual void update() override
        {
            static float phValue = 0;
            static float phValue_min = 1000;
            static float phValue_max = -1000;

            LOGGER_NOTICE_FMT("Simulierter pH-Wert: %.1f\n", phValue);

            if (phValue < phValue_min)
            {
                phValue_min = phValue;
                sprintf(_msg, "{ \"value\":%.1f }", phValue_min);
                _network->pubMsg("outGarden/ph_min", _msg);
                LOGGER_NOTICE_FMT("ph min: %.1f", phValue_min);
            }

            if (phValue >= phValue_max)
            {
                phValue_max = phValue;
                sprintf(_msg, "{ \"value\":%.1f }", phValue_max);
                _network->pubMsg("outGarden/ph_max", _msg);
                LOGGER_NOTICE_FMT("ph max: %.1f", phValue_max);
            }

            // Zufälliger pH-Wert zwischen 6.00 und 8.00
            phValue = random(600, 801) / 100.0;

            sprintf(_msg, "{ \"value\":%.1f }", phValue);
            _network->pubMsg("outGarden/ph_value", _msg);
        }
    };
} // End namespace Services