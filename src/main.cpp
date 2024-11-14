/*
File name: main.cpp
Author:    Stefan Scholz / Wilhel Kuckelsberg
Date:      2024.10.10
Project:   Garden Control

https://github.com/Edistechlab/DIY-Heimautomation-Buch/tree/master/Sensoren/Regensensor
*/

#include <Arduino.h>
#include <TaskManager.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "..\lib\model.h"
#include "..\lib\interface.h"
#include "..\lib\secrets.h"
#include "..\lib\def.h"

// #include <ArduinoOTA.h>   //Rainsensor

const char *ssid = SID;
const char *password = PW;
const char *mqtt_server = MQTT;

// #define ESPHostname "Regensensor"    //Rainsensor

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

const int rainDigitalPin = 12; // Rainsensor
const int rainAnalogPin = A0;
const int rainSensorPowerPin = 14;
const int rainLight = 1000;  // Bei ESP8266 - 980 als Richtwert
const int rainHeavy = 500;   // Bei ESP8266 - 900 als Richtwert
const int sensorTakt = 2000; // alle 2 Sekunden wird der Sensor ausgelesen

#define regenStatus_topic "RegenStatus"
#define regenAnalog_topic "RegenAnalog"
#define regenDigital_topic "RegenDigital"
#define inTopic "Regenensor/intopic"
#define outTopic "Regenensor/outTopic"

int lastRainDigital = 0; // Rainsensor

// if using ESP32
//    int rainAnalog = 4095;
// if using ESP8266
int rainAnalog = 1023;

void getRainValues()
{
  digitalWrite(rainSensorPowerPin, HIGH); // Schaltet den Strom für den Sensor ein
  delay(100);
  int newRainDigital = digitalRead(rainDigitalPin);
  int newRainAnalog = analogRead(rainAnalogPin);

  if (newRainDigital != lastRainDigital)
  {
    if (newRainDigital == HIGH)
    {
      client.publish(regenDigital_topic, "ON");
      Serial.print("Digital Info: Kein Regen\n");
    }
    else
    {
      client.publish(regenDigital_topic, "OFF");
      Serial.print("Digital Info: Regen\n");
    }
    // Kurze Pause zum entprellen
    delay(50);
  }
  lastRainDigital = newRainDigital;

  rainAnalog = newRainAnalog;
  Serial.print("RegenAnalog: ");
  Serial.println(rainAnalog);
  client.publish(regenAnalog_topic, String(rainAnalog).c_str(), true);
  if (rainAnalog <= rainLight)
  {
    if (rainAnalog <= rainHeavy)
    {
      Serial.print("Starker Regen\n");
      client.publish(regenStatus_topic, "Starker Regen");
    }
    else
    {
      Serial.print("Leichter Regen\n");
      client.publish(regenStatus_topic, "Leichter Regen");
    }
  }
  else
  {
    Serial.print("Kein Regen\n");
    client.publish(regenStatus_topic, "Kein Regen");
  }

  digitalWrite(rainSensorPowerPin, LOW); // Schaltet den Strom für den Sensor aus
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

} /*--------------------------------------------------------------------------*/

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String topicStr(topic); // macht aus dem Topic ein String -> topicStr
  if (topicStr.indexOf('/') >= 0)
  /*prüft ob die Nachricht ein / enthält was ja den Pfad des Topics aufteilt
  und mindestens eins sollte bei inPump/Egon ja drin sein
  */
  {
    Serial.print("topic = ");
    Serial.println(topic);
    //  The topic includes a '/', we'll try to read the number of bottles from just after that
    topicStr.remove(0, topicStr.indexOf('/') + 1);
    /*
      löscht inPump/ so dass in topicStr nur noch Egon übrig bleibt
    */
    if (topicStr.indexOf('/') >= 0)
    {
      String rootStr = topicStr.substring(0, topicStr.indexOf('/'));
      Serial.println(rootStr);
      if (rootStr == "hcl_pump")
      {
        switch (payload[0])
        {
        case '0': // false
          // Pump off
          hcl_pump(false);
          break;
        case '1':
          // Pump on
          hcl_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "naoh_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          naoh_pump(false);
          break;
        case '1':
          naoh_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "wash_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          wash_pump(false);
          break;
        case '1':
          wash_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "pont_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          pont_pump(false);
          break;
        case '1':
          pont_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "pool_light")
      {
        switch ((char)payload[0])
        {
        case '0':
          pool_light(false);
          break;
        case '1':
          pool_light(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else if (rootStr == "heat_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          heat_pump(false);
          break;
        case '1':
          heat_pump(true);
          break;
        default:
          // Warning !! Undefined payload or not 1/0
          break;
        }
      }
      else
      {
        Serial.println("Unknown topic");
      }
    }
  }
} /*--------------------------------------------------------------------------*/

void setup()
{
  delay(2000);
  Serial.begin(115200);

  pinMode(HCL_SWT, OUTPUT);
  digitalWrite(HCL_SWT, LOW);

  pinMode(HCL_ERR, INPUT);
  digitalWrite(HCL_SWT, LOW);

  pinMode(NAOH_SWT, OUTPUT);
  digitalWrite(NAOH_SWT, LOW);

  pinMode(NAOH_ERR, INPUT);
  digitalWrite(NAOH_ERR, LOW); 

  pinMode(CLEAN_SWT, OUTPUT);
  digitalWrite(CLEAN_SWT, LOW);

  pinMode(CLEAN_ERR, INPUT);
  digitalWrite(CLEAN_ERR, LOW); 

  pinMode(PONT_SWT, OUTPUT);
  digitalWrite(PONT_SWT, LOW);

  pinMode(POOL_LIGHT_SWT, OUTPUT);
  digitalWrite(POOL_LIGHT_SWT, LOW);

  pinMode(HEAT_PUMP_SWT, OUTPUT);
  digitalWrite(HEAT_PUMP_SWT, LOW);

  pinMode(rainDigitalPin, INPUT); // Rainsensor
  pinMode(rainAnalogPin, INPUT);
  pinMode(rainSensorPowerPin, OUTPUT);

  Serial.println();
  Serial.println("Poolservice inkl. pH, Pool Light, Pool erwärmen und Teichpumpe");
  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);

  setup_wifi();

  // ArduinoOTA.setHostname(ESPHostname);   //Rainsensor
  //  ArduinoOTA.setPassword("admin");
  // ArduinoOTA.begin();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

} /*--------------------------------------------------------------------------*/

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outGarden", "Garden control");
      // ... and resubscribe
      client.subscribe("inGarden/#");
      /*
      client.subscribed zu allen Nachrichten wie z.B.
      inPump/Status
      inPump/Egon
      inPump/Willy
*/
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
} /*--------------------------------------------------------------------------*/

void loop()
{
  static unsigned long lastMillis = millis();
  uint16_t delayTime = 10000; // 10 sec

  // Tasks.update();

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  //   ArduinoOTA.handle();             //Rainsensor
  long now = millis();
  if (now - lastMsg > sensorTakt)
  {
    lastMsg = now;
    getRainValues(); // Rainsensor
  }

} /*--------------------------------------------------------------------------*/
