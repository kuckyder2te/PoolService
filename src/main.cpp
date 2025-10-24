/*
File name: main.cpp
Author:    Stefan Scholz / Wilhelm Kuckelsberg
Date:      2024.10.10
Project:   Pool Service

https://github.com/Edistechlab/DIY-Heimautomation-Buch/tree/master/Sensoren/Regensensor
*/
/// @cond
#include <Arduino.h>
#include <TaskManager.h>
#include "..\lib\def.h"

#define LOCAL_DEBUG
char logBuf[DEBUG_MESSAGE_BUFFER_SIZE];
#include "../include/myLogger.h"

#include "../include/network.h"
#include "../include/messageBroker.h"
#include "../include/actuators/temperature.h"
#include "../include/actuators/pump_hcl.h"
#include "../include/actuators/pump_naoh.h"
#include "../include/actuators/pump_algizid.h"
#include "../include/actuators/pump_pont.h"
#include "../include/actuators/pump_heat.h"
#include "../include/actuators/valve_rinse.h"
#include "../include/actuators/valve_garden.h"
//#include "../include/actuators/valve_terrace.h"
#include <ArduinoJson.h>
#include "..\lib\interface.h"
#include "..\lib\secrets.h"

#include "..\lib\pump_error.h"
/// @endcond

Network *_network;
JsonDocument doc;

HardwareSerial *TestOutput = &Serial;
HardwareSerial *DebugOutput = &Serial;

//Pump_hcl pump_hcl(HCL_PUMP,HCL_MON);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
/*
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arr  ived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String topicStr(topic); // macht aus dem Topic ein String -> topicStr

  if (topicStr.indexOf('/') >= 0)

  {
    Serial.print("topic = ");
    Serial.println(topic);

    topicStr.remove(0, topicStr.indexOf('/') + 1);
 
    if (topicStr.indexOf('/') >= 0)
    {
      String rootStr = topicStr.substring(0, topicStr.indexOf('/'));
      Serial.println(rootStr);
      if (rootStr == "pool_light")
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
          topicStr.remove(0, topicStr.indexOf('/') + 1);
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
} */
 

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
  
  pinMode(NAOH_PUMP, OUTPUT);
  pinMode(NAOH_MON, INPUT);
  pinMode(ALGIZID_PUMP, OUTPUT);
  pinMode(ALGIZID_MON, INPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Tasks.add<Actuators::Temperature>("temperature")
      ->init(DALLAS)
      ->startFps(0.017); // ~ 1 minute

  msgBroker.printTopics();
  LOGGER_NOTICE("Finished building Poolservice. Will enter infinite loop");
} /*--------------------------------------------------------------------------*/

void loop()
{
  _network->update();

  Tasks.update();

} /*--------------------------------------------------------------------------*/
