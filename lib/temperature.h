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
#include "..\lib\def.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float tempC;    //später ins Model aufnehmen

#include <TaskManager.h>
#include <PubSubClient.h>

class temperature : public Task::Base
{
    PubSubClient* _client;
    char msg[30];
public:
    temperature(const String &name)
        : Task::Base(name)
    {
    }

    temperature *setClient(PubSubClient *client){
        _client = client;

        return this;
    }

    virtual void begin() override
    {
        Serial.println("Dallas Temperature IC Control Library Demo");
        sensors.begin();
    }

    virtual void update() override
    {
        // call sensors.requestTemperatures() to issue a global temperature
        // request to all devices on the bus
     //   Serial.print("Requesting temperatures...");
        sensors.requestTemperatures(); // Send the command to get temperatures
     //   Serial.println("DONE");
        // After we got the temperatures, we can print them here.
        // We use the function ByIndex, and as an example get the temperature from the first sensor only.
        MODEL.tempC = sensors.getTempCByIndex(0);

        // Check if reading was successful
        if (tempC != DEVICE_DISCONNECTED_C)
        {
            // Serial.print("Temperature for the device 1 (index 0) is: ");
            // Serial.println(MODEL.tempC);

            sprintf(msg, "{ value:%.2f }", MODEL.tempC);

            _client->publish("outPoolservice/temperature", msg);
            Serial.print("temp   ");Serial.println(MODEL.tempC);
     }
        else
        {
            Serial.println("Error: Could not read temperature data");
        }
    }
};
#endif // TEMPERATURE_TASK_H