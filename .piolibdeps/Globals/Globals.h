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



// defines go here
#define FIRMWAREVERSION "1.0.2"
#define USERAGENT "ESP8266"

#define DHTPIN 2        // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

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

#define ADCDIVISOR 191.8
#define ONE_WIRE_BUS D6 // DS18B20 on ESP pin12
#define RESOLUTION 12   // 12bit resolution == 750ms update rate
#define OWinterval (800 / (1 << (12 - RESOLUTION)))
#define CFGFILE "/config.json"
#define TKIDSIZE 40
#define MEDIANROUNDS 7
#define ACCINTERVAL 200
#define MEDIANAVRG 3

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

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
//#define DRD_TIMEOUT 1
// RTC Memory Address for the DoubleResetDetector to use
//#define DRD_ADDRESS 0

#define WIFIENADDR 1
#define RTCVALIDFLAG 0xCAFEBABE

// sleep management
#define RTCSLEEPADDR 5
#define MAXSLEEPTIME 3600UL //TODO
#define EMERGENCYSLEEP (my_sleeptime * 3 < MAXSLEEPTIME ? MAXSLEEPTIME : my_sleeptime * 3)
#define LOWBATT 3.3

#define UNINIT 0

extern int16_t ax, ay, az;
extern float Volt, Temperatur, Tilt, Gravity;
extern int16_t my_aX, my_aY, my_aZ;

extern bool saveConfig();
extern void formatSpiffs();

#endif