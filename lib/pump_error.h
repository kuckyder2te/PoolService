/*
    File name: pH.h
    Date: 2025.07.16
    Author: Wilhelm Kuckelsberg
    Description: pH measurement
*/

#pragma once
#ifndef PUMP_ERROR_H
#define PUMP_ERROR_H

#include <Arduino.h>
#include <TaskManager.h>
#include <PubSubClient.h>
#include "..\lib\def.h"

uint16_t delayTime = 1000; // 10 sec ???

class pumpError : public Task::Base
{
    PubSubClient *_client;
    char msg[30];

public:
    pumpError(const String &name)
        : Task::Base(name)
    {
    }

    pumpError *setClient(PubSubClient *client)
    {
        _client = client;
        return this;
    }

    virtual void begin() override
    {
        Serial.println("Pump errors");
    }

    virtual void update() override
    {
        static unsigned long lastMillis = millis();
        static bool clean_err = true;
        static bool hcl_err = true;
        static bool naoh_err = true;

        if (millis() - lastMillis >= delayTime)
        {
            if (digitalRead(CLEAN_MON) == digitalRead(CLEAN_PUMP))
            {
                if (!clean_err)
                {
                    client.publish("outGarden/clean_error", "true");
                }
                clean_err = true;
            }
            else
            {
                if (clean_err)
                {
                    client.publish("outGarden/clean_error", "false");
                }
                clean_err = false;
            }

            if (digitalRead(HCL_MON) == digitalRead(HCL_PUMP))
            {
                if (!hcl_err)
                {
                    client.publish("outGarden/hcl_error", "true");
                }
                hcl_err = true;
            }
            else
            {
                if (hcl_err)
                {
                    client.publish("outGarden/hcl_error", "false");
                }
                hcl_err = false;
            }

            if (digitalRead(NAOH_MON) == digitalRead(NAOH_PUMP))
            {
                if (!naoh_err)
                {
                    client.publish("outGarden/naoh_error", "true");
                }
                naoh_err = true;
            }
            else
            {
                if (naoh_err)
                {
                    client.publish("outGarden/naoh_error", "false");
                }
                naoh_err = false;
            }

            // bool clear_err = digitalRead(CLEAN_MON);
            // if (clear_err)
            // {
            //   Serial.println("Clear alert");
            // }
            // else
            // {
            //   Serial.println("Clear pump switched off");
            // }

            lastMillis = millis();
        }
    }
};
#endif // PUMP_ERROR_H