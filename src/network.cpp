/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include "../include/network.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

void Network::onOTAStart()
{
    LOGGER_NOTICE("OTA update started!");
}
void Network::onOTAProgress(size_t current, size_t final)
{
    static unsigned long ota_progress_millis = 0;
    if (millis() - ota_progress_millis > 1000)
    {
        ota_progress_millis = millis();
        LOGGER_NOTICE_FMT("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
    }
}

void Network::onOTAEnd(bool success)
{
    if (success)
    {
        LOGGER_NOTICE("OTA update finished successfully!");
    }
    else
    {
        LOGGER_FATAL("There was an error during OTA update!");
    }
}

void Network::begin()
{
    // LOGGER_NOTICE_FMT("Connecting to %s", _ssid);
    wifi_connect();

    _mqtt_client->setServer(_broker, 1883);
    _mqtt_client->setCallback(callback);

    _web_server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                    {
            String message = "Poolservice (Build: ";
            message += __DATE__; // Kompilierdatum, z. B. "Oct  5 2025"
            message += " ";
            message += __TIME__; // Kompilierzeit, z. B. "14:27:36"
            message += ")";
            request->send(200, "text/plain", message); });

    ElegantOTA.begin(_web_server); // Start ElegantOTA
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    ElegantOTA.setAutoReboot(true);
    _web_server->begin();
}

void Network::begin(char *ip, uint16_t port)
{
    begin();
    _logger_client = new WiFiUDP();
    _logger_port = port;
    _logger_server = ip;
    LOGGER_NOTICE_FMT("Init UDP Logger - Server on %s on port %d", ip, port);
}

void Network::update()
{
    // if WiFi is down, try reconnecting
    if (WiFi.status() != WL_CONNECTED)
    {
        LOGGER_NOTICE("Reconnecting to WiFi...");
        WiFi.disconnect();
        wifi_connect();
    }

    if (!_mqtt_client->connected())
    {
        mqtt_connect();
    }
    _mqtt_client->loop();
    ElegantOTA.loop();
}

bool Network::pubMsg(const char *topic, const char *payload)
{
    LOGGER_NOTICE_FMT("%s - %s", topic, payload);
    return _mqtt_client->publish(topic, payload);
}

bool Network::pubMsg(const char *topic, const JsonDocument payload)
{
    LOGGER_NOTICE_FMT("%s - %s", topic, payload);
    // LOGGER_NOTICE_FMT("%s - %s", topic, String(payload["payload"]).c_str());  // Fehler
    String output;
    serializeJson(payload, output);
    String Topic = ROOT_OUT_TOPIC "/";
    Topic += topic;
    Serial.println(Topic);
    return _mqtt_client->publish(Topic.c_str(), output.c_str());
}

bool Network::mqtt_connect()
{
    LOGGER_NOTICE("Attempting MQTT connection...");
    String clientId = "Poolservice";
    clientId += String(random(0xffff), HEX);

    if (_mqtt_client->connect(clientId.c_str()))
    {
        LOGGER_NOTICE("connected");
        _mqtt_client->publish(ROOT_OUT_TOPIC, "{\"msg\":\"Reconnect: Poolservice\"}");
        LOGGER_NOTICE_FMT("subscribed to: %s", ROOT_IN_TOPIC "/#");
        _mqtt_client->subscribe(ROOT_IN_TOPIC "/#");
        return true;
    }
    else
    {
        LOGGER_NOTICE_FMT("failed, rc= %d", _mqtt_client->state());
        return false;
    }
} /*--------------------------------------------------------------------------*/
void Network::wifi_connect()
{
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(_hostname);
    WiFi.begin(_ssid, _password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    randomSeed(micros());
    // LOGGER_NOTICE_FMT("WiFi connected. IP address: %s", WiFi.localIP().toString().c_str());
}
void Network::sendLoggerMessage(const char *msg)
{
    if (_logger_client != NULL)
    {
        _logger_client->beginPacket(_logger_server, _logger_port);
        _logger_client->println(msg);
        _logger_client->endPacket();
    }
}

void Network::sendLoggerMessage(String msg)
{
    sendLoggerMessage(msg.c_str());
}
void Network::sendLoggerMessage(JsonDocument msg)
{
    String jsonStr;
    serializeJson(msg, jsonStr);
    sendLoggerMessage(jsonStr.c_str());
}