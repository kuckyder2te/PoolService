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
#include "../include/services/pump_peristaltic_hcl.h"
#include "../include/services/pump_peristaltic_naoh.h"
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
  Logger::setOutputFunction(&MyLoggerOutput::localUdpLogger);
  //Logger::setOutputFunction(&MyLoggerOutput::willyUdpLogger);
#endif
  Logger::setLogLevel(Logger::DEBUG); // Muss immer einen Wert in platformio.ini haben (SILENT)
  delay(500);                         // For switching on Serial Monitor
  LOGGER_NOTICE_FMT("************************* Poolservice (%s) *************************", __TIMESTAMP__);
  LOGGER_NOTICE("Start building Poolservice");

  _network = new Network(SID, PW, HOSTNAME, MQTT, MessageBroker::callback);
  //_network->begin("192.168.2.157",PORT_FOR_POOLSERVICE);
  _network->begin(MQTT, PORT_FOR_POOLSERVICE);

  /*220V pumps*/
  PumpPont = new Services::Pump_pont(PONT_PUMP, 255);
  PumpHeat = new Services::Pump_heat(HEAT_PUMP, 255);

  /* LED lights*/
  Tasks.add<Services::Ambience>("ambience")
      ->init(LED_STRIPE_RED, LED_STRIPE_GREEN, LED_STRIPE_BLUE)
      ->startFps(10); // 10 Hz = alle 100ms

  Tasks.add<Services::Temperature>("temperature")
      ->init(DALLAS)
      ->startFps(0.017); // 0.017 ~ 1 minute

  Tasks.add<Services::PH_Placebo>("pH")
      ->init(DALLAS)                    // not used
      ->startFps(10); // ~ 5 minuten

  // Add peristaltic pumps to TaskManager
  Tasks.add<Services::PumpPeristalticHCL>("pump_hcl")
      ->startFps(0.0033); // 10 Hz = alle 100ms

  Tasks.add<Services::PumpPeristalticNAOH>("pump_naoh")
      ->startFps(10); // 10 Hz = alle 100ms

  Tasks.add<Services::PumpPeristalticAlgizid>("pump_algizid")
      ->startFps(10); // 10 Hz = alle 100ms

  msgBroker.printTopics();

  LOGGER_NOTICE("Finished building Poolservice. Will enter infinite loop");
} /*--------------------------------------------------------------------------*/

void loop()
{
  _network->update();
  Tasks.update();
} /*--------------------------------------------------------------------------*/
