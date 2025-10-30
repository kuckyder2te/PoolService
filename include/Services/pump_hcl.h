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
    class Pump_hcl
    {
        uint8_t _pump_pin;
        uint8_t _monitor_pin;

    private:
        // Innere Klasse für die Nachrichtenverarbeitung
        class State : public Message
        {
            Pump_hcl &_parent; // Referenz zur äußeren Klasse
        public:
            // State(String topic) : Message(topic) {};
            State(Pump_hcl &parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Pump_hcl(const uint8_t pump_pin, const uint8_t monitor_pin) : _pump_pin(pump_pin), _monitor_pin(monitor_pin)
        {
            LOGGER_NOTICE("Create HCL pump");
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            pinMode(monitor_pin, INPUT);
            digitalWrite(monitor_pin, LOW);
            msgBroker.registerMessage(new State(*this, "inGarden/hcl_pump/state"));
        };
    };
    bool Pump_hcl::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            LOGGER_NOTICE("HCl Pump ON");
            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            LOGGER_NOTICE("HCl Pump OFF");
            digitalWrite(_parent._pump_pin, LOW);
        }
        _network->pubMsg("outGarden/hcl_pump/state", payload);
        return payload["state"];
    };
} //End namespace Services