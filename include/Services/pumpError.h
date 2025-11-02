#pragma once
/// @cond
#include <Arduino.h>
//#define LOCAL_DEBUG
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
    private:
        struct PumpInfo
        {
            uint8_t pumpPin;
            uint8_t monitorPin;
            const char *name;
            bool lastError = false;
            uint8_t stableCount = 0;
            bool lastMonitorState = false;
            unsigned long pumpOnSince = 0; // für Timeout-Erkennung
        };

        static const uint8_t NUM_PUMPS = 3;
        PumpInfo pumps[NUM_PUMPS];

        // wie viele aufeinanderfolgende gleiche Werte nötig sind, um stabil zu sein
        static const uint8_t debounceLimit = 3;
        static const unsigned long timeoutMs = 10000; // 10 Sekunden Timeout

    public:
        pumpError(const String &name)
            : Task::Base(name)
        {
        }

        pumpError *init(const uint8_t algizid_mon, const uint8_t algizid_pump,
                        const uint8_t hcl_mon, const uint8_t hcl_pump,
                        const uint8_t naoh_mon, const uint8_t naoh_pump)
        {
            LOGGER_VERBOSE("enter pumpError::init ..."); 
            {
                pumps[0].pumpPin = algizid_pump;
                pumps[0].monitorPin = algizid_mon;
                pumps[0].name = "Algizid";

                pumps[1].pumpPin = hcl_pump;
                pumps[1].monitorPin = hcl_mon;
                pumps[1].name = "HCl";

                pumps[2].pumpPin = naoh_pump;
                pumps[2].monitorPin = naoh_mon;
                pumps[2].name = "NaOH";

                for (uint8_t i = 0; i < NUM_PUMPS; i++)
                {
                    pinMode(pumps[i].pumpPin, INPUT);
                    digitalWrite(pumps[i].pumpPin, LOW); // sicherheitshalber aus
                    pinMode(pumps[i].monitorPin, INPUT);
                }

                LOGGER_NOTICE("PumpError task initialized");
                return this;
            }
        }

        virtual void begin() override
        {
            LOGGER_NOTICE("Pump errors startet");
        }

        virtual void update() override
        {
            {
                for (uint8_t i = 0; i < NUM_PUMPS; i++)
                {
                    checkPump(pumps[i]);
                }
            }
        }

    private:
        void checkPump(PumpInfo &pump)
        {
            bool pumpState = digitalRead(pump.pumpPin);
            bool monState = digitalRead(pump.monitorPin);

            // --- Debouncing ---
            if (monState == pump.lastMonitorState)
            {
                if (pump.stableCount < debounceLimit)
                    pump.stableCount++;
            }
            else
            {
                pump.stableCount = 0;
                pump.lastMonitorState = monState;
            }

            // --- Timeout-Überwachung ---
            if (pumpState == HIGH)
            {
                if (pump.pumpOnSince == 0)
                    pump.pumpOnSince = millis();

                if (monState == LOW && (millis() - pump.pumpOnSince > timeoutMs))
                {
                    if (!pump.lastError)
                    {
                        pump.lastError = true;
                        sendStatus(pump, true, "Timeout: no signal from monitor");
                    }
                }
            }
            else
            {
                pump.pumpOnSince = 0; // zurücksetzen, wenn Pumpe aus
            }

            // --- Normaler Fehlzustand (nach Debounce stabil) ---
            if (pump.stableCount >= debounceLimit)
            {
                bool error = (pumpState == HIGH && monState == LOW);

                if (error != pump.lastError)
                {
                    pump.lastError = error;
                    sendStatus(pump, error);
                }
            }
        }

        void sendStatus(const PumpInfo &pump, bool error, const char *reason = nullptr)
        {
            JsonDocument msg;
            msg["pump"] = pump.name;
            msg["error"] = error;
            msg["timestamp"] = millis();
            if (reason)
                msg["reason"] = reason;

            String topic = String("outGarden/pump/") + pump.name + "/error";
            _network->pubMsg(topic.c_str(), msg);

            if (error)
            {
                if (reason)
                {
                    LOGGER_ERROR_FMT("%s pump error: %s", pump.name, reason);
                }
                else
                {
                    LOGGER_ERROR_FMT("%s pump error detected!", pump.name);
                }
            }
            else
            {
                LOGGER_NOTICE_FMT("%s pump back to normal", pump.name);
            }
        }
    };
} // End namespace Services