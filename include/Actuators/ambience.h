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
    class Ambience
    {
    private:
        uint8_t _LED_red_pin;
        uint8_t _LED_green_pin;
        uint8_t _LED_blue_pin;
        static uint8_t _r, _g, _b;

        class State : public Message
        {
        public:
            State(String topic) : Message(topic) {}
            bool call(JsonDocument paylod);
        };

        class Color : public Message
        {
        public:
            Color(String topic) : Message(topic) {}
            bool call(JsonDocument paylod); // Hier der call Prototyp aus der Message Class
        };

    public:
        Ambience(const uint8_t led_red, uint8_t led_green, uint8_t led_blue)
        {
            LOGGER_NOTICE("Create LED stripes");
            _r = 0;
            _g = 0;
            _b = 0;
            msgBroker.registerMessage(new State("/inGarden/ambient/state"));
            msgBroker.registerMessage(new Color("/inGarden/ambient/color"));
        }
    };
    uint8_t Ambience::_r;
    uint8_t Ambience::_g;
    uint8_t Ambience::_b;

    bool Ambience::State::call(JsonDocument payload)
    { // Hier die call Implementierung des Prototyps
        LOGGER_NOTICE_FMT("Set State to: %d", (uint8_t)payload["value"]);
        if (payload["value"])
        {
            analogWrite(LED_STRIPE_RED, 255 - _r);
            analogWrite(LED_STRIPE_GREEN, 255 - _g);
            analogWrite(LED_STRIPE_BLUE, 255 - _b);
        }
        else
        {
            analogWrite(LED_STRIPE_RED, 255);
            analogWrite(LED_STRIPE_GREEN, 255);
            analogWrite(LED_STRIPE_BLUE, 255);
        }
        return _network->pubMsg("/inGarden/ambient/state", payload);
    }

    bool Ambience::Color::call(JsonDocument payload)
    {
        LOGGER_NOTICE_FMT("Set Color %s", payload["color"]);
        _r = payload["color"]["r"];
        _g = payload["color"]["g"];
        _b = payload["color"]["b"];
        analogWrite(LED_STRIPE_RED, 255 - _r);
        analogWrite(LED_STRIPE_GREEN, 255 - _g);
        analogWrite(LED_STRIPE_BLUE, 255 - _b);
        return _network->pubMsg("/inGarden/ambient/color", payload);
    }

} // End namespace Actuators