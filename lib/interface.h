#pragma once
/*
    File name. interface.h
    Date: 2024.10.06
    Author: Stephan Scholz / Wilhelm Kuckelsberg
    Description: Pool service
*/

#include <Arduino.h>
#include "../include/network.h"
#include "..\lib\def.h"

extern Network *_network;

namespace interface
{
    typedef struct
    {
        bool pump_state;
        bool valve_state;
    } model_t;
}

extern char msg[50];
uint8_t _r, _g, _b;
bool loop_state = false;
bool gradient_up = true;
uint16_t gradient_rate = 100;

void set_gradient_loop_state(bool);


void algizid_pump(bool option)
{
    if (option)
    {
        Serial.println("Algizid pump ON");
        digitalWrite(ALGIZID_PUMP, HIGH);
    }
    else
    {
        Serial.println("Algizid pump OFF");
        digitalWrite(ALGIZID_PUMP, LOW);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    _network->pubMsg("outGarden/algizid_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void pont_pump(bool option)
{
    if (option)
    {
        Serial.println("Pont ON");
        digitalWrite(RELAY_2, HIGH);
    }
    else
    {
        Serial.println("Pont OFF");
        digitalWrite(RELAY_2, LOW);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    _network->pubMsg("outGarden/pont_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void heat_pump(bool option)
{
    if (option)
    {
        Serial.println("Heat pump ON");
        digitalWrite(RELAY_3, HIGH);
    }
    else
    {
        Serial.println("Heat Pump OFF");
        digitalWrite(RELAY_3, LOW);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    _network->pubMsg("outGarden/heat_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void pool_light(bool state, bool silent = false) // turns the LED stripes on/off
{
    if (state)
    {
        Serial.println("LED stripes ON");
        analogWrite(LED_STRIPE_RED, 255 - _r);
        analogWrite(LED_STRIPE_GREEN, 255 - _g);
        analogWrite(LED_STRIPE_BLUE, 255 - _b);
    }
    else
    {
        Serial.println("LED stripes OFF");
        analogWrite(LED_STRIPE_RED, 255);
        analogWrite(LED_STRIPE_GREEN, 255);
        analogWrite(LED_STRIPE_BLUE, 255);
    }
    msg[0] = (state? '1' : '0');
    msg[1] = 0;
    if(!silent)
        _network->pubMsg("outGarden/pool_light/state", msg);
} /*--------------------------------------------------------------------------*/

void pool_light(uint8_t r, uint8_t g, uint8_t b, bool state = true) // Choose colors
{
    Serial.printf("Pool Light Colours: %d", state);
    Serial.printf("\n\rPool Light R:%d, G:%d, B:%d", r, g, b);
    _r = r;
    _g = g;
    _b = b;
    analogWrite(LED_STRIPE_RED, 255 - _r);
    analogWrite(LED_STRIPE_GREEN, 255 - _g);
    analogWrite(LED_STRIPE_BLUE, 255 - _b);
    sprintf(msg, "{ \"r\":%d, \"g\":%d, \"b\":%d }", _r, _g, _b);

    _network->pubMsg("outGarden/colors/rgb", msg);
    pool_light(state);
} /*--------------------------------------------------------------------------*/

void set_gradient_rate(uint16_t rate)
{
    gradient_rate = rate;
    Serial.printf("\n\rGradient rate: %i", rate);
    sprintf(msg, "{ \"gradiant_value\":%d}", rate);
    _network->pubMsg("outGarden/pool_light/gradient_rate", msg);
} /*--------------------------------------------------------------------------*/

void set_gradient_loop_state(bool state)
{
    loop_state = state;
    if (state)
    {
        _r = 0;
        _g = 255;
        _b = 0;
        gradient_up = true;
    }

    msg[0] = (state ? '1' : '0');
    msg[1] = 0;
    _network->pubMsg("outGarden/pool_light/gradient_state", msg);

    pool_light(state);
} /*--------------------------------------------------------------------------*/

void color_gradient_loop()
{
    static unsigned long lastMillis = millis();

    if (loop_state && (millis() - lastMillis >= gradient_rate))
    {
     //   Serial.println("Gradient loop");
        lastMillis = millis();

        if (gradient_up)
        {
            if (_b < 255)
            {
                _b+=5;
                _g-=5;
                Serial.printf("\n\rblue: %i, green: %i\n\r", _b, _g);
            }
            else
            {
                gradient_up = false;
            }
        }
        else
        { // down
            if (_b > 0)
            {
                _b-=5;
                _g+=5;
                Serial.printf("n\r\blue: %i, green: %i ", _b, _g);
            }
            else
            {
                gradient_up = true;
            }
            
        }
        sprintf(msg, "{ \"r\":%d, \"g\":%d, \"b\":%d }", _r, _g, _b);
        _network->pubMsg("outGarden/colors/rgb", msg);
        pool_light(true,true);
    }
}
/*------------------------ end of interface.h------------------------------------*/