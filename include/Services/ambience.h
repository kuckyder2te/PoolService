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
        static Ambience* _instance; // Static pointer to the instance
        
        // State machine states
        enum class LightState {
            OFF,
            ON,
            FADE,
            COLOR_CYCLE,
            BREATHING
        };
        
        LightState _currentState = LightState::OFF;
        LightState _targetState = LightState::OFF;
        
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

    private:
        bool _initialized = false;

    public:
        Ambience(const String &name) : Task::Base(name)
        {
            LOGGER_NOTICE("Create LED stripes");
            _r = 0;
            _g = 0;
            _b = 0;
            _instance = this; // Store the instance pointer
            msgBroker.registerMessage(new State("pool/light/state"));
            msgBroker.registerMessage(new Color("pool/light/colors/rgb"));
        }

        // Method to set the state
        void setState(LightState newState) {
            if (_instance != nullptr) {
                _instance->_currentState = newState;
            }
        }
        // Method to set the color
        void setColor(uint8_t r, uint8_t g, uint8_t b){
            _r = r;
            _g = g;
            _b = b;
        }

        Ambience* init(uint8_t led_red, uint8_t led_green, uint8_t led_blue) 
        {
            _LED_red_pin = led_red;
            _LED_green_pin = led_green;
            _LED_blue_pin = led_blue;
            LOGGER_NOTICE("Create Ambience Task");
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
        }

        virtual void update() override
        {
            // State machine implementation
            switch (_currentState) {
                case LightState::OFF:
                    handleOffState();
                    break;
                case LightState::ON:
                    handleOnState();
                    break;
                case LightState::FADE:
                    handleFadeState();
                    break;
                case LightState::COLOR_CYCLE:
                    handleColorCycleState();
                    break;
                case LightState::BREATHING:
                    handleBreathingState();
                    break;
            }
        }

    private:
        void handleOffState() {
            // Turn off all LEDs
            analogWrite(_LED_red_pin, 255);
            analogWrite(_LED_green_pin, 255);
            analogWrite(_LED_blue_pin, 255);
        }

        void handleOnState() {
            // Keep LEDs at current color
            analogWrite(_LED_red_pin, 255 - _r);
            analogWrite(_LED_green_pin, 255 - _g);
            analogWrite(_LED_blue_pin, 255 - _b);
        }

        void handleFadeState() {
            // Fade between colors
            unsigned long currentTime = millis();
            if (currentTime - _lastUpdate >= 50) { // Update every 50ms
                _lastUpdate = currentTime;
                
                // Simple fade implementation - cycle through colors
                _animationStep = (_animationStep + 1) % 255;
                uint8_t value = _animationStep;
                
                analogWrite(_LED_red_pin, 255 - value);
                analogWrite(_LED_green_pin, 255 - ((value + 85) % 255));
                analogWrite(_LED_blue_pin, 255 - ((value + 170) % 255));
            }
        }

        void handleColorCycleState() {
            // Cycle through predefined colors
            unsigned long currentTime = millis();
            if (currentTime - _lastUpdate >= 1000) { // Change color every second
                _lastUpdate = currentTime;
                
                // Cycle through colors: Red, Green, Blue, Yellow, Purple, Cyan, White
                switch (_animationStep % 7) {
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
        }

        void handleBreathingState() {
            // Breathing effect (fade in and out)
            unsigned long currentTime = millis();
            if (currentTime - _lastUpdate >= 30) { // Update every 30ms
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
        }
    };
    
    bool Ambience::State::call(JsonDocument payload)
    { // Hier die call Implementierung des Prototyps
        LOGGER_NOTICE_FMT("Set State to: %d", (uint8_t)payload["value"]);
        if ((uint8_t)payload["value"])
        {
            _instance->setState(LightState::ON);
        }
        else
        {
            _instance->setState(LightState::OFF);
        }
        return _network->pubMsg("pool_light/state", payload);
    }

    bool Ambience::Color::call(JsonDocument payload)
    {
        LOGGER_NOTICE("Enter");
        _instance->setColor(payload["value"]["r"],payload["value"]["g"],payload["value"]["b"]);
        return _network->pubMsg("pool_light/colors/rgb", payload);
    }

    Ambience* Ambience::_instance = nullptr; // Initialize the static instance pointer

} // End namespace Services
