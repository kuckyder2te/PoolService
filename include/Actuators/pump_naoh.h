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
    class Pump_naoh
    {
        uint8_t _pump_pin;
        uint8_t _monitor_pin;

    private:
        class State : public Message
        {
            Pump_naoh& _parent;
        public:
            State(Pump_naoh& parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Pump_naoh(const uint8_t pump_pin, const uint8_t monitor_pin) : _pump_pin(pump_pin), _monitor_pin(monitor_pin)
        
        {
            LOGGER_NOTICE("Create");
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            pinMode(monitor_pin, INPUT);
            digitalWrite(monitor_pin, LOW);
            msgBroker.registerMessage(new State(*this, "inGarden/naoh_pump/state"));
        };
    };
    bool Pump_naoh::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            LOGGER_NOTICE("NaOh Pump ON");
            // digitalWrite(Pump_naoh::_pump_pin, HIGH);
            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            LOGGER_NOTICE("NaOh Pump OFF");
            // digitalWrite(Pump_naoh::_pump_pin, LOW);
            digitalWrite(_parent._pump_pin, LOW);
        }
        _network->pubMsg("outGarden/naoh_pump/state", payload);
        return payload["state"];
    };
} // END namespace Actuators