/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "Logging/LogCategory.h"

// Log category to use within the RuleRanger plugin
DECLARE_LOG_CATEGORY_EXTERN(RuleRanger, Verbose, All);

#define RR_LOG(Verbosity, Format, ...)                        \
    {                                                         \
        UE_LOG(RuleRanger, Verbosity, Format, ##__VA_ARGS__); \
    }

#define RR_ERROR_LOG(Format, ...) RR_LOG(Error, Format, ##__VA_ARGS__)
#define RR_WARNING_LOG(Format, ...) RR_LOG(Warning, Format, ##__VA_ARGS__)
#define RR_INFO_LOG(Format, ...) RR_LOG(Log, Format, ##__VA_ARGS__)
#define RR_VERBOSE_LOG(Format, ...) RR_LOG(Verbose, Format, ##__VA_ARGS__)
#define RR_VERY_VERBOSE_LOG(Format, ...) RR_LOG(VeryVerbose, Format, ##__VA_ARGS__)

#define RR_ERROR_ALOG(Format, ...) RR_ERROR_LOG(TEXT(Format), ##__VA_ARGS__)
#define RR_WARNING_ALOG(Format, ...) RR_WARNING_LOG(TEXT(Format), ##__VA_ARGS__)
#define RR_INFO_ALOG(Format, ...) RR_INFO_LOG(TEXT(Format), ##__VA_ARGS__)
#define RR_VERBOSE_ALOG(Format, ...) RR_VERBOSE_LOG(TEXT(Format), ##__VA_ARGS__)
#define RR_VERY_VERBOSE_ALOG(Format, ...) RR_VERY_VERBOSE_LOG(TEXT(Format), ##__VA_ARGS__)
