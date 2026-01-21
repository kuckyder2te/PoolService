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

    class PeristalticPumps : public Task::Base
    {
    protected:
        uint8_t _pump_pin;
        uint8_t _mon_pin;
        String _topic;
        bool _state = false;
        // bool _last_mon_state = HIGH; // For Transistor monitoring
        // unsigned long _last_mon_change = 0;

        unsigned long _timeoutMs = 0; // 0 = disable
        unsigned long _onSince = 0;   // Time from which the pump is ON

        unsigned long offTimeoutStart = 0;
        const unsigned long OFF_TIMEOUT_MS = 3000; // z. B. 3 Sekunden

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
        PeristalticPumps(const String &taskName)
            : Task::Base(taskName),
              _pump_pin(0),
              _mon_pin(0),
              _topic(""),
              _timeoutMs(0)
        {
        }

        PeristalticPumps *init(uint8_t pumpPin, uint8_t monPin, const String &topic, unsigned long timeoutMs = 0)
        {
            _pump_pin = pumpPin;
            _mon_pin = monPin;
            _topic = topic;
            _timeoutMs = timeoutMs;
            LOGGER_NOTICE_FMT("Create peristaltic pump '%s' on pin %d", _topic.c_str(), _pump_pin);
            pinMode(_pump_pin, OUTPUT);
            digitalWrite(_pump_pin, LOW);

            if (_mon_pin != 255) // Check if a monitor PIN is implemented.
            {
                pinMode(_mon_pin, INPUT_PULLUP); // default
            }
            msgBroker.registerMessage(new StateMsg(*this, _topic + "/state"));
            return this;
        }

        virtual ~PeristalticPumps() = default;

        // ----------------------------------------------------------
        // Processing MQTT messages
        // ----------------------------------------------------------
        bool onMessage(JsonDocument payload)
        {
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
            // Check for transistor defect via monitoring pin
            checkMonitorPin();
            checkOffTimeout();

            // Handle timeout if applicable
            if (_state && _timeoutMs > 0 && _onSince > 0)
            {
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

            // if (on)
            // {
            //     _onSince = millis(); // Save time when turned on
            // }
            // else
            // {
            //     _onSince = 0; // Reset when turning off
            // }
            _state = on;
            digitalWrite(_pump_pin, on ? HIGH : LOW);

            LOGGER_NOTICE_FMT("%s %s (pin %d)", _topic.c_str(), on ? "ON" : "OFF", _pump_pin);
        }

        bool getState() const { return _state; }

    protected:
        // ----------------------------------------------------------
        // Check monitor pin for defective transistor
        // ----------------------------------------------------------
        void checkMonitorPin()
        {
            static uint8_t sameCnt = 0;

            if (_mon_pin == 255) // Muss das sein?
                return;

            // Nur prüfen, wenn die Pumpe laufen SOLL
            if (!_state)
            {
                sameCnt = 0; // wichtig: Counter zurücksetzen
                return;
            }

            // ===== HIER sitzt die Entprell-Logik =====
            bool same = (digitalRead(_mon_pin) == digitalRead(_pump_pin));

            sameCnt = same ? (uint8_t)min(255, sameCnt + 1) : 0; // den teil versteh ich nicht

            if (sameCnt >= 3)
            {
                LOGGER_ERROR_FMT(
                    "%s: Defective transistor detected (pump=%d, monitor=%d)",
                    _topic.c_str(),
                    digitalRead(_pump_pin),
                    digitalRead(_mon_pin));

                setState(false);
                publishState();
                sameCnt = 0;
            }
        }

        // ----------------------------------------------------------
        // Check Timeout and set PIN hard to LOW
        // ----------------------------------------------------------
        void checkOffTimeout()
        {
            // Nur relevant, wenn Pumpe laut Software AUS sein soll
            if (_state)
            {
                offTimeoutStart = 0;
                return;
            }

            // Pin ist trotzdem HIGH → verdächtig
            if (digitalRead(_pump_pin) == HIGH)
            {

                if (offTimeoutStart == 0)
                {
                    offTimeoutStart = millis(); // Start merken
                }

                // Timeout erreicht → NOTAUS
                if (millis() - offTimeoutStart > OFF_TIMEOUT_MS)
                {

                    LOGGER_ERROR_FMT(
                        "%s: EMERGENCY OFF (pin stuck HIGH)",
                        _topic.c_str());

                    // Hart abschalten
                    digitalWrite(_pump_pin, LOW);

                    // internen Zustand korrigieren
                    _state = false;

                    // optional: Fehler melden
                    publishState(); // oder eigenes Error-Publish

                    offTimeoutStart = 0;
                }
            }
            else
            {
                // Pin ist LOW → alles ok
                offTimeoutStart = 0;
            }
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
