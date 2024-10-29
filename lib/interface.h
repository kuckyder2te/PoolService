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

#define HCL_SWT 16  // D2
#define NAOH_SWT 5 // D3
#define WASH_SWT 4 // D5

#define PONT_SWT 0      // D0
#define POOL_LIGHT_SWT 2 // D1

extern char msg[50];
extern PubSubClient client;

void hcl_pump(bool option)
{
    if (option)
    {
        Serial.println("HCl Pump ON");
        digitalWrite(HCL_SWT, LOW);
    }
    else
    {
        Serial.println("HCl Pump OFF");
        digitalWrite(HCL_SWT, HIGH);
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
        digitalWrite(NAOH_SWT, LOW);
    }
    else
    {
        Serial.println("NaOH Pump OFF");
        digitalWrite(NAOH_SWT, HIGH);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    client.publish("outPoolservice/naoh_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void wash_pump(bool option)
{
    if (option)
    {
        Serial.println("Wash pump ON");
        digitalWrite(WASH_SWT, LOW);
    }
    else
    {
        Serial.println("Wash pump OFF");
        digitalWrite(WASH_SWT, HIGH);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    client.publish("outPoolservice/wash_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void pont_pump(bool option)
{
    if (option)
    {
        Serial.println("Pont ON");
        digitalWrite(PONT_SWT, LOW);
    }
    else
    {
        Serial.println("Pont OFF");
        digitalWrite(PONT_SWT, HIGH);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    client.publish("outGarden/pont_pump/state", msg);
} /*--------------------------------------------------------------------------*/

void pool_light(bool option)
{
    if (option)
    {
        Serial.println("Pool Light ON");
        digitalWrite(POOL_LIGHT_SWT, LOW);
    }
    else
    {
        Serial.println("Pool Light OFF");
        digitalWrite(POOL_LIGHT_SWT, HIGH);
    }
    msg[0] = (option ? '1' : '0');
    msg[1] = 0; // String end
    client.publish("outGarden/pool_light/state", msg);
} /*--------------------------------------------------------------------------*/
