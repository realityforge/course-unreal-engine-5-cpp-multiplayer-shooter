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

#include "CoreMinimal.h"
#include "RuleRangerActionContext.h"
#include "UObject/Object.h"
#include "ActionContextImpl.generated.h"

/**
 * ActionContext implementation used during the import process.
 */
UCLASS(Blueprintable, BlueprintType)
class RULERANGER_API UActionContextImpl : public UObject, public IRuleRangerActionContext
{
    GENERATED_BODY()

public:
    virtual ERuleRangerActionState GetState() override;
    virtual ERuleRangerActionTrigger GetActionTrigger() override;
    virtual bool IsDryRun() override;
    virtual void Info(const FText& InMessage) override;
    virtual void Warning(const FText& InMessage) override;
    virtual void Error(const FText& InMessage) override;
    virtual void Fatal(const FText& InMessage) override;

    void ResetContext(UObject* InObject, ERuleRangerActionTrigger InActionTrigger);
    void ClearContext();

    void EmitMessageLogs();

private:
    /** The object that the associated action is acting upon. */
    UPROPERTY(VisibleAnywhere)
    UObject* Object;

    /** The reason that the associated action was triggered. */
    UPROPERTY(VisibleAnywhere)
    ERuleRangerActionTrigger ActionTrigger{ ERuleRangerActionTrigger::AT_Max };

    /** Current state of the action. */
    UPROPERTY(VisibleAnywhere)
    ERuleRangerActionState ActionState{ ERuleRangerActionState::AS_Max };

    /** The array of info messages. */
    UPROPERTY(VisibleAnywhere)
    TArray<FText> InfoMessages;

    /** The array of warning messages. */
    UPROPERTY(VisibleAnywhere)
    TArray<FText> WarningMessages;

    /** The array of error messages. */
    UPROPERTY(VisibleAnywhere)
    TArray<FText> ErrorMessages;

    /** The array of fatal messages. */
    UPROPERTY(VisibleAnywhere)
    TArray<FText> FatalMessages;

public:
    FORCEINLINE TArray<FText>& GetInfoMessages() { return InfoMessages; }
    FORCEINLINE TArray<FText>& GetWarningMessages() { return WarningMessages; }
    FORCEINLINE TArray<FText>& GetErrorMessages() { return ErrorMessages; }
    FORCEINLINE TArray<FText>& GetFatalMessages() { return FatalMessages; }
};
