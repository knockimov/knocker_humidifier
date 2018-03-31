/**************************************************************
*
*    "iSpindel"
*    All rights reserverd by S.Lang <universam@web.de>
*
 **************************************************************/

#ifndef _GLOBALS_H
#define _GLOBALS_H

#pragma once

#include <Arduino.h>
#include <Hash.h>
#include <OLEDFont_Dialog.h>

// defines go here
#define FIRMWAREVERSION "1.1.4"
#define USERAGENT "ESP8266"
#define OLED32
//#define OLED64

#define debounce 50             // ms debounce period to prevent flickering when pressing or releasing the button
#define holdTime 3000           // ms hold period: how long to wait for press+hold event
#define displayOnTime 15000     // Display ontime when button is pressed

#define FANPIN 16
#define BUTTON 14
#define DHTPIN 13
#define DHTTYPE DHT22

#define API_FHEM true
#define API_UBIDOTS true
#define API_GENERIC true
#define API_TCONTROL true
#define API_INFLUXDB true
#define API_PROMETHEUS true

#ifndef DEBUG
#define DEBUG true
#endif

#ifdef NO_CONSOLE
#define CONSOLE(x) \
    do             \
    {              \
    } while (0)
#define CONSOLELN CONSOLE
#define CONSOLEF CONSOLE
#else
#define CONSOLE(x)       \
    do                   \
    {                    \
        Serial.print(x); \
    } while (0)
#define CONSOLELN(x)      \
    do                     \
    {                      \
        Serial.println(x); \
    } while (0)
#endif

#define PORTALTIMEOUT 180

#define CFGFILE "/config.json"
#define TKIDSIZE 40
/*#define ADCDIVISOR 191.8
#define ONE_WIRE_BUS D6 // DS18B20 on ESP pin12
#define RESOLUTION 12   // 12bit resolution == 750ms update rate
#define OWinterval (800 / (1 << (12 - RESOLUTION)))
#define MEDIANROUNDS 7
#define ACCINTERVAL 200
#define MEDIANAVRG 3*/

#define CBP_ENDPOINT "/api/hydrometer/v1/data"

#define DTUbiDots 0
#define DTThingspeak 1
#define DTCraftBeerPi 2
#define DTHTTP 3
#define DTTcontrol 4
#define DTFHEM 5
#define DTTCP 6
#define DTiSPINDELde 7
#define DTInfluxDB 8
#define DTPrometheus 9

#define TEMP_CELSIUS 0
#define TEMP_FAHRENHEIT 1
#define TEMP_KELVIN 2

/*#define WIFIENADDR 1
#define RTCVALIDFLAG 0xCAFEBABE

// sleep management
#define RTCSLEEPADDR 5
#define MAXSLEEPTIME 3600UL //TODO
#define EMERGENCYSLEEP (my_sleeptime * 3 < MAXSLEEPTIME ? MAXSLEEPTIME : my_sleeptime * 3)
#define LOWBATT 3.3*/

//#define UNINIT 0

//extern int16_t ax, ay, az;
//extern int16_t my_aX, my_aY, my_aZ;
//extern float Volt, Temperatur, Tilt, Gravity;
extern float HUMIDITY, TEMPERATURE;

extern bool saveConfig();
extern void formatSpiffs();

#endif