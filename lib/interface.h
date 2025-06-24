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

void pool_light(char state) // turns the LED stripes on/off
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

void pool_light(uint8_t r, uint8_t g, uint8_t b, char state) // Choose colors
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

bool wait()
{
    uint16_t gradient_rate = 1000;
    static unsigned long lastMillis = 0;

    Serial.println("WAIT");

    if (millis() - lastMillis >= gradient_rate)
    {
        lastMillis = millis();
        return true;
    }
} /*--------------------------------------------------------------------------*/

void color_gradient(char state)
{
    uint8_t r = 0;
    uint8_t g;
    uint8_t b;
    uint16_t gradient_rate = 1000;
    static unsigned long lastMillis = millis();
    static uint8_t steps = 255;
    bool gradient_loop = true;
    uint8_t count = 0; // nur für Testzwecke.

    Serial.println("Gradient");

    while (gradient_loop)
    {
        count++;
        for (int i = 0; i <= steps; i += 1)
        {
            if (wait)
            {
                //   g = 2.55 * (100 - (1 * i));
                g = 255 - i;
                //   b = (2.55 * 100) - (2.55 * (100 - (1 * i)));
                b = i;
                Serial.printf("LED green to blue g:%d, b: %d", g, b);
                Serial.println();
                //               analogWrite(LED_STRIPE_RED, 255 - r);
                analogWrite(LED_STRIPE_GREEN, 255 - g);
                analogWrite(LED_STRIPE_BLUE, 255 - b);
                
            }
        }
        for (int i = steps; i >= 0; i--)
        {
            if (wait)
            {
                //  g = 2.55 * (100 - (1 * i));
                g = 255 - i;
                //  b = (2.55 * 100) - (2.55 * (100 - (1 * i)));
                b = i;
                Serial.printf("LED blue to green g:%d, b: %d", g, b);
                Serial.println();
                //               analogWrite(LED_STRIPE_RED, 255 - r);
                analogWrite(LED_STRIPE_GREEN, 255 - g);
                analogWrite(LED_STRIPE_BLUE, 255 - b);
                
            }
        }
        if (count >= 255)
        {
            gradient_loop = false;
        }
    }

    msg[0] = (state ? '1' : '0');
    msg[1] = 0; // String end

    client.publish("outPoolservice/pool_light/gradient", msg);
}
/*------------------------ end of interface.h------------------------------------*/