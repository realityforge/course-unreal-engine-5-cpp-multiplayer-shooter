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
#include "RuleRangerCommands.h"
#include "RuleRangerStyle.h"

#define LOCTEXT_NAMESPACE "FRuleRangerModule"

FRuleRangerCommands::FRuleRangerCommands()
    : TCommands(TEXT("RuleRanger"),
                NSLOCTEXT("RuleRanger", "RuleRanger", "RuleRanger Plugin"),
                NAME_None,
                FRuleRangerStyle::GetStyleSetName())
{
}

void FRuleRangerCommands::RegisterCommands()
{
    UI_COMMAND(ScanSelectedPaths,
               "Scan with RuleRanger",
               "Scan selected paths with RuleRanger",
               EUserInterfaceActionType::Button,
               FInputChord());
    UI_COMMAND(FixSelectedPaths,
               "Apply fixes with RuleRanger",
               "Scan and fix selected paths with RuleRanger",
               EUserInterfaceActionType::Button,
               FInputChord());
    UI_COMMAND(ScanSelectedAssets,
               "Scan with RuleRanger",
               "Scan selected assets with RuleRanger",
               EUserInterfaceActionType::Button,
               FInputChord());
    UI_COMMAND(FixSelectedAssets,
               "Apply fixes with RuleRanger",
               "Scan and fix selected assets with RuleRanger",
               EUserInterfaceActionType::Button,
               FInputChord());
}

#undef LOCTEXT_NAMESPACE
