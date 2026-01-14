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
#include "def.h"

#define LOCAL_DEBUG
char logBuf[DEBUG_MESSAGE_BUFFER_SIZE];
#include "../include/myLogger.h"

#include "../include/network.h"
#include "../include/messageBroker.h"
#include "../include/services/temperature.h"
// #include "../include/services/pump_hcl.h"
#include "../include/services/pump_peristaltic_hcl.h"
// #include "../include/services/pump_naoh.h"
#include "../include/services/pump_peristaltic_naoh.h"
// #include "../include/services/pump_algizid.h"
#include "../include/services/pump_peristaltic_algizid.h"
#include "../include/services/pump_pont.h"
#include "../include/services/pump_heat.h"
#include "../include/services/ambience.h"
#include "../include/services/ph_placebo.h"

#include <ArduinoJson.h>
#include "secrets.h"
/// @endcond

Network *_network;
JsonDocument doc;

HardwareSerial *TestOutput = &Serial;
HardwareSerial *DebugOutput = &Serial;

MessageBroker msgBroker;

// Services::Pump_naoh *PumpNaOH;
// Services::Pump_hcl *PumpHCl;
// Services::Pump_algizid *PumpAlgizid;

Services::PumpPeristalticAlgizid *PumpPeristalticAlgizid;
Services::PumpPeristalticNAOH *PumpPeristalticNAOH;
Services::PumpPeristalticHCL *PumpPeristalticHCL;


Services::Pump_pont *PumpPont;
Services::Pump_heat *PumpHeat;

Services::Ambience *LEDLights;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setup()
{
  delay(2000);
  Serial.begin(DEBUG_SPEED);
#ifdef DEBUG_DESTINATION_SERIAL
  DebugOutput->begin(DEBUG_SPEED);
  Logger::setOutputFunction(&MyLoggerOutput::localLogger);
#endif
#ifdef DEBUG_DESTINATION_UDP
 // Logger::setOutputFunction(&MyLoggerOutput::localUdpLogger);
  Logger::setOutputFunction(&MyLoggerOutput::willyUdpLogger);
#endif
  Logger::setLogLevel(Logger::DEBUG); // Muss immer einen Wert in platformio.ini haben (SILENT)
  delay(500);                         // For switching on Serial Monitor
  LOGGER_NOTICE_FMT("************************* Poolservice (%s) *************************", __TIMESTAMP__);
  LOGGER_NOTICE("Start building Poolservice");

  _network = new Network(SID, PW, HOSTNAME, MQTT, MessageBroker::callback);
  //_network->begin("192.168.2.157",PORT_FOR_POOLSERVICE);
  _network->begin(MQTT, PORT_FOR_POOLSERVICE);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  /*Dosing pumps*/
  // PumpNaOH = new Services::Pump_naoh(NAOH_PUMP, NAOH_MON);
  PumpPeristalticNAOH = new Services::PumpPeristalticNAOH();

  // PumpHCl = new Services::Pump_hcl(HCL_PUMP, HCL_MON);
PumpPeristalticHCL = new Services::PumpPeristalticHCL();

  // PumpAlgizid = new Services::Pump_algizid(ALGIZID_PUMP, ALGIZID_MON);
PumpPeristalticAlgizid = new Services::PumpPeristalticAlgizid();


  /*220V pumps*/
  PumpPont = new Services::Pump_pont(PONT_PUMP, 255);
  PumpHeat = new Services::Pump_heat(HEAT_PUMP, 255);

  /* LED lights*/
  //LEDLights = new Services::Ambience();
  LEDLights = new Services::Ambience(LED_STRIPE_RED, LED_STRIPE_GREEN, LED_STRIPE_BLUE);

  Tasks.add<Services::Temperature>("temperature")
      ->init(DALLAS)
      ->startFps(0.017); //0.017 ~ 1 minute

        Tasks.add<Services::PH_Placebo>("pH")
      ->init(DALLAS)
      ->startFps(0.0033); // ~ 5 minuten


  msgBroker.printTopics();
  LOGGER_NOTICE("Finished building Poolservice. Will enter infinite loop");
} /*--------------------------------------------------------------------------*/

void loop()
{
  static unsigned long lastMillis;
  static bool lastState = LOW;

  _network->update();

  Tasks.update();

  if (millis() - lastMillis >= 1000) // This can also be used to test the main loop.
  {
    digitalWrite(LED_BUILTIN, lastState);
    lastState = !lastState;

      // PumpHCl->update();
      // PumpNaOH->update();
      // PumpAlgizid->update();

      PumpPeristalticAlgizid->update();
      PumpPeristalticHCL->update();
      PumpPeristalticNAOH->update();

      //LEDLights->update();

    lastMillis = millis();
  }

} /*--------------------------------------------------------------------------*/
