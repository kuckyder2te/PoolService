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
    class Pump_naoh
    {
        uint8_t _pump_pin; // to be refactored
        uint8_t _monitor_pin;

    private:
        class State : public Message
        {
            Pump_naoh& _parent; // Referenz zur äußeren Klasse
        public:
            // State(String topic) : Message(topic) {};
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
            msgBroker.registerMessage(new State("inGarden/naoh_pump/state"));
        };
    };
    bool Pump_naoh::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            Serial.println("NaOh Pump ON");
            // digitalWrite(Pump_naoh::_pump_pin, HIGH);
            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            Serial.println("NaOh Pump OFF");
            // digitalWrite(Pump_naoh::_pump_pin, LOW);
            digitalWrite(_parent._pump_pin, LOW);
        }
        _network->pubMsg("outGarden/naoh_pump/state", payload);
        return payload["state"];
    };
}