/*
    File name: pH.h
    Date: 2025.07.16
    Author: Wilhelm Kuckelsberg
    Description: pH measurement
*/

#pragma once
#ifndef PH_TASK_H
#define PH_TASK_H

#include <Arduino.h>
#include <TaskManager.h>
#include <PubSubClient.h>
#include <ph4502c_sensor.h>
#include "..\lib\def.h"

/* Pinout: https://cdn.awsli.com.br/969/969921/arquivos/ph-sensor-ph-4502c.pdf */
#define PH4502C_TEMPERATURE_PIN 34
#define PH4502C_PH_PIN A0
#define PH4502C_PH_TRIGGER_PIN 14

#define PH4502C_CALIBRATION 14.8f
#define PH4502C_READING_INTERVAL 100
#define PH4502C_READING_COUNT 10
// NOTE: The ESP32 ADC has a 12-bit resolution (while most arduinos have 10-bit)
#define ADC_RESOLUTION 4096.0f

// Create an instance of the PH4502C_Sensor
PH4502C_Sensor ph4502c(
    PH4502C_PH_PIN,
    PH4502C_TEMPERATURE_PIN,
    PH4502C_CALIBRATION,
    PH4502C_READING_INTERVAL,
    PH4502C_READING_COUNT,
    ADC_RESOLUTION);

class ph : public Task::Base
{
    PubSubClient *_client;
    char msg[30];

public:
    ph(const String &name)
        : Task::Base(name)
    {
    }

    ph *setClient(PubSubClient *client)
    {
        _client = client;

        return this;
    }

    virtual void begin() override
    {
        Serial.println("Starting PH4502C Sensor...");
        ph4502c.init();
    }

    virtual void update() override
    {
            {
    // Read the temperature from the sensor
    Serial.println("Temperature reading:"
        + String(ph4502c.read_temp()));

    // Read the pH level by average
    Serial.println("pH Level Reading: "
        + String(ph4502c.read_ph_level()));

    // Read a single pH level value
    Serial.println("pH Level Single Reading: "
        + String(ph4502c.read_ph_level_single()));

    //delay(1000);
    }
    }
};
#endif // PH_TASK_H