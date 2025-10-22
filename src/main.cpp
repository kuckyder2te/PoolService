/*
File name: main.cpp
Author:    Stefan Scholz / Wilhelm Kuckelsberg
Date:      2024.10.10
Project:   Pool Service

https://github.com/Edistechlab/DIY-Heimautomation-Buch/tree/master/Sensoren/Regensensor
*/

#include <Arduino.h>
#include <TaskManager.h>
#include "../include/myLogger.h"
#include "../include/network.h"
#include "../include/messageBroker.h"
#include <Wire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
//#include "..\lib\model.h"
#include "..\lib\interface.h"
#include "..\lib\secrets.h"
#include "..\lib\def.h"
#include "..\lib\temperature.h"
#include "..\lib\pump_error.h"

Network *_network;
JsonDocument doc;

HardwareSerial *TestOutput = &Serial;
HardwareSerial *DebugOutput = &Serial;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

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
      else if (rootStr == "algizid_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          algizid_pump(false);
          break;
        case '1':
          algizid_pump(true);
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
          pool_light(((char)payload[0] == '0' ? false : true));
          return;
        }
        if (rootStr == "gradient_state")
        {
          set_gradient_loop_state(((char)payload[0] == '0' ? false : true));
          return;
        }
        if (rootStr == "gradient_rate")
        {
          payload[length] = 0;
          uint16_t temp = (atoi((const char *)payload));
          Serial.printf("\n\rPayload: %i", temp);
          set_gradient_rate(temp);
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
              pool_light(doc["r"], doc["g"], doc["b"]);
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
   DebugOutput->begin(DEBUG_SPEED);
  Logger::setOutputFunction(&MyLoggerOutput::localLogger);
  Logger::setLogLevel(Logger::DEBUG); // Muss immer einen Wert in platformio.ini haben (SILENT)
  delay(500);                         // For switching on Serial Monitor
  LOGGER_NOTICE_FMT("************************* Poolservic (%s) *************************", __TIMESTAMP__);
  LOGGER_NOTICE("Start building Poolservic");

  _network = new Network(SID, PW, HOSTNAME, MQTT, MessageBroker::callback);
  _network->begin();
  
  pinMode(HCL_PUMP, OUTPUT);
  digitalWrite(HCL_PUMP, LOW);

  pinMode(HCL_MON, INPUT);
  digitalWrite(HCL_MON, LOW);

  pinMode(NAOH_PUMP, OUTPUT);
  digitalWrite(NAOH_PUMP, LOW);

  pinMode(NAOH_MON, INPUT);
  digitalWrite(NAOH_MON, LOW);

  pinMode(ALGIZID_PUMP, OUTPUT);
  digitalWrite(ALGIZID_PUMP, LOW);

  pinMode(ALGIZID_MON, INPUT);
  digitalWrite(ALGIZID_MON, LOW);

  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_2, LOW);

  pinMode(RELAY_3, OUTPUT);
  digitalWrite(RELAY_3, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  
  Tasks.add<temperature>("temperature")
      ->startFps(0.017); // ~ 1 minute

  Tasks.add<pumpError>("pumpError")
      ->startFps(1); // ~ 1 second

  pool_light(false);
  set_gradient_rate(500);
  set_gradient_loop_state(false);

  msgBroker.printTopics();
  LOGGER_NOTICE("Finished building Poolservice. Will enter infinite loop");
} /*--------------------------------------------------------------------------*/

void loop()
{
  static bool clean_err = true;
  static bool hcl_err = true;
  static bool naoh_err = true;
  static bool state_led = false;
  static unsigned long lastMillis = millis();
  static unsigned long previousMillis = millis();
  uint16_t delayTime = 1000;

  _network->update();

  Tasks.update();

  if (millis() - lastMillis >= 1000)
  {
    digitalWrite(LED_BUILTIN, state_led);
    state_led = !state_led;
    lastMillis = millis();
  }

  color_gradient_loop();

} /*--------------------------------------------------------------------------*/
