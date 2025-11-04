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

class PumpBaseSimple {
protected:
    uint8_t _pump_pin;
    String  _topic;
    bool    _state = false;
    unsigned long _lastCmd = 0;
    unsigned long _debounceMs = 200;
    unsigned long _timeoutMs = 0;
    unsigned long _onSince = 0;

private:
    class StateMsg : public Message {
        PumpBaseSimple &_parent;
    public:
        StateMsg(PumpBaseSimple &parent, String topic)
            : Message(topic), _parent(parent) {}
        bool call(JsonDocument payload) override {
            return _parent.onMessage(payload);
        }
    };

public:
    PumpBaseSimple(uint8_t pin, const String &topic,
                   unsigned long debounceMs = 200,
                   unsigned long timeoutMs = 0)
        : _pump_pin(pin), _topic(topic),
          _debounceMs(debounceMs), _timeoutMs(timeoutMs)
    {
        LOGGER_NOTICE_FMT("Create pump '%s' on Pin: %d", _topic.c_str(), _pump_pin);
        pinMode(_pump_pin, OUTPUT);
        digitalWrite(_pump_pin, LOW);
        msgBroker.registerMessage(new StateMsg(*this, _topic + "/state"));
    }

    virtual ~PumpBaseSimple() = default;

    // ---------------------------
    // Steuerung & MQTT-Kommandos
    // ---------------------------
    bool onMessage(JsonDocument payload) {
        unsigned long now = millis();
        if (now - _lastCmd < _debounceMs) {
            LOGGER_WARNING_FMT("%s: command debounced (%lums)", _topic.c_str(), now - _lastCmd);
            return true;
        }
        _lastCmd = now;

        if (payload.is<bool>()) {
            setState(payload.as<bool>());
            publishState();
            return true;
        }

        if (payload.is<JsonObject>()) {
            JsonObject obj = payload.as<JsonObject>();
            if (obj.containsKey("on"))
                setState(obj["on"].as<bool>());
            if (obj.containsKey("duration")) {
                unsigned long dur = obj["duration"].as<unsigned long>();
                if (dur > 0 && _state)
                    _timeoutMs = dur;
            }
            publishState();
            return true;
        }

        LOGGER_WARNING_FMT("%s: Payload not bool or object", _topic.c_str());
        return false;
    }

    // ---------------------------
    // Hardware-Schalten
    // ---------------------------
    virtual void setState(bool on) {
        if (on == _state) return;
        _state = on;
        digitalWrite(_pump_pin, on ? HIGH : LOW);
        if (on) {
            _onSince = millis();
            LOGGER_NOTICE_FMT("%s ON (pin %d)", _topic.c_str(), _pump_pin);
        } else {
            _onSince = 0;
            LOGGER_NOTICE_FMT("%s OFF (pin %d)", _topic.c_str(), _pump_pin);
        }
    }

    bool getState() const { return _state; }

    void setDebounce(unsigned long ms) { _debounceMs = ms; }
    void setTimeout(unsigned long ms) { _timeoutMs = ms; }

    // ---------------------------
    // Update für Timeout
    // ---------------------------
    virtual void update(unsigned long now = millis()) {
        if (_state && _timeoutMs > 0 && _onSince > 0) {
            if (now - _onSince >= _timeoutMs) {
                LOGGER_NOTICE_FMT("%s: timeout reached (%lums)", _topic.c_str(), _timeoutMs);
                setState(false);
                publishState();
            }
        }
    }

protected:
    void publishState() {
        DynamicJsonDocument doc(64);
        doc.set(_state);
        _network->pubMsg((_topic + "/state").c_str(), doc);
    }
};

} // namespace Services
