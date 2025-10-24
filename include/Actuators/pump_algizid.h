#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "../message.h"
#include "../messageBroker.h"

namespace Actuators
{
    class Pump_algizid
    {
        uint8_t _pump_pin; // to be refactored ??
        uint8_t _monitor_pin;

    private:
        class State : public Message
        {
            Pump_algizid& _parent;
        public:
            State(Pump_algizid& parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Pump_algizid(const uint8_t pump_pin, const uint8_t monitor_pin) : _pump_pin(pump_pin), _monitor_pin(monitor_pin)
        
        {
            LOGGER_NOTICE("Create");
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            pinMode(monitor_pin, INPUT);
            digitalWrite(monitor_pin, LOW);
            msgBroker.registerMessage(new State(*this,"inGarden/naoh_pump/state"));
        };
    };
    bool Pump_algizid::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            Serial.println("Algizid Pump ON");
            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            Serial.println("Algizid Pump OFF");
            digitalWrite(_parent._pump_pin, LOW);
        }
        _network->pubMsg("outGarden/algizid_pump/state", payload);
        return payload["state"];
    };
}