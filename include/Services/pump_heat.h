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
            LOGGER_NOTICE_FMT("Create heat pump on Pin: ", _pump_pin);
            pinMode(pump_pin, OUTPUT);
            msgBroker.registerMessage(new State(*this, "heat_pump/state"));
        };
    };
    bool Pump_heat::State::call(JsonDocument payload)
    {
        if (payload.is<bool>())
        {
            if (payload.as<bool>())
            {            LOGGER_NOTICE_FMT("Heat Pump ON - Pin:",_parent._pump_pin);

            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            LOGGER_NOTICE_FMT("Heat Pump OFF - Pin:",_parent._pump_pin);
            digitalWrite(_parent._pump_pin, LOW);
        }
        // State-Handler registrieren
        _network->pubMsg("heat_pump/state", payload);
        }
        else
        {
            LOGGER_WARNING("Payload not bool"); // Improve by sending error code as mqtt message
            return false;
        }
        return true;
    }
} // End namespace Services