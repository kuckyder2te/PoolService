#pragma once
/*
    File name. interface.h
    Date: 2024.10.06
    Author: Stephan Scholz
    Description: Pool Control
*/

#include <Arduino.h>
#include <PubSubClient.h>
#include "..\lib\def.h"

namespace interface
{
    typedef struct
    {
        bool pump_state;
        bool valve_state;
    } model_t;
}

extern char msg[50];
extern PubSubClient client;
uint8_t _r, _g, _b;
bool loop_state = false;
bool gradient_up = true;
uint16_t gradient_rate = 100;

void hcl_pump(bool option)
{
    if (option)
    {
        Serial.println("HCl Pump ON");
        digitalWrite(HCL_PUMP, HIGH);
    }
    else
    {
        Serial.println("HCl Pump OFF");
        digitalWrite(HCL_PUMP, LOW);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0;
    client.publish("outPoolservice/hcl_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void naoh_pump(bool option)
{
    if (option)
    {
        Serial.println("NaOH Pump ON");
        digitalWrite(NAOH_PUMP, HIGH);
    }
    else
    {
        Serial.println("NaOH Pump OFF");
        digitalWrite(NAOH_PUMP, LOW);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    client.publish("outPoolservice/naoh_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void clean_pump(bool option)
{
    if (option)
    {
        Serial.println("Wash pump ON");
        digitalWrite(CLEAN_PUMP, HIGH);
    }
    else
    {
        Serial.println("Wash pump OFF");
        digitalWrite(CLEAN_PUMP, LOW);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    client.publish("outPoolservice/clean_pump/state", msg);
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
    client.publish("outPoolservice/pont_pump/state", msg);
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
    client.publish("outPoolservice/heat_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void pool_light(char state) // turns the LED stripes on/off
{
    if (state == '1')
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
    msg[0] = state; //(state'' ? '1' : '0');
    msg[1] = 0;     // String end
    client.publish("outPoolservice/pool_light/state", msg);
} /*--------------------------------------------------------------------------*/

void pool_light(uint8_t r, uint8_t g, uint8_t b, char state = '1') // Choose colors
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

    // msg[0] = (state ? '1' : '0');
    // msg[1] = 0; // String end

    client.publish("outPoolservice/colors/rgb", msg);
    pool_light(state);
} /*--------------------------------------------------------------------------*/

void set_gradient_rate(uint16_t rate)
{
    gradient_rate = rate;
    Serial.printf("\n\rGradient rate: %i", rate);
    sprintf(msg, "{ \"gradiant_value\":%d}", rate);
    client.publish("outPoolservice/pool_light/gradient_rate", msg);
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
    msg[1] = 0; // String end
    // sprintf(msg, "{ \"value\":%s}", (state?"true":"false"));
    client.publish("outPoolservice/pool_light/gradient_state", msg);

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
                _b++;
                _g--;
                Serial.printf("\n\rblue: %i, green: %i\n\r", _b, _g);
                sprintf(msg, "{ r :%d, g :%d, b :%d }", _r, _g, _b);
                // Serial.println(msg);
                // delay(2000);
                client.publish("outPoolservice/pool_lights/rgb", msg);
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
                _b--;
                _g++;
                Serial.printf("n\r\blue: %i, green: %i ", _b, _g);
 //               sprintf(msg, "{ r :%d, g :%d, b :%d }", _r, _g, _b);
            }
            else
            {
                gradient_up = true;
            }
        }
        
 //       client.publish("inGarden/pool_light/colors/rgb", msg);
         pool_light('1');
    }
}
/*------------------------ end of interface.h------------------------------------*/