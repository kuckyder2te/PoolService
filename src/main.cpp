/*
File name: main.cpp
Author:    Stefan Scholz / Wilhelm Kuckelsberg
Date:      2024.10.10
Project:   Pool Service

https://github.com/Edistechlab/DIY-Heimautomation-Buch/tree/master/Sensoren/Regensensor
*/

#include <Arduino.h>
#include <TaskManager.h>
// #include <ESP8266WiFi.h>
// #include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
//#include "..\lib\model.h"
#include "..\lib\interface.h"
#include "..\lib\secrets.h"
#include "..\lib\def.h"
#include "..\lib\temperature.h"
#include "..\lib\pump_error.h"

const char *ssid = SID;
const char *password = PW;
const char *mqtt_server = MQTT;

WiFiClient espClient;
PubSubClient client(espClient);
JsonDocument doc;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

unsigned long lastReconnectAttempt = 0;

// ----- OTA begin--------
#include <ElegantOTA.h>

AsyncWebServer server(80);

unsigned long ota_progress_millis = 0;

void onOTAStart()
{
  // Log when OTA has started
  Serial.println("OTA update started!");
}

void onOTAProgress(size_t current, size_t final)
{
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000)
  {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success)
{
  // Log when OTA has finished
  if (success)
  {
    Serial.println("OTA update finished successfully!");
  }
  else
  {
    Serial.println("There was an error during OTA update!");
  }
}
// ----- OTA end --------

void setup_wifi()
{
  delay(10);
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
      else if (rootStr == "algizid_pump")
      {
        switch ((char)payload[0])
        {
        case '0':
          algizid_pump(false);
          break;
        case '1':
          algizid_pump(true);
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
      else if (rootStr == "pool_light")
      {
        topicStr.remove(0, topicStr.indexOf('/') + 1); // delete pool_light from topic
        rootStr = topicStr.substring(0, topicStr.indexOf('/'));

        Serial.print("RootState - ");
        Serial.println(rootStr);

        if (rootStr == "state")
        {
          pool_light(((char)payload[0] == '0' ? false : true));
          return;
        }
        if (rootStr == "gradient_state")
        {
          set_gradient_loop_state(((char)payload[0] == '0' ? false : true));
          return;
        }
        if (rootStr == "gradient_rate")
        {
          payload[length] = 0;
          uint16_t temp = (atoi((const char *)payload));
          Serial.printf("\n\rPayload: %i", temp);
          set_gradient_rate(temp);
          return;
        }

        if (rootStr == "colors")
        {
          topicStr.remove(0, topicStr.indexOf('/') + 1); // delete colors from topic
          if (topicStr == "rgb")
          { // should be the rest
            DeserializationError error = deserializeJson(doc, payload);

            if (error)
            {
              Serial.print("deserializeJson() failed: ");
              Serial.println(error.c_str());
              return;
            }
            else
            {
              pool_light(doc["r"], doc["g"], doc["b"]);
            }
          }
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

  pinMode(HCL_PUMP, OUTPUT);
  digitalWrite(HCL_PUMP, LOW);

  pinMode(HCL_MON, INPUT);
  digitalWrite(HCL_MON, LOW);

  pinMode(NAOH_PUMP, OUTPUT);
  digitalWrite(NAOH_PUMP, LOW);

  pinMode(NAOH_MON, INPUT);
  digitalWrite(NAOH_MON, LOW);

  pinMode(ALGIZID_PUMP, OUTPUT);
  digitalWrite(ALGIZID_PUMP, LOW);

  pinMode(ALGIZID_MON, INPUT);
  digitalWrite(ALGIZID_MON, LOW);

  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_2, LOW);

  pinMode(RELAY_3, OUTPUT);
  digitalWrite(RELAY_3, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println();
  Serial.println("Poolservice inkl. pH, Pool Light, Pool erwärmen und Teichpumpe");
  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Tasks.add<temperature>("temperature")
      ->setClient(&client)
      ->startFps(0.017); // ~ 1 minute

  Tasks.add<pumpError>("pumpError")
      ->setClient(&client)
      ->startFps(1); // ~ 1 second

  // code for current time
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { request->send(200, "text/plain", "Garden-Service"); });

  // code for Build-Date/-time   code von ChatGPT
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String message = "Pool-Service (Build: ";
    message += __DATE__;
    message += " ";
    message += __TIME__;
    message += ")";
    request->send(200, "text/plain", message); });

  ElegantOTA.begin(&server); // Start ElegantOTA
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.setAutoReboot(true);
  ElegantOTA.onEnd(onOTAEnd);
  server.begin();
  Serial.println("HTTP server started");

  pool_light(false);
  set_gradient_rate(500);
  set_gradient_loop_state(false);

} /*--------------------------------------------------------------------------*/

bool reconnect()
{
  Serial.print("Attempting MQTT connection...");
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);

  if (client.connect(clientId.c_str()))
  {
    Serial.println("connected");
    client.publish("outGarden", "Garden control");
    client.subscribe("inGarden/#");
    return true;
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    return false;
  }
} /*--------------------------------------------------------------------------*/

void loop()
{
  static bool clean_err = true;
  static bool hcl_err = true;
  static bool naoh_err = true;
  static bool state_led = false;
  static unsigned long lastMillis = millis();
  static unsigned long previousMillis = millis();
  uint16_t delayTime = 1000;

  ElegantOTA.loop();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >= 30000))
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = millis();
  }

  if (!client.connected())
  {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;
      if (reconnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    client.loop();
  }

  Tasks.update();

  if (millis() - lastMillis >= 1000)
  {
    digitalWrite(LED_BUILTIN, state_led);
    state_led = !state_led;
    lastMillis = millis();
  }

  color_gradient_loop();

} /*--------------------------------------------------------------------------*/
