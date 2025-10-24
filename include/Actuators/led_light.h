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
    class Led_lights
    {
        uint8_t _LED_red_pin; // to be refactored
        uint8_t _LED_green_pin;
        uint8_t _LED_blue_pin;

    private:
        class State : public Message
        {
            Led_lights &_parent; // Referenz zur äußeren Klasse
        public:
            State(Led_lights &parent, String topic) : Message(topic), _parent(parent) {}
            bool call(JsonDocument payload);
        };

    public:
        Led_lights(const uint8_t LED_red_pin, const uint8_t LED_green_pin, const uint8_t LED_blue_pin)
            : _LED_red_pin(LED_red_pin), _LED_green_pin(LED_green_pin), _LED_blue_pin(LED_blue_pin)

        {
            LOGGER_NOTICE("Create LED stripes");
            pinMode(LED_red_pin, OUTPUT);
            digitalWrite(LED_red_pin, LOW);
            pinMode(LED_green_pin, INPUT);
            digitalWrite(LED_green_pin, LOW);
            pinMode(LED_blue_pin, INPUT);
            digitalWrite(LED_blue_pin, LOW);

            msgBroker.registerMessage(new State(*this, "inGarden/led_lights/state"));
        };
    };
    bool Led_lights::State::call(JsonDocument payload)
    {
        if (payload["state"])
        {
            Serial.println("LED Lights ON");
            digitalWrite(_parent._LED_red_pin, HIGH);
            digitalWrite(_parent._LED_green_pin, HIGH);
            digitalWrite(_parent._LED_blue_pin, HIGH);
        }
        else
        {
            Serial.println("Lights Pump OFF");
            digitalWrite(_parent._LED_red_pin, LOW);
            digitalWrite(_parent._LED_green_pin, LOW);
            digitalWrite(_parent._LED_blue_pin, LOW);

        }
        _network->pubMsg("outGarden/Led_lights/state", payload);
        return payload["state"];
    };
} // End namespace Actuators