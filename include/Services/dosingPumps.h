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

class DosingPumps {
protected:
    uint8_t _pump_pin;
    String _topic;
    bool _state = false;

private:
    class StateMsg : public Message {
        DosingPumps &_parent;
    public:
        StateMsg(DosingPumps &parent, const String &topic)
            : Message(topic), _parent(parent) {}

        bool call(JsonDocument payload) override {
            return _parent.onMessage(payload);
        }
    };

public:
    DosingPumps(uint8_t pumpPin, const String &topic)
        : _pump_pin(pumpPin), _topic(topic)
    {
        LOGGER_NOTICE_FMT("Create dosing pump '%s' on pin %d", _topic.c_str(), _pump_pin);

        pinMode(_pump_pin, OUTPUT);
        digitalWrite(_pump_pin, LOW);

        msgBroker.registerMessage(
            new StateMsg(*this, _topic + "/state")
        );
    }

    virtual ~DosingPumps() = default;

    // ----------------------------------------------------------
    // MQTT-Nachrichten verarbeiten
    // ----------------------------------------------------------
    bool onMessage(JsonDocument payload)
    {
        if (payload.is<bool>()) {
            setState(payload.as<bool>());
            publishState();
            return true;
        }

        LOGGER_WARNING_FMT("%s: Payload not bool", _topic.c_str());
        return false;
    }

    // ----------------------------------------------------------
    // EIN/AUS schalten
    // ----------------------------------------------------------
    virtual void setState(bool on)
    {
        _state = on;
        digitalWrite(_pump_pin, on ? HIGH : LOW);

        LOGGER_NOTICE_FMT("%s %s (pin %d)",
                          _topic.c_str(), on ? "ON" : "OFF", _pump_pin);
    }

    bool getState() const { return _state; }

    // ----------------------------------------------------------
    // State per MQTT publizieren
    // ----------------------------------------------------------
    void publishState()
    {
        DynamicJsonDocument doc(64);
        doc.set(_state);
        _network->pubMsg((_topic + "/state").c_str(), doc);
    }
};

} // end of namespace Services
