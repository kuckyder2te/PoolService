#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "../message.h"
#include "../messageBroker.h"

namespace Services {

class PumpBaseDosDebounce {
protected:
    uint8_t _pump_pin;
    String _topic;
    bool   _state = false;

    unsigned long _lastCmd = 0;
    unsigned long _debounceMs = 200;   // Standardwert

private:
    class StateMsg : public Message {
        PumpBaseDosDebounce &_parent;
    public:
        StateMsg(PumpBaseDosDebounce &parent, String topic)
            : Message(topic), _parent(parent) {}

        bool call(JsonDocument payload) override {
            return _parent.onMessage(payload);
        }
    };

public:
    PumpBaseDosDebounce(uint8_t pumpPin,
                        const String &topic,
                        unsigned long debounceMs = 200)
        : _pump_pin(pumpPin),
          _topic(topic),
          _debounceMs(debounceMs)
    {
        LOGGER_NOTICE_FMT("Create dosing pump '%s' on pin %d",
                          _topic.c_str(), _pump_pin);

        pinMode(_pump_pin, OUTPUT);
        digitalWrite(_pump_pin, LOW);

        msgBroker.registerMessage(
            new StateMsg(*this, _topic + "/state")
        );
    }

    virtual ~PumpBaseDosDebounce() = default;

    // ----------------------------------------------------------
    // MQTT-Steuerung
    // ----------------------------------------------------------
    bool onMessage(JsonDocument payload)
    {
        unsigned long now = millis();

        // ⚠️ Debounce: zu schnell ankommende MQTT-Commands ignorieren
        if (now - _lastCmd < _debounceMs) {
            LOGGER_NOTICE_FMT("%s: command debounced (%lums)",
                             _topic.c_str(), now - _lastCmd);
            return true; 
        }
        _lastCmd = now;

        // Nur bool akzeptieren
        if (payload.is<bool>()) {
            setState(payload.as<bool>());
            publishState();
            return true;
        }

        LOGGER_WARNING_FMT("%s: Payload not bool", _topic.c_str());
        return false;
    }

    // ----------------------------------------------------------
    // Hardware EIN/AUS
    // ----------------------------------------------------------
    virtual void setState(bool on)
    {
        _state = on;
        digitalWrite(_pump_pin, on ? HIGH : LOW);

        LOGGER_NOTICE_FMT("%s %s (pin %d)",
                          _topic.c_str(),
                          on ? "ON" : "OFF",
                          _pump_pin);
    }

    bool getState() const { return _state; }

    void setDebounce(unsigned long ms) { _debounceMs = ms; }

protected:
    // ----------------------------------------------------------
    // MQTT-Response
    // ----------------------------------------------------------
    void publishState()
    {
        DynamicJsonDocument doc(64);
        doc.set(_state);
        _network->pubMsg((_topic + "/state").c_str(), doc);
    }
};

} // namespace Services
