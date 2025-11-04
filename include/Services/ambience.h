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
        // --- LED Pins und Farbwerte ---
        static uint8_t _ledRedPin;
        static uint8_t _ledGreenPin;
        static uint8_t _ledBluePin;
        static uint8_t _r, _g, _b;

        // --- State Message ---
        class State : public Message
        {
        public:
            explicit State(const String &topic) : Message(topic) {}
            bool call(JsonDocument payload) override;
        };

        // --- Color Message ---
        class Color : public Message
        {
        public:
            explicit Color(const String &topic) : Message(topic) {}
            bool call(JsonDocument payload) override;
        };

    public:
        Ambience(uint8_t redPin, uint8_t greenPin, uint8_t bluePin)
        {
            LOGGER_NOTICE("Create LED stripes");

            // Save Pins 
            _ledRedPin = redPin;
            _ledGreenPin = greenPin;
            _ledBluePin = bluePin;

            // Initialize pins
            pinMode(_ledRedPin, OUTPUT);
            pinMode(_ledGreenPin, OUTPUT);
            pinMode(_ledBluePin, OUTPUT);

            // Reset colors
            _r = _g = _b = 0;

            // Register messages
            msgBroker.registerMessage(new State("inGarden/ambient/state"));
            msgBroker.registerMessage(new Color("inGarden/ambient/color"));
        }

        // --- Static auxiliary functions for LED control ---
        static void setColor(uint8_t r, uint8_t g, uint8_t b)
        {
            _r = r;
            _g = g;
            _b = b;

            analogWrite(_ledRedPin, 255 - _r);
            analogWrite(_ledGreenPin, 255 - _g);
            analogWrite(_ledBluePin, 255 - _b);
        }

        static void setOff()
        {
            analogWrite(_ledRedPin, 255);
            analogWrite(_ledGreenPin, 255);
            analogWrite(_ledBluePin, 255);
        }
    };

    // --- Definition of static variables ---
    uint8_t Ambience::_ledRedPin = 0;
    uint8_t Ambience::_ledGreenPin = 0;
    uint8_t Ambience::_ledBluePin = 0;
    uint8_t Ambience::_r = 0;
    uint8_t Ambience::_g = 0;
    uint8_t Ambience::_b = 0;

    // --- Implementations of the inner classes ---
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
        uint8_t r = payload["color"]["r"];
        uint8_t g = payload["color"]["g"];
        uint8_t b = payload["color"]["b"];

        Ambience::setColor(r, g, b);
        return _network->pubMsg("inGarden/ambient/color", payload);
    }

} // namespace Services
