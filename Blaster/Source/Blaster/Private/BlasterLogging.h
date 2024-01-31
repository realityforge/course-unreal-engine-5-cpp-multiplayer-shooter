#pragma once

#include "Logging/LogCategory.h"

// Log category to use within the application
DECLARE_LOG_CATEGORY_EXTERN(Blaster, Verbose, All);

FString BlasterGetClientServerContextString(const UObject* Object);

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
#define _BL_ULOG(LogLevel, fmt, ...)                   \
    UE_LOG(Blaster,                                    \
           LogLevel,                                   \
           TEXT("[%s] %s:%i [%s]->[%s] %s"),           \
           *BlasterGetClientServerContextString(this), \
           *FString(__FUNCTION__),                     \
           __LINE__,                                   \
           *GetNameSafe(GetOwner()),                   \
           *GetNameSafe(this),                         \
           *FString::Printf(TEXT(fmt), ##__VA_ARGS__))

/**
 * BL_LOG*
 *
 *   You can use these macro in any code where `this` evaluates to a valid UObject.
 *   The log will print with the __FUNCTION__ and __LINE__ where you write the macro,
 *   and will report `this` identity as [OwnerName]->[ThisName] in the log message.
 *   The message that you pass in (fmt, ...) is appended to the rest of the log info.
 */
#define BL_ULOG(fmt, ...) _BL_ULOG(Log, fmt, ##__VA_ARGS__)
#define BL_ULOG_WARNING(fmt, ...) _BL_ULOG(Warning, fmt, ##__VA_ARGS__)
#define BL_ULOG_ERROR(fmt, ...) _BL_ULOG(Error, fmt, ##__VA_ARGS__)

// TODO: We should make these macros be no-ops when releasing packaged product?

#define BL_ULOG_VERBOSE(fmt, ...) _BL_ULOG(Verbose, fmt, ##__VA_ARGS__)
#define BL_ULOG_VERY_VERBOSE(fmt, ...) _BL_ULOG(VeryVerbose, fmt, ##__VA_ARGS__)
