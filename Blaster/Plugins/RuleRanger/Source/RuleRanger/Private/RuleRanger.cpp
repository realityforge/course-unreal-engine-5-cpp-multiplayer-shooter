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
#include "RuleRanger.h"
#include "MessageLogModule.h"
#include "RuleRangerLogging.h"

#define LOCTEXT_NAMESPACE "FRuleRangerModule"

static const FName MessageLogModuleName = FName(TEXT("MessageLog"));
const FName RuleRangerMessageLogName = FName(TEXT("RuleRanger"));

void FRuleRangerModule::StartupModule()
{
    // This code will execute after your module is loaded into memory;
    // the exact timing is specified in the .uplugin file per-module

    // create a MessageLog category to use in plugin
    FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(MessageLogModuleName);
    FMessageLogInitializationOptions InitOptions;
    InitOptions.bShowPages = true;
    InitOptions.bAllowClear = true;
    InitOptions.bShowFilters = true;
    MessageLogModule.RegisterLogListing(RuleRangerMessageLogName,
                                        NSLOCTEXT("RuleRanger", "RuleRangerLogLabel", "Rule Ranger"),
                                        InitOptions);
}

void FRuleRangerModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.
    // For modules that support dynamic reloading, we call this function before
    // unloading the module.

    // Deregister MessageLog created in Startup
    if (FModuleManager::Get().IsModuleLoaded(MessageLogModuleName))
    {
        FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>(MessageLogModuleName);
        if (MessageLogModule.IsRegisteredLogListing(RuleRangerMessageLogName))
        {
            MessageLogModule.UnregisterLogListing(RuleRangerMessageLogName);
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRuleRangerModule, RuleRanger)

// The implementation of log category top use within plugin
DEFINE_LOG_CATEGORY(RuleRanger);
