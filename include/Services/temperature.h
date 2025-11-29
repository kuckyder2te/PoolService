#pragma once
/// @cond
#include <Arduino.h>
#define LOCAL_DEBUG
#include "myLogger.h"
/// @endcond

#include <TaskManager.h>
#include "../network.h"
#include <Wire.h>
#include <DallasTemperature.h>

extern Network *_network;

namespace Services
{
    class Temperature : public Task::Base
    {
        OneWire *_interface;
        DallasTemperature *_sensor;
        char _msg[30];
        float _temperaturePool;
        float _temperaturePoolMin;
        float _temperaturePoolMax;

    public:
        Temperature(const String &name) : Task::Base(name)
        {
            LOGGER_NOTICE("Create Temperature Task");
        }
        virtual void begin() override
        {
            LOGGER_NOTICE("Dallas Temperature IC Control Library Demo");
        }
        Temperature *init(const uint8_t DallasPin)
        {
            LOGGER_VERBOSE("enter ...");
            _interface = new OneWire(DallasPin);
            _sensor = new DallasTemperature(_interface);
            _sensor->begin();
            LOGGER_VERBOSE("leave ...");
            return this;
        }

        virtual void update() override
        {
            _temperaturePoolMax = _temperaturePool;
            _temperaturePoolMin = _temperaturePool;

            _sensor->requestTemperatures(); // Send the command to get temperatures
            _temperaturePool = _sensor->getTempCByIndex(0);

            if (_temperaturePool != DEVICE_DISCONNECTED_C)
            {
                sprintf(_msg, "{ \"value\":%.1f }", _temperaturePool);
                _network->pubMsg("outGarden/temperature", _msg);
            }
            else
            {
                LOGGER_ERROR("Error: Could not read temperature data");
            }

            if (_temperaturePool >= _temperaturePoolMax)
            {
                _temperaturePoolMax = _temperaturePool;
                sprintf(_msg, "{ \"value\":%.1f }", _temperaturePoolMax);
                _network->pubMsg("outGarden/temperatureMax", _msg);
                LOGGER_NOTICE_FMT("Tempearture Max: %,1f", _temperaturePoolMax);
            }
            if (_temperaturePool < _temperaturePoolMin)
            {
                _temperaturePoolMin = _temperaturePool;
                sprintf(_msg, "{ \"value\":%.1f }", _temperaturePoolMin);
                _network->pubMsg("outGarden/temperatureMin", _msg);
                LOGGER_NOTICE_FMT("Tempearture Min: %,1f", _temperaturePoolMin);
            }
        };
    };
} // End namespace Services