#pragma once
/// @cond
#include <Arduino.h>
/// @endcond
#include <Logger.h>
#include <stdio.h>
#include <ArduinoJson.h>
#include "network.h"

extern HardwareSerial *DebugOutput;

#if defined LOCAL_DEBUG || defined GLOBAL_DEBUG
// #if (defined LOCAL_DEBUG || defined GLOBAL_DEBUG) && defined DEBUG
extern char logBuf[DEBUG_MESSAGE_BUFFER_SIZE];
#define LOGGER_VERBOSE_FMT(fmt, ...)   \
    sprintf(logBuf, fmt, __VA_ARGS__); \
    LOGGER_VERBOSE(logBuf)
#define LOGGER_NOTICE(msg) Logger::notice(__PRETTY_FUNCTION__, msg)
#define LOGGER_NOTICE_CHK(chk1, chk2, msg)        \
    if (chk1 != chk2)                             \
    {                                             \
        chk2 = chk1;                              \
        Logger::notice(__PRETTY_FUNCTION__, msg); \
    }
#define LOGGER_NOTICE_FMT(fmt, ...)    \
    sprintf(logBuf, fmt, __VA_ARGS__); \
    LOGGER_NOTICE(logBuf)
#define LOGGER_NOTICE_FMT_CHK(chk1, chk2, fmt, ...) \
    if (chk1 != chk2)                               \
    {                                               \
        chk2 = chk1;                                \
        sprintf(logBuf, fmt, __VA_ARGS__);          \
        LOGGER_NOTICE(logBuf);                      \
    }
#define LOGGER_WARNING_FMT(fmt, ...)   \
    sprintf(logBuf, fmt, __VA_ARGS__); \
    LOGGER_WARNING(logBuf)
#define LOGGER_ERROR_FMT(fmt, ...)     \
    sprintf(logBuf, fmt, __VA_ARGS__); \
    LOGGER_ERROR(logBuf)
#define LOGGER_FATAL_FMT(fmt, ...)     \
    sprintf(logBuf, fmt, __VA_ARGS__); \
    LOGGER_FATAL(logBuf)
#define LOGGER_SILENT_FMT(fmt, ...)    \
    sprintf(logBuf, fmt, __VA_ARGS__); \
    LOGGER_SILENT(logBuf)
#define LOGGER_VERBOSE(msg) Logger::verbose(__PRETTY_FUNCTION__, msg)
#define LOGGER_WARNING(msg) Logger::warning(__PRETTY_FUNCTION__, msg)
#define LOGGER_ERROR(msg) Logger::error(__PRETTY_FUNCTION__, msg)
#define LOGGER_FATAL(msg) Logger::fatal(__PRETTY_FUNCTION__, msg)
#define LOGGER_SILENT(msg) Logger::silent(__PRETTY_FUNCTION__, msg)
#else
#define LOGGER_VERBOSE_FMT(...) asm volatile("nop\n\t")
#define LOGGER_NOTICE(...) asm volatile("nop\n\t")
#define LOGGER_NOTICE_CHK(...) asm volatile("nop\n\t")
#define LOGGER_NOTICE_FMT(...) asm volatile("nop\n\t")
#define LOGGER_NOTICE_FMT_CHK(...) asm volatile("nop\n\t")
#define LOGGER_WARNING_FMT(...) asm volatile("nop\n\t")
#define LOGGER_ERROR_FMT(...) asm volatile("nop\n\t")
#define LOGGER_FATAL_FMT(...) asm volatile("nop\n\t")
#define LOGGER_SILENT_FMT(...) asm volatile("nop\n\t")
#define LOGGER_VERBOSE(...) asm volatile("nop\n\t")
#define LOGGER_WARNING(...) asm volatile("nop\n\t")
#define LOGGER_ERROR(...) asm volatile("nop\n\t")
#define LOGGER_FATAL(...) asm volatile("nop\n\t")
// #define LOGGER_FATAL(msg) Logger::fatal(__PRETTY_FUNCTION__, msg)   // should be visible even LOCAL_DEBUG is undefined
#define LOGGER_SILENT(...) asm volatile("nop\n\t")
#endif

class MyLoggerOutput
{
public:
    static void localLogger(Logger::Level level, const char *module, const char *message)
    {
#ifdef LOG_TIMESTAMP
        DebugOutput->print(millis());
        DebugOutput->print(" - ");
#endif
        DebugOutput->print(F("["));
        DebugOutput->print(Logger::asString(level));
        DebugOutput->print(F("]:"));
        if (strlen(module) > 0)
        {
            DebugOutput->print(module);
            DebugOutput->print(":");
        }
        DebugOutput->println(message);
    }

    static void localUdpLogger(Logger::Level level, const char *module, const char *message)
    {
        if (_network != NULL)
        {
            JsonDocument payload;
            String Source = module;
            Source.replace("::", "_");
            Source.replace(" ", "_");
            Source.replace("(", "_");
            Source.replace(")", "_");
            Source.replace("*", "_");
            Source.replace(",", "");
            Source.replace(".", "");
            payload["millis"] = millis();
            payload["level"] = Logger::asString(level);
            payload["source"] = Source;
            payload["message"] = message;
            if (_network != NULL)
            { // If Network isn't available yet
                _network->sendLoggerMessage(payload);
            }
        }
    }

    static void willyUdpLogger(Logger::Level level,const char *module, const char *message)
    {
        if (_network != NULL)
        {
            JsonDocument payload;
            String Source = module;
            Source.replace("::", "_");
            Source.replace(" ", "_");
            Source.replace("(", "_");
            Source.replace(")", "_");
            Source.replace("*", "_");
            Source.replace(",", "");
            Source.replace(".", "");
            // payload["millis"] = millis();
            // payload["level"]=Logger::asString(level);
            payload["message"] = message;
            // payload["source"] = Source;
            if (_network != NULL)
            { // If Network isn't available yet
                _network->sendLoggerMessage(payload);
            }
        }
    }
};

#undef LOCAL_DEBUG