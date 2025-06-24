/*
File name: main.cpp
Author:    Stefan Scholz / Wilhel Kuckelsberg
Date:      2024.10.10
Project:   Pool Control

https://github.com/Edistechlab/DIY-Heimautomation-Buch/tree/master/Sensoren/Regensensor
*/

#include <Arduino.h>
#include <TaskManager.h>
// #include <ESP8266WiFi.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include "..\lib\model.h"
#include "..\lib\interface.h"
#include "..\lib\secrets.h"
#include "..\lib\def.h"
#include "..\lib\temperature.h"

// #include <ArduinoOTA.h>

const char *ssid = SID;
const char *password = PW;
const char *mqtt_server = MQTT;

WiFiClient espClient;
PubSubClient client(espClient);
JsonDocument doc;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

} /*--------------------------------------------------------------------------*/

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String topicStr(topic); // macht aus dem Topic ein String -> topicStr

  if (topicStr.indexOf('/') >= 0)
  /*prüft ob die Nachricht ein / enthält was ja den Pfad des Topics aufteilt
  und mindestens eins sollte bei inPump/Egon ja drin sein
  */
  {
    Serial.print("topic = ");
    Serial.println(topic);
    //  The topic includes a '/', we'll try to read the number of bottles from just after that
    topicStr.remove(0, topicStr.indexOf('/') + 1);
    /*
      löscht inPump/ so dass in topicStr nur noch Egon übrig bleibt
    */
    if (topicStr.indexOf('/') >= 0)
    {
      String rootStr = topicStr.substring(0, topicStr.indexOf('/'));
      Serial.println(rootStr);
      if (rootStr == "hcl_pump")
      {
        switch (payload[0])
        {
        case '0': // false
          // Pump off
          hcl_pump(false);
          break;
        case '1':
          // Pump on
          hcl_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "naoh_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          naoh_pump(false);
          break;
        case '1':
          naoh_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "clean_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          clean_pump(false);
          break;
        case '1':
          clean_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "pont_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          pont_pump(false);
          break;
        case '1':
          pont_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "heat_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          heat_pump(false);
          break;
        case '1':
          heat_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "pool_light")
      {
        topicStr.remove(0, topicStr.indexOf('/') + 1); // delete pool_light from topic
        rootStr = topicStr.substring(0, topicStr.indexOf('/'));

        Serial.print("RootState - ");
        Serial.println(rootStr);

        if (rootStr == "state")
        {
          pool_light((char)payload[0]);
          return;
        }
        if (rootStr == "gardient")
        {
          color_gradient((char)payload[0]);
          return;
        }
        if (rootStr == "colors")
        {
          topicStr.remove(0, topicStr.indexOf('/') + 1); // delete colors from topic
          if (topicStr == "rgb")
          { // should be the rest
            DeserializationError error = deserializeJson(doc, payload);

            if (error)
            {
              Serial.print("deserializeJson() failed: ");
              Serial.println(error.c_str());
              return;
            }
            else
            {
              pool_light(doc["r"], doc["g"], doc["b"], (char)payload[0]);
            }
          }
        }
      }
      else
      {
        Serial.println("Unknown topic");
      }
    }
  }
} /*--------------------------------------------------------------------------*/

void setup()
{
  delay(2000);
  Serial.begin(115200);

  pinMode(HCL_PUMP, OUTPUT);
  digitalWrite(HCL_PUMP, LOW);

  pinMode(HCL_MON, INPUT);
  digitalWrite(HCL_MON, LOW);

  pinMode(NAOH_PUMP, OUTPUT);
  digitalWrite(NAOH_PUMP, LOW);

  pinMode(NAOH_MON, INPUT);
  digitalWrite(NAOH_MON, LOW);

  pinMode(CLEAN_PUMP, OUTPUT);
  digitalWrite(CLEAN_PUMP, LOW);

  pinMode(CLEAN_MON, INPUT);
  digitalWrite(CLEAN_MON, LOW);

  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_2, LOW);

  pinMode(RELAY_3, OUTPUT);
  digitalWrite(RELAY_3, LOW);

  // pinMode(RELAY_1, OUTPUT); // not in used
  // digitalWrite(RELAY_1, LOW);

  Serial.println();
  Serial.println("Poolservice inkl. pH, Pool Light, Pool erwärmen und Teichpumpe");
  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);

  setup_wifi();

  // ArduinoOTA.setHostname(ESPHostname);
  // ArduinoOTA.setPassword("admin");
  // ArduinoOTA.begin();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Tasks.add<temperature>("temperature")->startFps(1);

} /*--------------------------------------------------------------------------*/

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outGarden", "Garden control");
      // ... and resubscribe
      client.subscribe("inGarden/#");
      /*
      client.subscribed zu allen Nachrichten wie z.B.
      inPump/Status
      inPump/Egon
      inPump/Willy
*/
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
} /*--------------------------------------------------------------------------*/

void loop()
{
  static bool clean_err = true;
  static bool hcl_err = true;
  static bool naoh_err = true;
  static unsigned long lastMillis = millis();
  uint16_t delayTime = 1000; // 10 sec

  Tasks.update();

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  //   ArduinoOTA.handle();             //Rainsensor
  // long now = millis();
  // if (now - lastMsg > sensorTakt)
  // {
  //   lastMsg = now;
  //   getRainValues(); // Rainsensor
  // }

  if (millis() - lastMillis >= delayTime)
  {
    if (digitalRead(CLEAN_MON) == digitalRead(CLEAN_PUMP))
    {
      if (!clean_err)
      {
        client.publish("outPoolservice/clean_error", "true");
      }
      clean_err = true;
    }
    else
    {
      if (clean_err)
      {
        client.publish("outPoolservice/clean_error", "false");
      }
      clean_err = false;
    }

    if (digitalRead(HCL_MON) == digitalRead(HCL_PUMP))
    {
      if (!hcl_err)
      {
        client.publish("outPoolservice/hcl_error", "true");
      }
      hcl_err = true;
    }
    else
    {
      if (hcl_err)
      {
        client.publish("outPoolservice/hcl_error", "false");
      }
      hcl_err = false;
    }

    if (digitalRead(NAOH_MON) == digitalRead(NAOH_PUMP))
    {
      if (!naoh_err)
      {
        client.publish("outPoolservice/naoh_error", "true");
      }
      naoh_err = true;
    }
    else
    {
      if (naoh_err)
      {
        client.publish("outPoolservice/naoh_error", "false");
      }
      naoh_err = false;
    }

    // bool clear_err = digitalRead(CLEAN_MON);
    // if (clear_err)
    // {
    //   Serial.println("Clear alert");
    // }
    // else
    // {
    //   Serial.println("Clear pump switched off");
    // }

    lastMillis = millis();
  }
  client.publish("outGarden/temperature", String(MODEL.tempC).c_str());
  // client.publish("outGarden/pool_pump/state", String(MODEL.interface.pump_state).c_str());
  // client.publish("outGarden/valve/state", String(MODEL.interface.valve_state).c_str());

} /*--------------------------------------------------------------------------*/
