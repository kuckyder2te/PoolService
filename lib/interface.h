#pragma once
/*
    File name. interface.h
    Date: 2024.10.06
    Author: Stephan Scholz
    Description: Pool Control
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
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

void pool_light(bool option)
{
    if (option)
    {
        Serial.println("Pool Light ON");
        digitalWrite(POOL_LIGHT, HIGH);
    }
    else
    {
        Serial.println("Pool Light OFF");
        digitalWrite(POOL_LIGHT, LOW);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    client.publish("outPoolservice/pool_light/state", msg);
} /*--------------------------------------------------------------------------*/

void setColor(int r, int g, int b){

    Serial.printf("r: %d, g: %d, b: %d\r\n",r,g,b);

    digitalWrite(LED_STRIPE_RED, r);
    digitalWrite(LED_STRIPE_GREEN, g);
    digitalWrite(LED_STRIPE_BLUE, b);

} /*--------------------------------------------------------------------------*/

/*------------------------ end of interface.h------------------------------------*/