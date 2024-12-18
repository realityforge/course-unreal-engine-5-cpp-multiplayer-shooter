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
#include "RuleRangerCommands.h"
#include "RuleRangerContentBrowserExtensions.h"
#include "RuleRangerDefaultEvents.h"
#include "RuleRangerLogging.h"
#include "RuleRangerMessageLog.h"
#include "RuleRangerStyle.h"

const FName FRuleRangerModule::ModuleName{ TEXT("RuleRanger") };

// -------------------------------------------------------------------------------------------
// Core Module functionality
// -------------------------------------------------------------------------------------------

void FRuleRangerModule::StartupModule()
{
    FRuleRangerMessageLog::Initialize();

    if (!IsRunningCommandlet())
    {
        // Add integrations of RuleRanger into Editor UI
        RR_VERY_VERBOSE_ALOG("RuleRangerModule: Initializing Editor integration.");
        FRuleRangerStyle::Initialize();
        FRuleRangerStyle::ReloadTextures();
        FRuleRangerCommands::Register();
        FRuleRangerContentBrowserExtensions::Initialize();
        FRuleRangerDefaultEvents::Initialize(this);
    }
    else
    {
        RR_VERY_VERBOSE_ALOG("RuleRangerModule: Skipping Editor integration as running as a commandlet.");
    }
}

void FRuleRangerModule::ShutdownModule()
{
    if (!IsRunningCommandlet())
    {
        // Remove integrations of RuleRanger into Editor UI
        FRuleRangerDefaultEvents::Shutdown(this);
        FRuleRangerContentBrowserExtensions::Shutdown();
        FRuleRangerCommands::Unregister();
        FRuleRangerStyle::Shutdown();
    }

    FRuleRangerMessageLog::Shutdown();
}

IMPLEMENT_MODULE(FRuleRangerModule, RuleRanger)
