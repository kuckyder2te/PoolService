#pragma once
#include <Arduino.h>
#include "myLogger.h"
#include "../message.h"
#include "../messageBroker.h"

namespace Services
{
    class Ambience
    {
    private:
        const uint8_t _ledRedPin;
        const uint8_t _ledGreenPin;
        const uint8_t _ledBluePin;

        static uint8_t _r, _g, _b;

        // --- State Message ---
        class State : public Message
        {
        public:
            explicit State(const String& topic) : Message(topic) {}
            bool call(JsonDocument payload) override;
        };

        // --- Color Message ---
        class Color : public Message
        {
        public:
            explicit Color(const String& topic) : Message(topic) {}
            bool call(JsonDocument payload) override;
        };

    public:
        Ambience(uint8_t redPin, uint8_t greenPin, uint8_t bluePin)
            : _ledRedPin(redPin), _ledGreenPin(greenPin), _ledBluePin(bluePin)
        {
            LOGGER_NOTICE("Create LED stripes");

            _r = _g = _b = 0;

            pinMode(_ledRedPin, OUTPUT);
            pinMode(_ledGreenPin, OUTPUT);
            pinMode(_ledBluePin, OUTPUT);

            msgBroker.registerMessage(new State("inGarden/ambient/state"));
            msgBroker.registerMessage(new Color("inGarden/ambient/color"));
        }

        // optional: Setter zum direkten Ändern ohne Message
        static void setColor(uint8_t r, uint8_t g, uint8_t b)
        {
            _r = r;
            _g = g;
            _b = b;
            analogWrite(LED_STRIPE_RED, 255 - _r);
            analogWrite(LED_STRIPE_GREEN, 255 - _g);
            analogWrite(LED_STRIPE_BLUE, 255 - _b);
        }

        static void setOff()
        {
            analogWrite(LED_STRIPE_RED, 255);
            analogWrite(LED_STRIPE_GREEN, 255);
            analogWrite(LED_STRIPE_BLUE, 255);
        }
    };

    // --- Static Member Definition ---
    uint8_t Ambience::_r = 0;
    uint8_t Ambience::_g = 0;
    uint8_t Ambience::_b = 0;

    // --- Implementations ---

    bool Ambience::State::call(JsonDocument payload)
    {
        bool state = payload["value"];
        LOGGER_NOTICE_FMT("Set State to: %d", state);

        if (state)
            Ambience::setColor(_r, _g, _b);
        else
            Ambience::setOff();

        return _network->pubMsg("inGarden/ambient/state", payload);
    }

    bool Ambience::Color::call(JsonDocument payload)
    {
        LOGGER_NOTICE_FMT("Set Color %s", payload["color"]);
        _r = payload["color"]["r"];
        _g = payload["color"]["g"];
        _b = payload["color"]["b"];

        Ambience::setColor(_r, _g, _b);

        return _network->pubMsg("inGarden/ambient/color", payload);
    }

} // namespace Services
