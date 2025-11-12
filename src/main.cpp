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
#include "../include/services/pump_hcl.h"
#include "../include/services/pump_naoh.h"
#include "../include/services/pump_algizid.h"
#include "../include/services/pump_pont.h"
#include "../include/services/pump_heat.h"
#include "../include/services/ambience.h"

#include <ArduinoJson.h>
#include "secrets.h"
/// @endcond

Network *_network;
JsonDocument doc;

HardwareSerial *TestOutput = &Serial;
HardwareSerial *DebugOutput = &Serial;

MessageBroker msgBroker;

/* Dosing pumps */
Services::Pump_naoh *PumpNaOH;
Services::Pump_hcl *PumpHCl;
Services::Pump_algizid *PumpAlgizid;

/* 220V pumps */
Services::Pump_pont *PumpPont;
Services::Pump_heat *PumpHeat;

/* LED lights */
Services::Ambience *LEDLights;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

void setup()
{
  delay(2000);
  DebugOutput->begin(DEBUG_SPEED);
  Logger::setOutputFunction(&MyLoggerOutput::localLogger);
  Logger::setLogLevel(Logger::DEBUG); // Must have a value in platformio.ini(SILENT)
  delay(500);                         // For switching on Serial Monitor
  LOGGER_NOTICE_FMT("************************* Poolservice (%s) *************************", __TIMESTAMP__);
  LOGGER_NOTICE("Start building Poolservice");

  _network = new Network(SID, PW, HOSTNAME, MQTT, MessageBroker::callback);
  _network->begin();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  /* Dosing pumps */
  PumpNaOH = new Services::Pump_naoh(NAOH_PUMP, NAOH_MON, true);
  PumpHCl = new Services::Pump_hcl(HCL_PUMP, HCL_MON, true);
  PumpAlgizid = new Services::Pump_algizid(ALGIZID_PUMP, ALGIZID_MON, true);

  /* 220V pumps */
  PumpPont = new Services::Pump_pont(PONT_PUMP, 200, 10000); // 200ms debounce and 10s timeout
  PumpHeat = new Services::Pump_heat(HEAT_PUMP, 200, 5000);  // 5 s timeout

  /* LED lights */
  LEDLights = new Services::Ambience(LED_STRIPE_RED, LED_STRIPE_GREEN, LED_STRIPE_BLUE);

  Tasks.add<Services::Temperature>("temperature")
      ->init(DALLAS)
      ->startFps(0.017); // ~ 1 minute

  msgBroker.printTopics();
  LOGGER_NOTICE("Finished building Poolservice. Will enter infinite loop");
} /*--------------------------------------------------------------------------*/

void loop()
{
  static unsigned long lastMillis;
  static bool lastState = LOW;

  _network->update();

  Tasks.update();

  PumpNaOH->update();
  PumpHCl->update();
  PumpAlgizid->update();

  PumpPont->update();
  PumpHeat->update();

  if (millis() - lastMillis >= 1000) // This can also be used to test the main loop.
  {
    digitalWrite(LED_BUILTIN, lastState);
    lastState = !lastState;
    lastMillis = millis();
  }

} /*--------------------------------------------------------------------------*/
