#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include <TaskManager.h>
#include "../message.h"
#include "../messageBroker.h"
#include "../network.h"

extern Network *_network;

namespace Services
{
    class Ambience : public Task::Base
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
            bool call(JsonDocument payload);
        };

        class Color : public Message
        {
        public:
            Color(String topic) : Message(topic) {}
            bool call(JsonDocument payload); // Hier der call Prototyp aus der Message Class
        };

    private:
        bool _initialized = false;

    public:
        Ambience(const String &name) : Task::Base(name)
        {
            LOGGER_NOTICE("Create LED stripes");
            _r = 0;
            _g = 0;
            _b = 0;
            msgBroker.registerMessage(new State("inGarden/ambient/state"));
            msgBroker.registerMessage(new Color("inGarden/ambient/color"));
        }

        Ambience* init(uint8_t led_red, uint8_t led_green, uint8_t led_blue) 
        {
            _LED_red_pin = led_red;
            _LED_green_pin = led_green;
            _LED_blue_pin = led_blue;
            _initialized = true;
            return this;
        }

        virtual void begin() override
        {
            LOGGER_NOTICE("Ambience Task started");
            // Initialize LED pins
            pinMode(_LED_red_pin, OUTPUT);
            pinMode(_LED_green_pin, OUTPUT);
            pinMode(_LED_blue_pin, OUTPUT);
            
            // Turn off LEDs initially
            analogWrite(_LED_red_pin, 255);
            analogWrite(_LED_green_pin, 255);
            analogWrite(_LED_blue_pin, 255);
        }

        virtual void update() override
        {
            // The update function is called every 100ms
            // The actual LED control is handled by the MQTT callbacks
            // This function can be used for additional periodic tasks if needed
            LOGGER_NOTICE("Tick");
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
        return _network->pubMsg("inGarden/ambient/state", payload);
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
        return _network->pubMsg("inGarden/ambient/color", payload);
    }

} // End namespace Services
