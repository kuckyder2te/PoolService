#pragma once

#include "interface.h"

typedef struct 
{
    interface::model_t interface;
    float tempC;
}model_t;

model_t MODEL;

interface::model_t *interface_model = &MODEL.interface;