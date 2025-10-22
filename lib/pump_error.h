/*
    File name: pumo_error.h
    Date: 2025.07.16
    Author: Wilhelm Kuckelsberg
    Description: pH measurement
*/

#pragma once
#ifndef PUMP_ERROR_H
#define PUMP_ERROR_H

#include <Arduino.h>
#include <TaskManager.h>
#include "../include/network.h"
#include "..\lib\def.h"

extern Network *_network;

class pumpError : public Task::Base
{
    char msg[30];

public:
    pumpError(const String &name)
        : Task::Base(name)
    {
    }
    virtual void begin() override
    {
        Serial.println("Pump errors");
    }

    virtual void update() override
    {
        static bool algizid_err = true;
        static bool hcl_err = true;
        static bool naoh_err = true;

        {
            if (digitalRead(ALGIZID_MON) == digitalRead(ALGIZID_PUMP))
            {
                if (!algizid_err)
                {
                    _network->pubMsg("outGarden/algizid_error", "true");
                }
                algizid_err = true;
            }
            else
            {
                if (algizid_err)
                {
                    _network->pubMsg("outGarden/algizid_error", "false");
                }
                algizid_err = false;
            }

            if (digitalRead(HCL_MON) == digitalRead(HCL_PUMP))
            {
                if (!hcl_err)
                {
                    _network->pubMsg("outGarden/hcl_error", "true");
                }
                hcl_err = true;
            }
            else
            {
                if (hcl_err)
                {
                    _network->pubMsg("outGarden/hcl_error", "false");
                }
                hcl_err = false;
            }

            if (digitalRead(NAOH_MON) == digitalRead(NAOH_PUMP))
            {
                if (!naoh_err)
                {
                    _network->pubMsg("outGarden/naoh_error", "true");
                }
                naoh_err = true;
            }
            else
            {
                if (naoh_err)
                {
                    _network->pubMsg("outGarden/naoh_error", "false");
                }
                naoh_err = false;
            }

        }
    }
};
#endif // PUMP_ERROR_H