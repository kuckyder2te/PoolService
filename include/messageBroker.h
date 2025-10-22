#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#define MAX_MESSAGES 50

#include "network.h"
#include <ArduinoJson.h>
#include "message.h"

extern Network *_network;

class MessageBroker
{
private:
    static Message *_messages[MAX_MESSAGES];
    static uint8_t _msgCounter;
    static JsonDocument payload;

protected:
    static PubSubClient *_client;
    static boolean responseMsg(String topic, String payload)
    {
        return _client->publish(topic.c_str(), payload.c_str());
    };

public:
    static void callback(char *topic, byte *payload, unsigned int length)
    {
        LOGGER_NOTICE_FMT("Message arrived [%s]", topic);
        String topicStr(topic);
        topicStr.trim();
        bool found = false;
        if (topicStr.indexOf('/') >= 0)
        {
            topicStr.remove(0, topicStr.indexOf('/') + 1);
            for (uint8_t i = 0; i < _msgCounter; i++)
            {
                if (_messages[i]->isMessage(topicStr))
                {
                    char s[length + 1] = {'\0'};
                    memcpy(s, payload, length);
                    s[length] = 0;
                    LOGGER_NOTICE_FMT("Payload: %s", s);
                    DeserializationError error = deserializeJson(MessageBroker::payload, s);
                    if (error)
                    {
                        LOGGER_WARNING_FMT("%s", error.c_str());
                    }
                    else
                    {
                        if (_messages[i]->call(MessageBroker::payload))
                        {
                            
                        }
                        found = true;
                    }
                    break;
                }
            }
            if (!found)
            {
                LOGGER_WARNING_FMT("Topic %s not supported", topic);
            }
        }
        else
        {
            LOGGER_WARNING("Wrong formed Topic");
        }
    };

    MessageBroker()
    {
        LOGGER_NOTICE("Create");
        _msgCounter = 0;
    };

    Message *registerMessage(Message *msg)
    {
        LOGGER_NOTICE_FMT("Register Message with Topic %s", msg->toString().c_str());
        if (_msgCounter >= MAX_MESSAGES)
        {
            LOGGER_FATAL_FMT("%d Messages already registerd. Increase MAX_MESSAGES value", MAX_MESSAGES);
            while (1)
                ;
        }
        else
        {
            _messages[_msgCounter] = msg;
        }
        return _messages[_msgCounter++];
    }
    void printTopics()
    {
        LOGGER_NOTICE("--------");
        for (uint8_t i = 0; i < _msgCounter; i++)
        {
            LOGGER_NOTICE_FMT("%s", _messages[i]->toString().c_str());
        }
        LOGGER_NOTICE("--------");
    }
};

uint8_t MessageBroker::_msgCounter;
Message *MessageBroker::_messages[MAX_MESSAGES];
JsonDocument MessageBroker::payload;

extern MessageBroker msgBroker;