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
#include "RuleRangerActionContext.h"
#include "Misc/UObjectToken.h"
#include "RuleRangerMessageLog.h"
#include "RuleRangerRule.h"

ERuleRangerActionState URuleRangerActionContext::GetState()
{
    return ActionState;
}

ERuleRangerActionTrigger URuleRangerActionContext::GetActionTrigger()
{
    return ActionTrigger;
}

bool URuleRangerActionContext::IsDryRun()
{
    return !(ERuleRangerActionTrigger::AT_Save == ActionTrigger || ERuleRangerActionTrigger::AT_Import == ActionTrigger
             || ERuleRangerActionTrigger::AT_Reimport == ActionTrigger
             || ERuleRangerActionTrigger::AT_Fix == ActionTrigger);
}

void URuleRangerActionContext::ResetContext(URuleRangerRule* const InRule,
                                            UObject* const InObject,
                                            const ERuleRangerActionTrigger InActionTrigger)
{
    check(nullptr != InObject);
    Rule = InRule;
    Object = InObject;
    ActionTrigger = InActionTrigger;
    ActionState = ERuleRangerActionState::AS_Success;
    InfoMessages.Reset();
    WarningMessages.Reset();
    ErrorMessages.Reset();
    FatalMessages.Reset();
}

void URuleRangerActionContext::ClearContext()
{
    Object = nullptr;
    ActionTrigger = ERuleRangerActionTrigger::AT_Report;
    ActionState = ERuleRangerActionState::AS_Success;
    InfoMessages.Reset();
    WarningMessages.Reset();
    ErrorMessages.Reset();
    FatalMessages.Reset();
}

void URuleRangerActionContext::EmitMessageLogs()
{
    for (int i = 0; i < InfoMessages.Num(); i++)
    {
        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
            .Info()
            ->AddToken(FUObjectToken::Create(Object))
            ->AddToken(FTextToken::Create(InfoMessages[i]));
    }
    for (int i = 0; i < WarningMessages.Num(); i++)
    {
        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
            .Warning()
            ->AddToken(FUObjectToken::Create(Object))
            ->AddToken(FTextToken::Create(WarningMessages[i]));
    }
    for (int i = 0; i < ErrorMessages.Num(); i++)
    {
        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
            .Error()
            ->AddToken(FUObjectToken::Create(Object))
            ->AddToken(FTextToken::Create(ErrorMessages[i]));
    }
    for (int i = 0; i < FatalMessages.Num(); i++)
    {
        FMessageLog(FRuleRangerMessageLog::GetMessageLogName())
            .Error()
            ->AddToken(FUObjectToken::Create(Object))
            ->AddToken(FTextToken::Create(FatalMessages[i]));
    }
}

FText URuleRangerActionContext::ToMessage(const FText& InMessage) const
{
    return FText::FromString(FString::Printf(TEXT("%s (Emitted from rule %s)"),
                                             *InMessage.ToString(),
                                             Rule ? *Rule->GetOutermost()->GetPathName() : TEXT("?")));
}

void URuleRangerActionContext::Info(const FText& InMessage)
{
    InfoMessages.Add(ToMessage(InMessage));
}
void URuleRangerActionContext::Warning(const FText& InMessage)
{
    WarningMessages.Add(ToMessage(InMessage));
    ActionState = ActionState < ERuleRangerActionState::AS_Warning ? ERuleRangerActionState::AS_Warning : ActionState;
}
void URuleRangerActionContext::Error(const FText& InMessage)
{
    ErrorMessages.Add(ToMessage(InMessage));
    ActionState = ActionState < ERuleRangerActionState::AS_Error ? ERuleRangerActionState::AS_Error : ActionState;
}
void URuleRangerActionContext::Fatal(const FText& InMessage)
{
    FatalMessages.Add(ToMessage(InMessage));
    ActionState = ActionState < ERuleRangerActionState::AS_Fatal ? ERuleRangerActionState::AS_Fatal : ActionState;
}
