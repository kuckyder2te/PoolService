#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include <TaskManager.h>
#include <PubSubClient.h>
#include "../network.h"

extern Network *_network;

namespace Services
{
    class pumpError : public Task::Base
    {
        // PubSubClient *_client;
        // char msg[30];
        uint8_t _pin_algizid;
        uint8_t _pin_algizid_mon; // Is that necessary?  Kucky
        uint8_t _pin_naoh;
        uint8_t _pin_naoh_mon;
        uint8_t _pin_hcl;
        uint8_t _pin_hcl_mon;

    private:
        struct PumpInfo
        {
            uint8_t pumpPin;
            uint8_t monitorPin;
            const char *name;
            bool lastError = false;
            uint8_t stableCount = 0;
            bool lastMonitorState = false;
        };

        static const uint8_t NUM_PUMPS = 3;
        PumpInfo pumps[NUM_PUMPS];

        // wie viele aufeinanderfolgende gleiche Werte nötig sind, um stabil zu sein
        static const uint8_t debounceLimit = 3;

    public:
        pumpError(const String &name)
            : Task::Base(name)
        {
        }

        pumpError *init(const uint8_t pin_algizid_mon, const uint8_t pin_algizid,
                        const uint8_t pin_hcl_mon, const uint8_t pin_hcl,
                        const uint8_t pin_naoh_mon, const uint8_t pin_naoh)
        {
            LOGGER_VERBOSE("enter ..."); // Is that necessary?  Kucky
            _pin_algizid = pin_algizid;
            _pin_algizid_mon = pin_algizid_mon;
            _pin_naoh = pin_naoh;
            _pin_naoh_mon = pin_naoh_mon;
            _pin_hcl = pin_hcl;
            _pin_hcl_mon = pin_hcl_mon;

            LOGGER_VERBOSE("leave ...");
            return this;
        }

        virtual void begin() override
        {
            LOGGER_NOTICE("Pump errors");
        }

        virtual void update() override
        {
            static bool algizid_err = false;
            static bool hcl_err = false;
            static bool naoh_err = false;

            if (digitalRead(_pin_algizid_mon) == digitalRead(_pin_algizid))
            {
                algizid_err = false;                                  // No malfunction
                _network->pubMsg("outGarden/algizid_error", "false"); // this is not clearly
            }
            else
            {
                algizid_err = true; // Malfunktion   must this be?
                _network->pubMsg("outGarden/algizid_error", "true");
            }

            if (digitalRead(_pin_naoh_mon) == digitalRead(_pin_naoh))
            {
                naoh_err = false; // No malfunction
                _network->pubMsg("outGarden/naoh_error", "false");
            }
            else
            {
                naoh_err = true; // Malfunktion   must this be?
                _network->pubMsg("outGarden/naoh_error", "true");
            }

            if (digitalRead(_pin_hcl_mon) == digitalRead(_pin_hcl))
            {
                algizid_err = false; // No malfunction
                _network->pubMsg("outGarden/hcl_error", "false");
            }
            else
            {
                algizid_err = true; // Malfunktion   must this be?
                _network->pubMsg("outGarden/hcl_error", "true");
            }
        }
    };
} // End namespace Services