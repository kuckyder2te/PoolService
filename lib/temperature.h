/*
    File name: temperature.h
    Date: 2025.01.07
    Author: Wilhelm Kuckelsberg
    Description: Temperature measurement with Dallas DS18B20
*/

#pragma once
#ifndef TEMPERATURE_TASK_H
#define TEMPERATURE_TASK_H

#include <Arduino.h>
#include <TaskManager.h>
#include "../include/network.h"
#include "..\lib\def.h"

OneWire oneWire(DALLAS);
DallasTemperature sensors(&oneWire);
extern Network *_network;
class temperature : public Task::Base
{
    char msg[30];
    float temperaturePool;
public:
    temperature(const String &name)
        : Task::Base(name)
    {
    }

    virtual void begin() override
    {
        Serial.println("Dallas Temperature IC Control Library Demo");
        sensors.begin();
    }

    virtual void update() override
    {
        sensors.requestTemperatures(); // Send the command to get temperatures
        temperaturePool = sensors.getTempCByIndex(0);

        if (temperaturePool != DEVICE_DISCONNECTED_C)
        {
            sprintf(msg, "{ \"value\":%.1f }", temperaturePool);
            _network->pubMsg("outGarden/temperature", msg);
     }
        else
        {
            Serial.println("Error: Could not read temperature data");
        }
    }
};
#endif // TEMPERATURE_TASK_H