#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include <TaskManager.h>
#include "../message.h"
#include "../messageBroker.h"
#include "../../src/def.h"

namespace Services
{

    class Pumps_220V : public Task::Base
    {
    protected:
        uint8_t _pump_pin;
        uint8_t _mon_pin;
        String _topic;
        bool _state = false;

        unsigned long _lastCmd = 0;
        unsigned long _debounceMs = 200; // Standard value
        unsigned long _timeoutMs = 0;    // 0 = disable
        unsigned long _onSince = 0;      // Time from which the pump is ON

        String _topic_prefix; // e.g. "pont_pump"

    private:
        class StateMsg : public Message
        {
            Pumps_220V &_parent;

        public:
            StateMsg(Pumps_220V &parent, const String &topic)
                : Message(topic), _parent(parent) {}

            bool call(JsonDocument payload) override
            {
                return _parent.onMessage(payload);
            }
        };

    public:
        Pumps_220V(const String &taskName,
                  uint8_t pumpPin,
                  uint8_t monPin,
                  const String &topic,
                  unsigned long debounceMs = 200,
                  unsigned long timeoutMs = 0)
            : Task::Base(taskName),
              _pump_pin(pumpPin),
              _mon_pin(monPin),
              _topic(topic),
              _debounceMs(debounceMs),
              _timeoutMs(timeoutMs)
        {
            LOGGER_NOTICE_FMT("Create 220V pump '%s' on pin %d", _topic.c_str(), _pump_pin);
            msgBroker.registerMessage(new StateMsg(*this, _topic + "/state"));
        }

        Pumps_220V* init(uint8_t pumpPin, uint8_t monPin)
        {
            _pump_pin = pumpPin;
            _mon_pin = monPin;
            return this;
        }

        virtual ~Pumps_220V() = default;

        // Initialize the pump pins
        virtual void begin() override
        {
            LOGGER_NOTICE_FMT("Initializing 220V pump '%s' on pin %d", _topic.c_str(), _pump_pin);

            pinMode(_pump_pin, OUTPUT);
            digitalWrite(_pump_pin, LOW);

            if (_mon_pin != 255) // Check if a monitor PIN is implemented.
            {
                pinMode(_mon_pin, INPUT_PULLUP); // default
                digitalWrite(_mon_pin, LOW);     // Is this OK?
            }
        }

        // ----------------------------------------------------------
        // Processing MQTT messages
        // ----------------------------------------------------------
        bool onMessage(JsonDocument payload)
        {
            static unsigned long _lastCmd = millis();

            // Debounce: Ignore MQTT commands arriving too quickly
            if (millis() - _lastCmd <= _debounceMs)
            {
                LOGGER_NOTICE_FMT("%s: command debounced (%lums)", _topic.c_str(), millis() - _lastCmd);
                return true;
            }
            _lastCmd = millis();

            // Accept only boolean
            if (payload.is<bool>())
            {
                setState(payload.as<bool>());
                publishState();
                return true;
            }

            LOGGER_WARNING_FMT("%s: Payload not bool", _topic.c_str());
            return false;
        }

        // Must be called periodically (e.g., in loop()).
        virtual void update() override
        {
            if (_state && _timeoutMs > 0 && _onSince > 0)
            {
                LOGGER_NOTICE_FMT("%s: timeout check", _topic.c_str());
                if (millis() - _onSince >= _timeoutMs)
                {
                    LOGGER_NOTICE_FMT("%s: timeout reached (%lums)", _topic.c_str(), _timeoutMs);

                    setState(false);
                    publishState();
                }
            }
        }

        // ----------------------------------------------------------
        // Switch ON/OFF
        // ----------------------------------------------------------
        virtual void setState(bool on)
        {
            if (on)
            {
                _onSince = millis(); // Save time when turned on
            }
            else
            {
                _onSince = 0; // Reset when turning off
            }
            _state = on;
            digitalWrite(_pump_pin, on ? HIGH : LOW);

            LOGGER_NOTICE_FMT("%s %s (pin %d)", _topic.c_str(), on ? "ON" : "OFF", _pump_pin);
        }

        bool getState() const { return _state; }

    protected:
        // ----------------------------------------------------------
        // Publish state via MQTT
        // ----------------------------------------------------------
        void publishState()
        {
            DynamicJsonDocument doc(64);
            doc.set(_state);
            _network->pubMsg((_topic + "/state").c_str(), doc);
        }
    };

} // end of namespace Services
