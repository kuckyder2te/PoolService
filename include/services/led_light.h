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
    class Led_lights
    {
        uint8_t _pump_pin; // to be refactored
        uint8_t _monitor_pin;

    private:
        class State : public Message
        {
            Led_lights& _parent; // Referenz zur äußeren Klasse
        public:
            State(Led_lights& parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Led_lights(const uint8_t pump_pin, const uint8_t monitor_pin) : _pump_pin(pump_pin), _monitor_pin(monitor_pin)
        
        {
            LOGGER_NOTICE("Create");
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            pinMode(monitor_pin, INPUT);
            digitalWrite(monitor_pin, LOW);
            msgBroker.registerMessage(new State(*this,"inGarden/led_lights/state"));
        };
    };
    bool Led_lights::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            Serial.println("LED Lights ON");
            digitalWrite(_parent._pump_pin, HIGH);
        }
        else
        {
            Serial.println("Lights Pump OFF")
            digitalWrite(_parent._pump_pin, LOW);
        }
        _network->pubMsg("outGarden/Led_lights/state", payload);
        return payload["state"];
    };
}