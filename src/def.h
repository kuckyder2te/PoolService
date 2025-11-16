/*
    File name: def.h
    Date: 2024.11.14
    Author: Wilhelm Kuckelsberg
    Description: Definitions for PINs and constands
*/

#define GLOBAL_BUFFER_SIZE 100

#define HCL_PUMP     5
#define HCL_MON      39
#define NAOH_PUMP    17
#define NAOH_MON     35
#define ALGIZID_PUMP 32
#define ALGIZID_MON  33

#define POOL_LIGHT   25 

#define LED_STRIPE_GREEN 16
#define LED_STRIPE_BLUE   4
#define LED_STRIPE_RED    2

#define RELAY_1    26  // NC
#define PONT_PUMP  27  // ehem. Relay 2
#define HEAT_PUMP  14  // ehem. Relay 3

#define DALLAS 19

#define PORT_FOR_POOLSERVICE 4000
#define DEBOUNCE_TIME 200

#define TIMEOUT_PONT 31UL * 60UL * 1000UL   // ~ 30 Minuten + 1 minute waiting period
#define TIMEOUT_HEATPUMP 61UL * 60UL * 1000UL   // ~ 60 Minuten + 1 minute waiting period

#define TIMEOUT_HCL_PUMP 16UL * 1000UL  // ~ 15 Seconds + 1 second waiting period
#define TIMEOUT_NAOH_PUMP 16UL * 1000UL  // ~ 15 Seconds + 1 second waiting period
#define TIMEOUT_ALGIZID_PUMP 16UL * 1000UL  // ~ 15 Seconds + 1 second waiting period
