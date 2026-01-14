#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#define MAX_MESSAGES 50

#include "network.h"
#include <ArduinoJson.h>

extern Network *_network;

class Message
{
private:
    String _topic;
protected:
    const char **CommandStr;        //!< Pointer to Command String array
public:
    Message(const String topic) : _topic(topic)
    {
        LOGGER_VERBOSE("Create");
    };
    bool isMessage(const String topic)
    {
        return (topic == _topic);
    }
    const String toString()
    {
        return _topic;
    }
    virtual bool call(JsonDocument payload) = 0;
};