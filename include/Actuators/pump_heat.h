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
    class Pump_heat
    {
        uint8_t _pump_pin;

    private:
        class State : public Message
        {
            Pump_heat& _parent; // Referenz zur äußeren Klasse
        public:
            State(Pump_heat& parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Pump_heat(const uint8_t pump_pin) : _pump_pin(pump_pin)       // changed by Kucky
        {
            LOGGER_NOTICE("Create heat pump");
            pinMode(pump_pin, OUTPUT);
            msgBroker.registerMessage(new State(*this, "inGarden/heat_pump/state"));
        };
    };
    bool Pump_heat::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            LOGGER_NOTICE("Heat Pump ON");
            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            LOGGER_NOTICE("Heat Pump OFF");
            digitalWrite(_parent._pump_pin, LOW);
        }
        // State-Handler registrieren
        _network->pubMsg("outGarden/heat_pump/state", payload);
        return payload["state"];
    };
} // End namespace Actuators