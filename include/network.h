#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

#define ROOT_IN_TOPIC "inGarden"
#define ROOT_OUT_TOPIC "outGarden"
#define WEB_SERVER_PORT 80

class Network
{
    static void onOTAStart();

    static void onOTAProgress(size_t current, size_t final);

    static void onOTAEnd(bool success);

private:
    const char *_ssid;
    const char *_password;
    const char *_hostname;
    const char *_broker;
    const char *_logger_server;     ///< Logger Server
    uint16_t _logger_port;          ///< Logger Port

    WiFiClient *_client;
    WiFiUDP *_logger_client;    ///< Socket Client
    PubSubClient *_mqtt_client;
    AsyncWebServer *_web_server;
    MQTT_CALLBACK_SIGNATURE;

public:
    Network(const char *ssid, const char *password, const char *hostname, const char *broker, MQTT_CALLBACK_SIGNATURE) : _ssid(ssid), _password(password), _hostname(hostname), _broker(broker), callback(callback)
    {
        //LOGGER_NOTICE("Create");
        _logger_client = NULL;
        _client = new WiFiClient();
        _mqtt_client = new PubSubClient(*_client);
        _web_server = new AsyncWebServer(WEB_SERVER_PORT);
    };
    void begin();
    void begin(char *ip, uint16_t port);
    void update();

    bool pubMsg(const char *topic, const char *payload);

    bool pubMsg(const char *topic, const JsonDocument payload);

    void sendLoggerMessage(const char *msg);
    void sendLoggerMessage(String msg);
    void sendLoggerMessage(JsonDocument msg);
private:
    bool mqtt_connect();
    void wifi_connect();
};

extern Network *_network; 