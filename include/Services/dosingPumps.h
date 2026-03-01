#pragma once
/// @cond
#include <Arduino.h>
#include <ArduinoJson.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include <TaskManager.h>      // hideakitai/TaskManager :contentReference[oaicite:2]{index=2}
#include "../message.h"
#include "../messageBroker.h"
#include "../network.h"

extern Network* _network;
extern MessageBroker msgBroker;

namespace Services {

class dosingPumps : public Task::Base {
protected:
    uint8_t _pump_pin;
    uint8_t _mon_pin;
    String  _topic;
    bool    _state = false;

 //   unsigned long _debounceMs = 200; // ignoriert (wie gewünscht)
    unsigned long _timeoutMs  = 0;   // 0 = disable
    unsigned long _onSince    = 0;   // Zeitpunkt ab dem Pumpe AN ist

private:
    class CmdMsg : public Message {
        dosingPumps& _parent;
    public:
        CmdMsg(dosingPumps& parent, const String& topic)
            : Message(topic), _parent(parent) {}

        bool call(JsonDocument payload) override {
            return _parent.onMessage(payload);
        }
    };

public:
    // WICHTIG: Name muss als erstes rein, weil Tasks.add<T>("name") diesen ctor nutzt. :contentReference[oaicite:3]{index=3}
    dosingPumps(const String& taskName,
               uint8_t pumpPin,
               uint8_t monPin,
               const String& topic,
//               unsigned long /*debounceMs*/ = 200,  // ignoriert
               unsigned long timeoutMs = 0)
        : Task::Base(taskName),
          _pump_pin(pumpPin),
          _mon_pin(monPin),
          _topic(topic),
          _timeoutMs(timeoutMs)
    {
        LOGGER_NOTICE_FMT("Create dosing pump '%s' on pin %d", _topic.c_str(), _pump_pin);

        pinMode(_pump_pin, OUTPUT);
        digitalWrite(_pump_pin, LOW);

        if (_mon_pin != 255) {
            pinMode(_mon_pin, INPUT_PULLUP);
        }

        // Eingehende Commands auf _topic (z.B. "pool/pump/hcl")
        // MessageBroker strippt "inGarden/" vorher weg -> Topic muss ohne Root sein.
        msgBroker.registerMessage(new CmdMsg(*this, _topic));
    }

    virtual ~dosingPumps() = default;

    // hideakitai TaskManager ruft update() zyklisch auf, wenn die Task läuft. :contentReference[oaicite:4]{index=4}
    void update() override {
        tick(millis());
    }

    // ----------------------------------------------------------
    // MQTT handling: erwartet bool (true=ON / false=OFF)
    // ----------------------------------------------------------
    bool onMessage(JsonDocument payload) {
        // _debounceMs wird ignoriert: keine Debounce-Logik

        if (payload.is<bool>()) {
            setState(payload.as<bool>());
            publishState();
            return true;
        }

        LOGGER_WARNING_FMT("%s: Payload not bool", _topic.c_str());
        return false;
    }

    // ----------------------------------------------------------
    // Periodische Logik
    // ----------------------------------------------------------
    virtual void tick(unsigned long now) {
        if (_state && _timeoutMs > 0 && _onSince > 0) {
            if (now - _onSince >= _timeoutMs) {
                LOGGER_NOTICE_FMT("%s: timeout reached (%lums)", _topic.c_str(), _timeoutMs);
                setState(false);
                publishState();
            }
        }
    }

    // ----------------------------------------------------------
    // Switch ON/OFF
    // ----------------------------------------------------------
    virtual void setState(bool on) {
        _onSince = on ? millis() : 0;
        _state = on;

        digitalWrite(_pump_pin, on ? HIGH : LOW);
        LOGGER_NOTICE_FMT("%s %s (pin %d)", _topic.c_str(), on ? "ON" : "OFF", _pump_pin);
    }

protected:
    // ----------------------------------------------------------
    // Publish state via MQTT (outGarden/...)
    // ----------------------------------------------------------
    void publishState() {
        JsonDocument doc;
        doc.set(_state);

        if (_network) {
            // Status nach "<topic>/state" (z.B. "pool/pump/hcl/state")
            _network->pubMsg((_topic + "/state").c_str(), doc);
        }
    }
};

} // namespace Services