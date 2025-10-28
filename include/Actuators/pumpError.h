#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include <TaskManager.h>
#include <PubSubClient.h>
#include "../network.h"

extern Network *_network;

namespace Actuators
{
    class pumpError : public Task::Base
    {
        PubSubClient *_client;
        char msg[30];

    public:
        pumpError(const String &name)
            : Task::Base(name)
        {
        }

        // pumpError *init(const uint8_t pin_algizid, const uint8_t pin_hcl, const uint8_t pin_naoh)
        // {
        //     LOGGER_VERBOSE("enter ...");

        //     LOGGER_VERBOSE("leave ...");
        //     return this;
        // }

        virtual void begin() override
        {
            LOGGER_NOTICE("Pump errors");
        }

        virtual void update() override
        {
            static bool algizid_err = false;
            static bool hcl_err = false;
            static bool naoh_err = false;

            if (digitalRead(ALGIZID_MON) == digitalRead(ALGIZID_PUMP))
            {
                algizid_err = false; // No malfunction
                _network->pubMsg("outGarden/algizid_error", "false");
            }
            else
            {
                algizid_err = true; // Malfunktion   must this be?
                _network->pubMsg("outGarden/algizid_error", "true");
            }

            if (digitalRead(NAOH_MON) == digitalRead(NAOH_PUMP))
            {
                naoh_err = false; // No malfunction
                _network->pubMsg("outGarden/naoh_error", "false");
            }
            else
            {
                naoh_err = true; // Malfunktion   must this be?
                _network->pubMsg("outGarden/naoh_error", "true");
            }

            if (digitalRead(HCL_MON) == digitalRead(HCL_PUMP))
            {
                algizid_err = false; // No malfunction
                _network->pubMsg("outGarden/hcl_error", "false");
            }
            else
            {
                algizid_err = true; // Malfunktion   must this be?
                _network->pubMsg("outGarden/hcl_error", "true");
            }

            // if (digitalRead(ALGIZID_MON) == digitalRead(ALGIZID_PUMP))
            // {
            //     if (!algizid_err)
            //     {
            //         _network->pubMsg("outGarden/algizid_error", "true");
            //     }
            //     algizid_err = true;
            // }
            // else
            // {
            //     if (algizid_err)
            //     {
            //         _network->pubMsg("outGarden/algizid_error", "false");
            //     }
            //     algizid_err = false;
            // }  //---------------------------------------------------------------------

            // if (digitalRead(HCL_MON) == digitalRead(HCL_PUMP))
            // {
            //     if (!hcl_err)
            //     {
            //         _network->pubMsg("outGarden/hcl_error", "true");
            //     }
            //     hcl_err = true;
            // }
            // else
            // {
            //     if (hcl_err)
            //     {
            //         _network->pubMsg("outGarden/hcl_error", "false");
            //     }
            //     hcl_err = false;
            // }

            // if (digitalRead(NAOH_MON) == digitalRead(NAOH_PUMP))
            // {
            //     if (!naoh_err)
            //     {
            //         _network->pubMsg("outGarden/naoh_error", "true");
            //     }
            //     naoh_err = true;
            // }
            // else
            // {
            //     if (naoh_err)
            //     {
            //         _network->pubMsg("outGarden/naoh_error", "false");
            //     }
            //     naoh_err = false;
            // }
        }
    };
} // End namespace Actuators