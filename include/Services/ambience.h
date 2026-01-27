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
        uint8_t _r, _g, _b;
        uint32_t _fadePeriodMs = 20000; // Default: 20s
        static Ambience *_instance;     // Static pointer to the instance

        // State machine states
        enum class LightState
        {
            OFF,
            ON,
            FADE,
            COLOR_CYCLE,
            BREATHING
        };

        LightState _currentState = LightState::OFF;
        LightState _targetState = LightState::ON; // Default = Static

        // For animations
        unsigned long _lastUpdate = 0;
        uint8_t _animationStep = 0;
        uint8_t _fadeDirection = 1;

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

        class Mode : public Message
        {
        public:
            Mode(String topic) : Message(topic) {}

            bool call(JsonDocument payload) override;
        };

        class FadeSpeed : public Message
        {
        public:
            FadeSpeed(String topic) : Message(topic) {}
            bool call(JsonDocument payload) override;
        };

    private:
        bool _initialized = false;

    public:
        Ambience(const String &name) : Task::Base(name)
        {
            LOGGER_NOTICE("Create LED stripes");
            LOGGER_NOTICE_FMT("Ambience instance: %p", this);
            _r = 0;
            _g = 0;
            _b = 0;
            _instance = this; // Store the instance pointer
            msgBroker.registerMessage(new State("pool/light/state"));
            msgBroker.registerMessage(new Color("pool/light/colors/rgb"));
            msgBroker.registerMessage(new Mode("pool/light/mode"));
            msgBroker.registerMessage(new FadeSpeed("pool/light/fadespeed"));
        }

        // Method to set the state
        void setState(LightState newState)
        {
            LOGGER_NOTICE_FMT("Current State=%d", (uint8_t)_currentState);
            _currentState = newState;
        }
        // Method to set the color
        void setColor(uint8_t r, uint8_t g, uint8_t b)
        {
            _r = r;
            _g = g;
            _b = b;
        }

        Ambience *init(uint8_t led_red, uint8_t led_green, uint8_t led_blue)
        {
            _LED_red_pin = led_red;
            _LED_green_pin = led_green;
            _LED_blue_pin = led_blue;
            // Initialize LED pins
            pinMode(_LED_red_pin, OUTPUT);
            pinMode(_LED_green_pin, OUTPUT);
            pinMode(_LED_blue_pin, OUTPUT);

            // Turn off LEDs initially
            analogWrite(_LED_red_pin, 255);
            analogWrite(_LED_green_pin, 255);
            analogWrite(_LED_blue_pin, 255);

            _initialized = true;
            return this;
        } //------------------------------- Ambience *init --------------------------//

        virtual void update() override
        {
            static uint32_t lastStateLog = 0;
            const uint32_t now = millis();
            if (now - lastStateLog > 1000)
            {
                lastStateLog = now;
                LOGGER_NOTICE_FMT("STATE=%d", (uint8_t)_currentState);
            } // neu

            if (!_initialized)
            {
                _targetState = LightState::ON;
                _initialized = true;
            }
            // State machine implementation
            switch (_currentState)
            {
            case LightState::OFF:
                if (now - lastStateLog > 1000) // neu
                {
                    lastStateLog = now;
                    LOGGER_NOTICE_FMT("STATE OFF =%d", (uint8_t)_currentState);
                } // neu
                handleOffState();
                break;
            case LightState::ON:
                if (now - lastStateLog > 1000)
                {
                    lastStateLog = now;
                    LOGGER_NOTICE_FMT("STATE ON =%d", (uint8_t)_currentState);
                } // neu
                handleOnState();
                break;
            case LightState::FADE:
                if (now - lastStateLog > 1000)
                {
                    lastStateLog = now;
                    LOGGER_NOTICE_FMT("STATE FADE =%d", (uint8_t)_currentState);
                } // neu
                handleFadeState();
                break;
            case LightState::COLOR_CYCLE:
                if (now - lastStateLog > 1000)
                {
                    lastStateLog = now;
                    LOGGER_NOTICE_FMT("STATE COLOR_CYCLE =%d", (uint8_t)_currentState);
                } // neu
                handleColorCycleState();
                break;
            case LightState::BREATHING:
                if (now - lastStateLog > 1000)
                {
                    lastStateLog = now;
                    LOGGER_NOTICE_FMT("STATE BREATHING=%d", (uint8_t)_currentState);
                } // neu
                handleBreathingState();
                break;
            }
        } //------------------------------- update  ----------------------------------//

    private:
        void handleOffState()
        {
            // Turn off all LEDs
            analogWrite(_LED_red_pin, 255);
            analogWrite(_LED_green_pin, 255);
            analogWrite(_LED_blue_pin, 255);
        } //-------------------------- handleOffState ---------------------------------//

        void handleOnState()
        {
            // Keep LEDs at current color
            analogWrite(_LED_red_pin, 255 - _r);
            analogWrite(_LED_green_pin, 255 - _g);
            analogWrite(_LED_blue_pin, 255 - _b);
        } //-------------------------- handleOnState ----------------------------------//

        void handleFadeState()
        {
            // Fade between colors
            LOGGER_NOTICE("AnimationStep");
            unsigned long currentTime = millis();
            if (currentTime - _lastUpdate >= 50)
            { // Update every 50ms
                _lastUpdate = currentTime;

                // Simple fade implementation - cycle through colors
                _animationStep = (_animationStep + 1) % 255;
                uint8_t value = _animationStep;

                analogWrite(_LED_red_pin, 255 - value);
                analogWrite(_LED_green_pin, 255 - ((value + 85) % 255));
                analogWrite(_LED_blue_pin, 255 - ((value + 170) % 255));
                LOGGER_NOTICE_FMT("AnimationStep %i value %i ", _animationStep, value);
            }
        } //-------------------------- handleFadeState --------------------------------//

        void handleColorCycleState()
        {
            // Cycle through predefined colors
            unsigned long currentTime = millis();
            if (currentTime - _lastUpdate >= 1000)
            { // Change color every second
                _lastUpdate = currentTime;

                // Cycle through colors: Red, Green, Blue, Yellow, Purple, Cyan, White
                switch (_animationStep % 7)
                {
                case 0: // Red
                    analogWrite(_LED_red_pin, 0);
                    analogWrite(_LED_green_pin, 255);
                    analogWrite(_LED_blue_pin, 255);
                    break;
                case 1: // Green
                    analogWrite(_LED_red_pin, 255);
                    analogWrite(_LED_green_pin, 0);
                    analogWrite(_LED_blue_pin, 255);
                    break;
                case 2: // Blue
                    analogWrite(_LED_red_pin, 255);
                    analogWrite(_LED_green_pin, 255);
                    analogWrite(_LED_blue_pin, 0);
                    break;
                case 3: // Yellow
                    analogWrite(_LED_red_pin, 0);
                    analogWrite(_LED_green_pin, 0);
                    analogWrite(_LED_blue_pin, 255);
                    break;
                case 4: // Purple
                    analogWrite(_LED_red_pin, 0);
                    analogWrite(_LED_green_pin, 255);
                    analogWrite(_LED_blue_pin, 0);
                    break;
                case 5: // Cyan
                    analogWrite(_LED_red_pin, 255);
                    analogWrite(_LED_green_pin, 0);
                    analogWrite(_LED_blue_pin, 0);
                    break;
                case 6: // White
                    analogWrite(_LED_red_pin, 0);
                    analogWrite(_LED_green_pin, 0);
                    analogWrite(_LED_blue_pin, 0);
                    break;
                }
                _animationStep++;
            }
        } //--------------------------- handleColorCycleState -------------------------//

        void handleBreathingState()
        {
            // Breathing effect (fade in and out)
            unsigned long currentTime = millis();
            if (currentTime - _lastUpdate >= 30)
            { // Update every 30ms
                _lastUpdate = currentTime;

                // Calculate breathing effect using sine wave approximation
                float angle = (_animationStep * 3.14159 * 2) / 255;
                uint8_t brightness = (sin(angle) + 1) * 127.5;
                _animationStep = (_animationStep + 1) % 255;

                // Apply brightness to current color
                uint8_t red = (brightness * (255 - _r)) / 255;
                uint8_t green = (brightness * (255 - _g)) / 255;
                uint8_t blue = (brightness * (255 - _b)) / 255;

                analogWrite(_LED_red_pin, 255 - red);
                analogWrite(_LED_green_pin, 255 - green);
                analogWrite(_LED_blue_pin, 255 - blue);
            }
        } //--------------------------- handleBreathingState --------------------------//
    };

    bool Ambience::State::call(JsonDocument payload)
    { // Hier die call Implementierung des Prototyps

        const uint8_t v = (uint8_t)(payload["value"] | 0);
        LOGGER_NOTICE_FMT("Set State to: %u", v);

        if (_instance == nullptr)
            return false;

        if (v)
        {
            // AN: starte den aktuell gewählten Mode
            _instance->setState(_instance->_targetState);
        }
        else
        {
            // AUS: hart aus
            _instance->setState(LightState::OFF);
        }
        // LOGGER_NOTICE_FMT("Set State to: %d", (uint8_t)payload["value"]);
        // LOGGER_NOTICE_FMT("State call instance: %p", _instance);

        // if (_instance == nullptr)
        //     return false; // neu

        // if ((uint8_t)payload["value"])
        // {
        //     _instance->setState(_instance->_targetState); // Neu ChatGPT
        // }
        // else
        // {
        //     _instance->setState(LightState::OFF);
        // }
        return _network->pubMsg("pool/light/state", payload);
    } // ----------------------------- Ambience::State::call --------------------------//

    bool Ambience::Color::call(JsonDocument payload)
    {
        LOGGER_NOTICE("Enter");
        _instance->setColor(payload["value"]["r"], payload["value"]["g"], payload["value"]["b"]);
        return _network->pubMsg("pool/light/colors/rgb", payload);
    } //------------------------------- bool Ambience::Color::call -------------------//

    bool Ambience::Mode::call(JsonDocument payload)
    {
        const uint8_t m = (uint8_t)(payload["value"] | 1);
        LOGGER_NOTICE_FMT("Mode command received: %d", m);
        LOGGER_NOTICE_FMT("Mode call instance: %p", _instance);
        // Mapping:
        // 1 = Static
        // 2 = Green <-> Blue (Fade)
        // 3 = Rainbow
        LightState target = LightState::ON;

        switch (m)
        {
        case 1:
            target = LightState::ON;
            break;
        case 2:
            target = LightState::FADE;
            break;
        case 3:
            target = LightState::COLOR_CYCLE;
            break;
        default:
            target = LightState::ON;
            break;
        }

        _instance->_targetState = target;

        LOGGER_NOTICE_FMT("targetState now: %d", (uint8_t)_instance->_targetState);

        // Wenn Licht nicht OFF ist, sofort umschalten
        if (_instance->_currentState != LightState::OFF)
        {
            _instance->setState(target);
        }

        return true;
    } //------------------------------- bool Ambience::Mode::call ---------------------//

    bool Ambience::FadeSpeed::call(JsonDocument payload)
    {
        uint32_t s = payload["value"] | 20; // z. B. Sekunden
        LOGGER_NOTICE_FMT("FadeSpeed set to %u", s);

        _instance->_fadePeriodMs = s * 1000;
        return true;
    } // ----------------------------- Ambience::FadeSpeed::call ----------------------//

    Ambience *Ambience::_instance = nullptr; // Initialize the static instance pointer

} // End namespace Services
