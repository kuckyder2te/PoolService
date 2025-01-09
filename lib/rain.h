/*
    File name: temperature.h
    Date: 2025.01.07
    Author: Wilhelm Kuckelsberg
    Description: Temperature measurement with Dallas DS18B20
*/

   #pragma once
#ifndef RAIN_TASK_H
#define RAIN_TASK_H

#include <Arduino.h>
#include <TaskManager.h>
#include "..\lib\def.h"


class rain : public Task::Base {
public:
    rain(const String& name)
    : Task::Base(name) {
    }

    virtual ~rain() {
    }

    // optional (you can remove this method)
    // virtual void begin() override {
    // }

    // optional (you can remove this method)
    // virtual void enter() override {
    // }

    virtual void update() override {


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

#endif  // RAIN_TASK_H