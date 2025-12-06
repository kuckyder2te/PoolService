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

- inGarden/algizid_hclpump/state - Control algizid pump ON/OFF
- inGarden/hcl_pump/state - Control HCL pump ON/OFF
- `inGarden/naoh_pump/state` - Control NaOH pump ON/OFF
- `inGarden/hcl_pump/state` - Control HCl pump ON/OFF
- `inGarden/algezid_pump/state` - Control Algezid pump ON/OFF
- `inGarden/rinse_valve/state` - Control rinse valve for pool
- Protection against uncontrolled pump action. -
- e.g. A pump PIN is HIGH (it means, the pump is off), but the pump is still running.
- `outGarden/algizid_error", "false"`
- `outGarden/naoh_error", "false"`
- `outGarden/hcl_error", "false"`

#### Monitor PINs

| PIN OUT | PIN Monotor | Error | 
|------|--------|---------|
| LOW  |  LOW   | true| 
| HIGH |  LOW   |false| 
| LOW |  HIGH  |false| 
| HIGH |  High  | true| 


- `inGarden/controller/config` - Controller configuration endpoint

### Debounce and Time out rules
- The debouncing is always checked after 200 milliseconds bot only the dosingpumps. The debounce time must be checked.
- The timeout depends on the specific pump. This option applies to all pumps.

### Controller Configuration
The controller now supports dynamic configuration through MQTT messages. The new configuration endpoint allows sending JSON payloads to `inGarden/controller/config` which will be processed and acknowledged with a response on `outGarden/controller/config`.

## Building and Flashing
1. Install PlatformIO
2. Configure your network settings in `src/secrets.h`
3. Build and upload using PlatformIO



