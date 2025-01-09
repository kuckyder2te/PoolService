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

    OneWire oneWire1(DALLAS_1);
    DS18B20_INT sensorInside1(&oneWire1);

    OneWire oneWire2(DALLAS_2);
    DS18B20_INT sensorInside2(&oneWire2);
 



#include <TaskManager.h>

class temperature : public Task::Base {
public:
    temperature(const String& name)
    : Task::Base(name) {
    }

    virtual ~temperature() {
    }

    // optional (you can remove this method)
    // virtual void begin() override {
    // }

    // optional (you can remove this method)
    // virtual void enter() override {
    // }

    virtual void update() override {

   sensorInside1.requestTemperatures();
    int tempC_1 = sensorInside1.getTempC();
    int tempF_1 = tempC_1 + 32;
    float tempK = tempC_1 + 273.15;
    Serial.print("Temperatur ");
    Serial.print(String(tempC_1));
    Serial.print("°C");
    Serial.print(" | ");
    Serial.print(String(tempF_1));
    Serial.print("°F");
    Serial.print(" | ");
    Serial.print(String(tempK, 2));
    Serial.print("°K");
    Serial.println();

   sensorInside2.requestTemperatures();
    int tempC_2 = sensorInside2.getTempC();
    int tempF_2 = tempC_2 + 32;
     tempK = tempC_2 + 273.15;
    Serial.print("Temperatur ");
    Serial.print(String(tempC_2));
    Serial.print("°C");
    Serial.print(" | ");
    Serial.print(String(tempF_2));
    Serial.print("°F");
    Serial.print(" | ");
    Serial.print(String(tempK, 2));
    Serial.print("°K");
    Serial.println();

    delay(2500);

    }

    // optional (you can remove this method)
    // virtual void exit() override {
    // }

    // optional (you can remove this method)
    // virtual void idle() override {
    // }

    // optional (you can remove this method)
    // virtual void reset() override {
    // }
};

#endif  // TEMPERATURE_TASK_H