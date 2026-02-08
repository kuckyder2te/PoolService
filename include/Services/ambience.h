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
        char _msg[64];

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
            //   DebugOutput("STATE");

            if (!_initialized)
            {
                _targetState = LightState::ON;
                _initialized = true;
            }
            // State machine implementation
            switch (_currentState)
            {
            case LightState::OFF:
                //   DebugOutput("STATE OFF");
                handleOffState();
                break;
            case LightState::ON:
                //   DebugOutput("STATE ON");
                handleOnState();
                break;
            case LightState::FADE:
                DebugOutput("STATE FADE");
                handleFadeState();
                break;
            case LightState::COLOR_CYCLE:
                //   DebugOutput("STATE OFF");
                handleColorCycleState();
                break;
            case LightState::BREATHING:
                //   DebugOutput("STATE BREATHING");
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
            const uint32_t now = millis();
            static uint32_t lastPub = 0;

            // Periodendauer für eine komplette Runde (grün->blau->grün)
            uint32_t periodMs = _fadePeriodMs;
            if (periodMs < 1000)
                periodMs = 1000; // Mindestwert 1s, schützt vor 0/zu klein

            // Normierte Phase 0..1
            const float t = (float)(now % periodMs) / (float)periodMs;

            // Dreieckswelle: 0..1..0 (weich und stabil)
            const float tri = (t < 0.5f) ? (t * 2.0f) : (2.0f - t * 2.0f);

            // Grün -> Blau -> Grün
            const uint8_t r = 0;
            const uint8_t g = (uint8_t)(255.0f * (1.0f - tri));
            const uint8_t b = (uint8_t)(255.0f * tri);

            // r, g, b sind uint8_t (0..255)
            if (now - lastPub >= 500)  // nur Debug sonst 500
            {
                lastPub = now;

                char msg[64];
                snprintf(msg, sizeof(msg),
                         "{\"value\":{\"r\":%u,\"g\":%u,\"b\":%u}}",
                         (unsigned)r, (unsigned)g, (unsigned)b);
                LOGGER_NOTICE_FMT("Colors r = %d g = %d b = %d", r, g, b);
                _network->pubMsg("outGarden/pool/light/rgb", msg);
            }

            // Rot aus, nur Gelb/Blau
            analogWrite(_LED_red_pin, 255); // OFF (invertiert)
            analogWrite(_LED_green_pin, 255 - g);
            analogWrite(_LED_blue_pin, 255 - b);
        } //-------------------------- handleFadeState --------------------------------//

        // Hilfsfunktion: HSV->RGB (0..255)
        static void hsvToRgb(uint8_t h, uint8_t s, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b)
        {
            const uint8_t region = h / 43; // 0..5
            const uint8_t remainder = (h - (region * 43)) * 6;

            const uint8_t p = (uint16_t)v * (255 - s) / 255;
            const uint8_t q = (uint16_t)v * (255 - ((uint16_t)s * remainder) / 255) / 255;
            const uint8_t t = (uint16_t)v * (255 - ((uint16_t)s * (255 - remainder)) / 255) / 255;

            switch (region)
            {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            default:
                r = v;
                g = p;
                b = q;
                break;
            }
        } //--------------------------- end of hsvToRgb -------------------------//

        void handleColorCycleState()
        {
            const uint32_t now = millis();
            static uint32_t lastPub = 0;

            uint32_t periodMs = _fadePeriodMs;
            if (periodMs < 1000)
                periodMs = 1000;

            // Hue läuft 0..255 über eine Runde
            const uint8_t hue = (uint8_t)((now % periodMs) * 255UL / periodMs);

            uint8_t r, g, b;
            hsvToRgb(hue, 255, 255, r, g, b);

            // r, g, b sind uint8_t (0..255)
            if (now - lastPub >= 500)
            {
                lastPub = now;

                char msg[64];
                snprintf(msg, sizeof(msg),
                         "{\"value\":{\"r\":%u,\"g\":%u,\"b\":%u}}",
                         (unsigned)r, (unsigned)g, (unsigned)b);
                _network->pubMsg("outGarden/pool/light/rgb", msg);
            }

            LOGGER_NOTICE_FMT("CycleColors r = %d g = %d b = %d", r, g, b);

            // invertierte PWM
            analogWrite(_LED_red_pin, 255 - r);
            analogWrite(_LED_green_pin, 255 - g);
            analogWrite(_LED_blue_pin, 255 - b);
        } //--------------------------- handleColorCycleState -------------------------//

        void handleBreathingState()
        /*
        Derzeit nicht aktiv. Wird in einem späteren Upgrade wieder aufgenommen.
        */
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

                // r, g, b sind uint8_t (0..255)
                snprintf(
                    _msg,
                    sizeof(_msg),
                    "{\"value\":{\"r\":%u,\"g\":%u,\"b\":%u}}",
                    _r,
                    _g,
                    _b);
                _network->pubMsg("outGarden/pool/light/rgb", _msg);

                LOGGER_NOTICE_FMT("BreathingColors r = %d g = %d b = %d", red, green, blue);

                analogWrite(_LED_red_pin, 255 - red);
                analogWrite(_LED_green_pin, 255 - green);
                analogWrite(_LED_blue_pin, 255 - blue);
            }
        } //--------------------------- handleBreathingState --------------------------//

        void DebugOutput(const char *msg)
        {
            static uint32_t lastStateLog = 0;
            const uint32_t now = millis();
            if (now - lastStateLog > 1000)
            {
                lastStateLog = now;
                LOGGER_NOTICE_FMT("STATE= %s %d", msg, (uint8_t)_currentState);
            }

        } //--------------------------- DebugOutput -----------------------------------//
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
            target = LightState::COLOR_CYCLE;  // green - blue - green
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
        if (_instance == nullptr)
            return false;

        // Sekunden pro kompletter Runde (grün->blau->grün)
        uint32_t s = (uint32_t)(payload["value"] | 20);

        // Grenzen (optional, aber sinnvoll)
        if (s < 2)
            s = 2;
        if (s > 60)
            s = 60;

        _instance->_fadePeriodMs = s * 1000UL;
        LOGGER_NOTICE_FMT("FadeSpeed set to %u s (%lu ms)", s, (unsigned long)_instance->_fadePeriodMs);

        return true;
    } // ----------------------------- Ambience::FadeSpeed::call ----------------------//

    Ambience *Ambience::_instance = nullptr; // Initialize the static instance pointer

} // End namespace Services
