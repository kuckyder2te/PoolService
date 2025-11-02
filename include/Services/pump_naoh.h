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
        uint8_t _pump_pin;
        uint8_t _monitor_pin;

    private:
        class State : public Message
        {
            Pump_naoh &_parent;

        public:
            State(Pump_naoh &parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Pump_naoh(const uint8_t pump_pin) : _pump_pin(pump_pin)
        {
            LOGGER_NOTICE_FMT("Create NaOH pumps on Pin: %d",pump_pin);
            pinMode(pump_pin, OUTPUT);
            digitalWrite(pump_pin, LOW);
            msgBroker.registerMessage(new State(*this, "naoh_pump/state"));
        };
    };
    bool Pump_naoh::State::call(JsonDocument payload)
    {
        if(payload.is<bool>()){
            if (payload.as<bool>())
            {
                LOGGER_NOTICE_FMT("NaOH Pump ON - Pin: %d",_parent._pump_pin);
                digitalWrite(_parent._pump_pin, HIGH);
            }
            else
            {
                LOGGER_NOTICE_FMT("NaOH Pump OFF - Pin: %d",_parent._pump_pin);
                digitalWrite(_parent._pump_pin, LOW);
            }
            _network->pubMsg("naoh_pump/state", payload);
        }else{
            LOGGER_WARNING("Payload not bool");     //Improve by sending error code as mqtt message
            return false;
        }
        return true;
    };
} // END namespace Services