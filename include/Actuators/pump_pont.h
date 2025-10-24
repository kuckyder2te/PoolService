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
    class Pump_pont
    {
        uint8_t _pump_pin; // to be refactored
      //uint8_t _monitor_pin;

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
        //Pump_pont(const uint8_t pump_pin, const uint8_t monitor_pin) : _pump_pin(pump_pin), _monitor_pin(monitor_pin)
       Pump_pont(const uint8_t pump_pin) : _pump_pin(pump_pin)   // changed by Kucky
        {
            LOGGER_NOTICE("Create pond pump");
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            // pinMode(monitor_pin, INPUT);
            // digitalWrite(monitor_pin, LOW);
            msgBroker.registerMessage(new State(*this, "inGarden/Pont_pump/state"));
        };
    };

    // Definition der State-Methoden außerhalb der Klasse
    bool Pump_pont::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            Serial.println("Pont Pump ON");
            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            Serial.println("Pont Pump OFF");
            digitalWrite(_parent._pump_pin, LOW);
        }
        _network->pubMsg("outGarden/pont_pump/state", payload);
        return payload["state"];
    };
} // namespace Actuators