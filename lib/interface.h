#pragma once
/*
    File name. interface.h
    Date: 2024.10.06
    Author: Stephan Scholz
    Description: Pool Control
*/

#include <Arduino.h>
// #include <ESP8266WiFi.h>
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

void pool_light(char state)
{
    static bool test = false;
    if (state == 116)
    {
        test = true;
    }
    else
    {
        test = false;
    }

    Serial.printf("Pool Light State: %d", state);
    Serial.println();
    if (test)
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
    msg[0] = (state ? '1' : '0');
    msg[1] = 0; // String end
    client.publish("outPoolservice/pool_light/state", msg);
} /*--------------------------------------------------------------------------*/

void pool_light(uint8_t r, uint8_t g, uint8_t b, char state)
{
    Serial.printf("Pool Light Colours: %d", state);
    Serial.println();

    Serial.printf("Pool Light R:%d, G:%d, B:%d", r, g, b);
    _r = r;
    _g = g;
    _b = b;
    analogWrite(LED_STRIPE_RED, 255 - _r);
    analogWrite(LED_STRIPE_GREEN, 255 - _g);
    analogWrite(LED_STRIPE_BLUE, 255 - _b);
    sprintf(msg, "{ r:%d, g:%d, b:%d }", _r, _g, _b);

    msg[0] = (state ? '1' : '0');
    msg[1] = 0; // String end

    client.publish("outPoolservice/pool_light/state", msg);
} /*--------------------------------------------------------------------------*/

void color_gardient()
{
    uint8_t r = 0;
    uint8_t g;
    uint8_t b;
    static uint8_t step = 0;
    uint16_t gradient_rate = 1000;
    uint16_t gradient_step = 0;

    unsigned long lastMillis = millis();
    if (lastMillis - millis() >= gradient_rate)
    {
        switch (step)
        {
        case 0:
            g = 255;
            b = 0;
            step++;
        case 1:
            g = 230;
            b = 25;
            step++;
        case 2:
            g = 204;
            b = 51;
            step++;
        case 3:
            g = 179;
            b = 76;
            step++;
        case 4:
            g = 153;
            b = 102;
            step++;
        case 5:
            g = 128;
            b = 128;
            step++;
        case 6:
            g = 102;
            b = 153;
            step++;
        case 7:
            g = 76;
            b = 179;
            step++;
        case 8:
            g = 51;
            b = 204;
            step++;
        case 9:
            g = 25;
            b = 230;
            step++;
        case 10:
            g = 0;
            b = 255;
            step++;
        }
    }
}

/*------------------------ end of interface.h------------------------------------*/