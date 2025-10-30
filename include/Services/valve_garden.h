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
    class Valve_garden
    {
        uint8_t _valve_pin;

    private:
        class State : public Message
        {
            Valve_garden& _parent;
        public:
            State(Valve_garden& parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Valve_garden(const uint8_t pump_pin) : _valve_pin(pump_pin)
        
        {
            LOGGER_NOTICE("Create garden valve");
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            msgBroker.registerMessage(new State(*this,"inGarden/garden_valve/state"));
        };
    };
    bool Valve_garden::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            LOGGER_NOTICE("Garden valve ON");
            digitalWrite(_parent._valve_pin, HIGH);
        }
        else
        {
            LOGGER_NOTICE("Garden valve OFF");
            digitalWrite(_parent._valve_pin, LOW);
        }
        _network->pubMsg("outGarden/garden_valve/state", payload);
        return payload["state"];
    };
}// End namespace Services