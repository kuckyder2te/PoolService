# Garden Service
Controller for pool and garden

## Overview
This project is an Arduino-based application for the ESP32 that controls various modules including pumps, LEDs, temperatur sensor, and MQTT network communication. It uses PlatformIO as the build system.

## Features
- Chemicals are added using 3 12V peristaltic pumps
- The chemical lines are automatically flushed.
- The pool pump is controlled via a schedule and manual on/off switching
- The pool can be filled.
- RGB LED strips for color gradients and ambience
- Temperature measurement in the pool and ground
- Garden watering
- The pont pump via a schedule and manual on/off switching
- Considered pH measurement in the pool in the pool
- MQTT communication for remote control
- Task-based architecture using TaskManager

## Hardware
![Motherboard](/pictures/Motherboard.png)

## Microcontroller
- ESP32 upesy_wroom

## Connectors
## J1 - 5V supply
| PIN | Signal | 
|------|--------|
| 1 | +12V | 
| 2 | GND -| 
| 3 | +5V -| 

## J2 - aux 12V out
| PIN | Signal | 
|------|--------|
| 1 | +12V | 
| 2 | GND -| 
| 3 | +5V -| 

## J3 - Safety jumper
| PIN | Signal | 
|------|--------|
| 1 |  -| 
| 2 |  -| 

## J4 - 12V In main
| PIN | Signal | 
|------|--------|
| 1 | -J3 -| 
| 2 |+12V IN-| 

## J5 - HCl pump
| PIN | Signal | 
|------|--------|
| 1 | C Q1 | 
| 2 |+12V IN-| 

## J6 - NaOH pump
| PIN | Signal | 
|------|--------|
| 1 | C Q2 | 
| 2 |+12V IN-| 

## J7 - Algizid pump
| PIN | Signal | 
|------|--------|
| 1 | C Q3 | 
| 2 |+12V IN| 

## J8 - not there

## J9 - aux I°C not in use
| PIN | Signal | 
|------|--------|
| 1 | 3V3 | 
| 2 | GND | 
| 3 | SCL | 
| 4 | SDA | 

## J10 - Relays
| PIN | Signal | Color | 
|------|--------|-------|
| 1 | NC  |   | 
| 2 | PONT |  | 
| 3 | HEAT |  | 
| 4 | GND |  | 
| 5 | 5V  |   | 

## J11 - Dallas temperature sensor
| PIN |  Signal   | Color |
|------|--------|--------|
| 1 | 3V3 | 
| 2 | Signal | 
| 3 | GND | 

## J12 - LED stripes
| PIN |  Signal   | Color | 
|------|--------|--------|
| 1 |  GND  |   | 
| 2 | LED red  |   | 
| 3 | LED blue |   | 
| 4 | LED green |   | 
| 5 |  -5V   |    | 


## Valve Relays
| .Actor   |  Color  | 
|------|--------|
|  terrace| rown/blue | 
|  garden | grey/red  | 
| pool füllen|green/black | 
|  +12V  |yellow/white| 



## Software Architecture
The project follows a modular design with the following key components:

- **Task Management**: Using TaskManager for concurrent module operation
- **Network Communication**: MQTT protocol for remote control
- **Message Broker**: Centralized message handling system

### Network Configuration (secrets.h)
```cpp
#define SID "Your_WiFi_SSID"
#define PASSWORD "Your_WiFi_Password"
#define BROKER "MQTT_Broker_IP"
#define USER "MQTT_Username"
#define PW "MQTT_Password"
```

### IP Adress
- Garden 192.168.2.195/update
- Poolservices 192.168.2.211/update

## MQTT Topics
The system uses the following MQTT topics for communication:

- `inGarden/#` - Incoming commands
- `outGarden/#` - Outgoing status updates

### Supported Commands

#### Peristaltic Pumps
- `inGarden/pump/hcl/state` - Control HCl pump ON/OFF
- `inGarden/pump/naoh/state` - Control NaOH pump ON/OFF
- `inGarden/pump/algizid/state` - Control Algizid pump ON/OFF

#### Pont Pump
- `inGarden/pont/pump/state` - Control Pont pump ON/OFF

### Status Updates

#### Peristaltic Pumps
- `outGarden/pump/hcl/state` - HCl pump status
- `outGarden/pump/naoh/state` - NaOH pump status
- `outGarden/pump/algizid/state` - Algizid pump status

#### Pont Pump
- `outGarden/pont/pump/state` - Pont pump status

#### Temperature Monitoring
- `outGarden/current_temp_pool` - Current pool temperature
- `outGarden/pool_temp_min` - Minimum pool temperature
- `outGarden/pool_temp_max` - Maximum pool temperature

#### Error Monitoring
Each pump has automatic error detection through monitoring pins. Errors are reported through the respective pump state topics.

##### Monitor PIN Logic

| Pump PIN | Monitor PIN | Error Status |
|----------|-------------|--------------|
| LOW      | LOW         | true (Error) |
| HIGH     | LOW         | false (OK)   |
| LOW      | HIGH        | false (OK)   |
| HIGH     | HIGH        | true (Error) |

## Building and Flashing
1. Install PlatformIO
2. Configure your network settings in `src/secrets.h`
3. Build and upload using PlatformIO
