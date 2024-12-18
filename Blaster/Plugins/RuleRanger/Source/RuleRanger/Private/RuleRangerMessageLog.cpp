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
#include "RuleRangerMessageLog.h"
#include "MessageLogModule.h"
#include "RuleRangerLogging.h"

const FName FRuleRangerMessageLog::MessageLogName{ TEXT("RuleRanger") };
static const FName MessageLogModuleName = FName(TEXT("MessageLog"));

void FRuleRangerMessageLog::Initialize()
{
    // create a MessageLog category to use in plugin
    FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(MessageLogModuleName);
    FMessageLogInitializationOptions InitOptions;
    InitOptions.bShowPages = true;
    InitOptions.bAllowClear = true;
    InitOptions.bShowFilters = true;
    InitOptions.MaxPageCount = 4;
    MessageLogModule.RegisterLogListing(GetMessageLogName(),
                                        NSLOCTEXT("RuleRanger", "RuleRangerLogLabel", "Rule Ranger"),
                                        InitOptions);
    RR_VERY_VERBOSE_ALOG("FRuleRangerMessageLog::Shutdown(): Registered MessageLog.");
}

void FRuleRangerMessageLog::Shutdown()
{
    // Deregister MessageLog created in Startup
    if (FModuleManager::Get().IsModuleLoaded(MessageLogModuleName))
    {
        FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>(MessageLogModuleName);
        if (MessageLogModule.IsRegisteredLogListing(GetMessageLogName()))
        {
            RR_VERY_VERBOSE_ALOG("FRuleRangerMessageLog::Shutdown(): Deregistering MessageLog.");
            MessageLogModule.UnregisterLogListing(GetMessageLogName());
        }
        else
        {
            RR_VERBOSE_ALOG("FRuleRangerMessageLog::Shutdown(): Skipping deregister of MessageLog as not registered.");
        }
    }
}
