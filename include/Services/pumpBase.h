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

class PumpBase {
public:
    enum class ErrorCode : uint8_t {
        NONE = 0,
        STUCK = 1,
        SHORT = 2,
        UNKNOWN = 255
    };

protected:
    uint8_t _pump_pin;
    String  _topic;             // e.g. "hcl_pump"
    bool    _state = false;     // physischer Zustand (HIGH = on)
    unsigned long _lastCmd = 0; // für Debounce
    unsigned long _debounceMs = 200;
    unsigned long _timeoutMs = 0;    // 0 = kein Timeout
    unsigned long _onSince = 0;

    // Monitor (optional)
    uint8_t _monitorPin = 255;       // 255 = kein Monitor
    bool _monitorActiveLow = true;   // true => LOW bedeutet "aktiv"
    unsigned long _monitorFailSince = 0;
    unsigned long _monitorToleranceMs = 500; // Zeit bevor Fehler angenommen wird

private:
    class StateMsg : public Message {
        PumpBase &_parent;
    public:
        StateMsg(PumpBase &parent, String topic) : Message(topic), _parent(parent) {}
        bool call(JsonDocument payload) override { return _parent.onMessage(payload); }
    };

    ErrorCode _error = ErrorCode::NONE;

public:
    PumpBase(uint8_t pump_pin, const String &topic,
             unsigned long debounceMs = 200,
             unsigned long timeoutMs = 0,
             uint8_t monitorPin = 255,
             bool monitorActiveLow = true,
             unsigned long monitorToleranceMs = 500)
        : _pump_pin(pump_pin),
          _topic(topic),
          _debounceMs(debounceMs),
          _timeoutMs(timeoutMs),
          _monitorPin(monitorPin),
          _monitorActiveLow(monitorActiveLow),
          _monitorToleranceMs(monitorToleranceMs)
    {
        LOGGER_NOTICE_FMT("Create pump '%s' on Pin: %d (monitorPin: %d)", _topic.c_str(), _pump_pin, _monitorPin);
        pinMode(_pump_pin, OUTPUT);
        digitalWrite(_pump_pin, LOW);
        if (_monitorPin != 255) {
            pinMode(_monitorPin, INPUT_PULLUP); // default
        }
        msgBroker.registerMessage(new StateMsg(*this, _topic + "/state"));
    }

    virtual ~PumpBase() = default;

    // -----------------------------------------------------------
    // Grundfunktion: Pumpenzustand setzen
    // -----------------------------------------------------------
    virtual void setState(bool on) {
        if (on == _state)
            return;
        _state = on;
        digitalWrite(_pump_pin, on ? HIGH : LOW);
        if (on) {
            _onSince = millis();
            _monitorFailSince = 0;
            LOGGER_NOTICE_FMT("%s ON (pin %d)", _topic.c_str(), _pump_pin);
        } else {
            _onSince = 0;
            LOGGER_NOTICE_FMT("%s OFF (pin %d)", _topic.c_str(), _pump_pin);
        }
    }

    // -----------------------------------------------------------
    // Nachricht vom MessageBroker verarbeiten (MQTT)
    // -----------------------------------------------------------
    bool onMessage(JsonDocument payload) {
        unsigned long now = millis();
        if (now - _lastCmd < _debounceMs) {
            LOGGER_NOTICE_FMT("%s: command debounced (%lums)", _topic.c_str(), now - _lastCmd);
            return true;
        }
        _lastCmd = now;

        // --- einfacher bool-Wert ---
        if (payload.is<bool>()) {
            this->setState(payload.as<bool>());
            publishState();
            return true;
        }

        // --- Objekt mit optionalen Parametern ---
        if (payload.is<JsonObject>()) {
            JsonObject obj = payload.as<JsonObject>();

            if (obj.containsKey("on")) {
                this->setState(obj["on"].as<bool>());
            }

            if (obj.containsKey("duration")) {
                unsigned long dur = obj["duration"].as<unsigned long>();
                if (dur > 0) {
                    bool wantOn = _state;
                    if (obj.containsKey("on"))
                        wantOn = obj["on"].as<bool>();
                    if (wantOn)
                        _timeoutMs = dur;
                }
            }

            publishState();
            return true;
        }

        LOGGER_WARNING_FMT("%s: Payload not bool or object", _topic.c_str());
        return false;
    }

    // -----------------------------------------------------------
    // Getter / Setter
    // -----------------------------------------------------------
    bool getState() const { return _state; }
    void setDebounce(unsigned long ms) { _debounceMs = ms; }
    void setTimeout(unsigned long ms) { _timeoutMs = ms; }
    ErrorCode getError() const { return _error; }

    void setMonitorPin(uint8_t pin, bool activeLow = true, unsigned long toleranceMs = 500) {
        _monitorPin = pin;
        _monitorActiveLow = activeLow;
        _monitorToleranceMs = toleranceMs;
        if (_monitorPin != 255) pinMode(_monitorPin, INPUT_PULLUP);
    }

    // -----------------------------------------------------------
    // Zyklisches Update (Timeout, Monitorüberwachung)
    // -----------------------------------------------------------
    virtual void update(unsigned long now = millis()) {
        // Timeout prüfen
        if (_state && _timeoutMs > 0 && _onSince > 0) {
            if (now - _onSince >= _timeoutMs) {
                LOGGER_NOTICE_FMT("%s: timeout reached (%lums)", _topic.c_str(), _timeoutMs);
                setState(false);
                publishState();
            }
        }

        // Monitor prüfen (nur wenn vorhanden)
        if (_monitorPin != 255 && _state) {
            int val = digitalRead(_monitorPin);
            bool active = _monitorActiveLow ? (val == LOW) : (val == HIGH);

            if (!active) {
                if (_monitorFailSince == 0)
                    _monitorFailSince = now;
                else if (now - _monitorFailSince >= _monitorToleranceMs) {
                    if (_error != ErrorCode::STUCK) {
                        _error = ErrorCode::STUCK;
                        LOGGER_ERROR_FMT("%s: Monitor indicates STUCK (pin %d)", _topic.c_str(), _monitorPin);
                        publishError();
                    }
                }
            } else {
                // Monitor OK -> clear possible STUCK error
                if (_monitorFailSince != 0) _monitorFailSince = 0;
                if (_error == ErrorCode::STUCK) {
                    _error = ErrorCode::NONE;
                    publishError();
                }
            }
        }
    }

protected:
    // -----------------------------------------------------------
    // MQTT-Publishing für State & Error
    // -----------------------------------------------------------
    void publishState() {
        DynamicJsonDocument doc(128);
        doc.set(_state);
        _network->pubMsg((_topic + "/state").c_str(), doc);
    }

    void publishError() {
        DynamicJsonDocument doc(128);
        if (_error == ErrorCode::NONE) {
            doc["error"] = "none";
            doc["code"] = 0;
        } else if (_error == ErrorCode::STUCK) {
            doc["error"] = "stuck";
            doc["code"] = static_cast<int>(_error);
        } else if (_error == ErrorCode::SHORT) {
            doc["error"] = "short";
            doc["code"] = static_cast<int>(_error);
        } else {
            doc["error"] = "unknown";
            doc["code"] = static_cast<int>(_error);
        }
        _network->pubMsg((_topic + "/error").c_str(), doc);
    }
};

} // namespace Services
