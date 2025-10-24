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
    class Valve_rinse
    {
        uint8_t _valve_pin; // to be refactored ??
    //   uint8_t _monitor_pin;

    private:
        class State : public Message
        {
            Valve_rinse& _parent;
        public:
            State(Valve_rinse& parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Valve_rinse(const uint8_t pump_pin) : _valve_pin(pump_pin)
        
        {
            LOGGER_NOTICE("Create rinse valve");
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            msgBroker.registerMessage(new State(*this,"inGarden/rinse_valve/state"));
        };
    };
    bool Valve_rinse::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            Serial.println("Rinse valve ON");
            digitalWrite(_parent._valve_pin, HIGH);
        }
        else
        {
            Serial.println("Rinse valve OFF");
            digitalWrite(_parent._valve_pin, LOW);
        }
        _network->pubMsg("outGarden/rinse:valve/state", payload);
        return payload["state"];
    };
}