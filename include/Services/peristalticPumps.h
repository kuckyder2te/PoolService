#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "../message.h"
#include "../messageBroker.h"
#include "../../src/def.h"

namespace Services
{

    class PeristalticPumps
    {
    protected:
        uint8_t _pump_pin;
        uint8_t _mon_pin;
        String _topic;
        bool _state = false;
        bool _last_mon_state = HIGH; // For debounce monitoring
        unsigned long _last_mon_change = 0;
        unsigned long _mon_debounce_time = 50; // 50ms debounce for monitoring

        unsigned long _lastCmd = 0;
        unsigned long _debounceMs = 200; // Standard value
        unsigned long _timeoutMs = 0;    // 0 = disable
        unsigned long _onSince = 0;      // Time from which the pump is ON

        String _topic_prefix; // e.g. "hcl_pump"

    private:
        class StateMsg : public Message
        {
            PeristalticPumps &_parent;

        public:
            StateMsg(PeristalticPumps &parent, const String &topic)
                : Message(topic), _parent(parent) {}

            bool call(JsonDocument payload) override
            {
                return _parent.onMessage(payload);
            }
        };

    public:
        PeristalticPumps(uint8_t pumpPin,
                         uint8_t monPin,
                         const String &topic,
                         unsigned long debounceMs = 200,
                         unsigned long timeoutMs = 0)
            : _pump_pin(pumpPin),
              _mon_pin(monPin),
              _topic(topic),
              _debounceMs(debounceMs),
              _timeoutMs(timeoutMs)
        {
            LOGGER_NOTICE_FMT("Create peristaltic pump '%s' on pin %d", _topic.c_str(), _pump_pin);

            pinMode(_pump_pin, OUTPUT);
            digitalWrite(_pump_pin, LOW);

            if (_mon_pin != 255) // Check if a monitor PIN is implemented.
            {
                pinMode(_mon_pin, INPUT_PULLUP); // default
            }

            msgBroker.registerMessage(new StateMsg(*this, _topic + "/state"));
        }

        virtual ~PeristalticPumps() = default;

        // ----------------------------------------------------------
        // Processing MQTT messages
        // ----------------------------------------------------------
        bool onMessage(JsonDocument payload)
        {
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
        virtual void update(unsigned long now = millis())
        {
            // Check for transistor defect via monitoring pin
            checkMonitorPin(now);

            // Handle timeout if applicable
            if (_state && _timeoutMs > 0 && _onSince > 0)
            {
                if (now - _onSince >= _timeoutMs)
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
        // Check monitor pin for defective transistor
        // ----------------------------------------------------------
        void checkMonitorPin(unsigned long now)
        {
            if (_mon_pin == 255)
                return; // No monitor pin configured

            bool current_mon_state = digitalRead(_mon_pin);

            // Debounce the monitor pin reading
            if (current_mon_state != _last_mon_state)
            {
                _last_mon_change = now;
            }
            else if ((now - _last_mon_change) >= _mon_debounce_time)
            {
                // State has been stable for longer than debounce time
                if (current_mon_state == LOW) // LOW means transistor defect detected
                {
                    LOGGER_ERROR_FMT("%s: Defective transistor detected on monitor pin %d", _topic.c_str(), _mon_pin);
                    // Turn off pump if it's on
                    if (_state)
                    {
                        setState(false);
                        publishState();
                    }
                }
            }

            _last_mon_state = current_mon_state;
        }

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