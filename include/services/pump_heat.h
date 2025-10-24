#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "../message.h"
#include "../messageBroker.h"

namespace Services
{
    class Heat_pont
    {
        uint8_t _pump_pin; // to be refactored
        uint8_t _monitor_pin;

    private:
        class State : public Message
        {
            Heat_pont& _parent; // Referenz zur äußeren Klasse
        public:
            State(Heat_pont& parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Heat_pont(const uint8_t pump_pin, const uint8_t monitor_pin) : _pump_pin(pump_pin), _monitor_pin(monitor_pin)       
        {
            LOGGER_NOTICE("Create");
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            pinMode(monitor_pin, INPUT);
            digitalWrite(monitor_pin, LOW);
            msgBroker.registerMessage(*this, new State(*this, "inGarden/heat_pump/state"));
        };
    };
    bool Heat_pont::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            Serial.println("Heat Pump ON");
            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            Serial.println("Heat Pump OFF");
            digitalWrite(_parent._pump_pin, LOW);
        }
        // State-Handler registrieren
        _network->pubMsg("outGarden/heat_pump/state", payload);
        return payload["state"];
    };
}