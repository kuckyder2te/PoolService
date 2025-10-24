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
#include "../include/actuators/temperature.h"
#include "../include/actuators/pump_hcl.h"
#include "../include/actuators/pump_naoh.h"
#include "../include/actuators/pump_algizid.h"
#include "../include/actuators/pump_pont.h"
#include "../include/actuators/pump_heat.h"
#include "../include/actuators/valve_rinse.h"
#include "../include/actuators/valve_garden.h"
#include "../include/actuators/led_light.h"
//#include "../include/actuators/valve_terrace.h"    // Fehlermeldung
#include <ArduinoJson.h>
#include "secrets.h"
/// @endcond

Network *_network;
JsonDocument doc;

HardwareSerial *TestOutput = &Serial;
HardwareSerial *DebugOutput = &Serial;

MessageBroker msgBroker;  // Change by Kucky Chat GPT

Actuators::Pump_naoh *PumpNaOH;
Actuators::Pump_hcl *PumpHCl;
Actuators::Pump_algizid *PumpAlgizid;

Actuators::Pump_pont *PumpPont;
Actuators::Pump_heat *PumpHeat;

Actuators::Led_lights *LEDLights;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE]; 

void setup()
{
  delay(2000);
   DebugOutput->begin(DEBUG_SPEED);
  Logger::setOutputFunction(&MyLoggerOutput::localLogger);
  Logger::setLogLevel(Logger::DEBUG); // Muss immer einen Wert in platformio.ini haben (SILENT)
  delay(500);                         // For switching on Serial Monitor
  LOGGER_NOTICE_FMT("************************* Poolservic (%s) *************************", __TIMESTAMP__);
  LOGGER_NOTICE("Start building Poolservice");

  _network = new Network(SID, PW, HOSTNAME, MQTT, MessageBroker::callback);
  _network->begin();
  
  pinMode(NAOH_PUMP, OUTPUT);
  pinMode(NAOH_MON, INPUT);
  pinMode(ALGIZID_PUMP, OUTPUT);
  pinMode(ALGIZID_MON, INPUT);
  pinMode(PONT_PUMP, OUTPUT);
  pinMode(HEAT_PUMP, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  /*Dosing pumps*/
  PumpNaOH = new Actuators::Pump_naoh(NAOH_PUMP, NAOH_MON);
  PumpHCl = new Actuators::Pump_hcl(HCL_PUMP, HCL_MON);
  PumpAlgizid = new Actuators::Pump_algizid(ALGIZID_PUMP, ALGIZID_MON);

  /*220V pumps*/
  PumpPont = new Actuators::Pump_pont(PONT_PUMP);
  PumpHeat = new Actuators::Pump_heat(HEAT_PUMP);

  /* LED lights*/


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
