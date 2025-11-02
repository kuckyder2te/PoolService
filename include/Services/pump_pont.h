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
    class Pump_pont
    {
        uint8_t _pump_pin;

    private:
        // Innere Klasse für die Nachrichtenverarbeitung
        class State : public Message
        {
            Pump_pont &_parent; // Referenz zur äußeren Klasse
        
            public:
            State(Pump_pont &parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Pump_pont(const uint8_t pump_pin) : _pump_pin(pump_pin) // changed by Kucky
        {
            LOGGER_NOTICE_FMT("Create pond pump on Pin: %d", _pump_pin);
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            msgBroker.registerMessage(new State(*this, "Pont_pump/state"));
        };
    };

    // Definition der State-Methoden außerhalb der Klasse
    bool Pump_pont::State::call(JsonDocument payload)
    {
        if (payload.is<bool>())
        {
            if (payload.as<bool>())
            {
                LOGGER_NOTICE_FMT("Pont pump ON - Pin: %d", _parent._pump_pin);
                digitalWrite(_parent._pump_pin, HIGH);
            }
            else
            {
                LOGGER_NOTICE_FMT("Pont pump OFF - Pin: %d", _parent._pump_pin);
                digitalWrite(_parent._pump_pin, LOW);
            }
            _network->pubMsg("pont_pump/state", payload);
        }
        else
        {
            LOGGER_WARNING("Payload not bool"); // Improve by sending error code as mqtt message
            return false;
        }
        return true;
    }
    } // End namespace Services