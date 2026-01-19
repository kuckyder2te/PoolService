#pragma once
/// @cond
#include <Arduino.h>
//#define LOCAL_DEBUG
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
            static float _pool_tmin = 1000;
            static float _pool_tmax = -1000;

            _sensor->requestTemperatures();
            float _current_pool = _sensor->getTempCByIndex(0);

            if (_current_pool == DEVICE_DISCONNECTED_C || _current_pool == 85.0)
            {
                LOGGER_ERROR("Sensor error!");
                return;
            }

            LOGGER_NOTICE_FMT("Pool current: %.1f min = %.1f max = %.1f\n", _current_pool, _pool_tmin, _pool_tmax);

            if (_current_pool < _pool_tmin)
            {
                _pool_tmin = _current_pool;
                sprintf(_msg, "{ \"value\":%.1f }", _pool_tmin);
                _network->pubMsg("outGarden/pool_temp_min", _msg);
                LOGGER_NOTICE_FMT("temp min: %.1f", _pool_tmin);
            }

            if (_current_pool >= _pool_tmax)
            {
                _pool_tmax = _current_pool;
                sprintf(_msg, "{ \"value\":%.1f }", _pool_tmax);
                _network->pubMsg("outGarden/pool_temp_max", _msg);
                LOGGER_NOTICE_FMT("temp max: %.1f", _pool_tmax);
            }

            sprintf(_msg, "{ \"value\":%.1f }", _current_pool);
            _network->pubMsg("outGarden/current_temp_pool", _msg);
        }
    };
} // End namespace Services