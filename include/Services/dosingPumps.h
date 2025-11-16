#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "../message.h"
#include "../messageBroker.h"

namespace Services
{

    class DosingPumps
    {
    protected:
        uint8_t _pump_pin;
        uint8_t _mon_pin;
        String _topic;
        bool _state = false;

        unsigned long _lastCmd = 0;
        unsigned long _debounceMs = 200; // Standardwert
        String _topic_prefix; // z.B. "hcl_pump"

    private:
        class StateMsg : public Message
        {
            DosingPumps &_parent;

        public:
            StateMsg(DosingPumps &parent, const String &topic)
                : Message(topic), _parent(parent) {}

            bool call(JsonDocument payload) override
            {
                return _parent.onMessage(payload);
            }
        };

    public:
        DosingPumps(uint8_t pumpPin,
                    uint8_t monPin,
                    const String &topic,
                    unsigned long debounceMs = 200)
            : _pump_pin(pumpPin),
              _mon_pin(monPin),
              _topic(topic),
              _debounceMs(debounceMs)
        {
            LOGGER_NOTICE_FMT("Create dosing pump '%s' on pin %d", _topic.c_str(), _pump_pin);

            pinMode(_pump_pin, OUTPUT);
            digitalWrite(_pump_pin, LOW);

            if (_mon_pin != 255)
            {
                pinMode(_mon_pin, INPUT_PULLUP); // default
            }

            msgBroker.registerMessage(new StateMsg(*this, _topic + "/state"));
        }

        virtual ~DosingPumps() = default;

        // ----------------------------------------------------------
        // Processing MQTT messages
        // ----------------------------------------------------------
        bool onMessage(JsonDocument payload)
        {
            unsigned long now = millis();

            // ⚠️ Debounce: Ignore MQTT commands arriving too quickly
            if (now - _lastCmd < _debounceMs)
            {
                LOGGER_NOTICE_FMT("%s: command debounced (%lums)",
                                  _topic.c_str(), now - _lastCmd);
                return true;
            }
            _lastCmd = now;

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
            // static unsigned long lastDebounce = millis();
            // // Check debounce
            // if (millis() - lastDebounce >= DEBOUNCE_TIME)
            // {
            //     LOGGER_NOTICE_FMT("%s Pump bebounce reached (%lums) - switching off", _topic_prefix.c_str(), (millis() - lastDebounce));
            //     setState(false);
            //     // publish state changed
            //     DynamicJsonDocument doc(128);
            //     doc.set(false);
            //     msgBroker.registerMessage(new StateMsg(*this, "/state"));
            // }
        }

        // ----------------------------------------------------------
        // Switch ON/OFF
        // ----------------------------------------------------------
        virtual void setState(bool on)
        {
            _state = on;
            digitalWrite(_pump_pin, on ? HIGH : LOW);

            LOGGER_NOTICE_FMT("%s %s (pin %d)",
                              _topic.c_str(), on ? "ON" : "OFF", _pump_pin);
        }

        // bool getState() const { return _state; } // Not used anywhere

        // void setDebounce(unsigned long ms) { _debounceMs = ms; } // Not used anywhere

    protected:
        // ----------------------------------------------------------
        // Puplish state wia MQTT
        // ----------------------------------------------------------
        void publishState()
        {
            DynamicJsonDocument doc(64);
            doc.set(_state);
            _network->pubMsg((_topic + "/state").c_str(), doc);
        }
    };

} // end of namespace Services
