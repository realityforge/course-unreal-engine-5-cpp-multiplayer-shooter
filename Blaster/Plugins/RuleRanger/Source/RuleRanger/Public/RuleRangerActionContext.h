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
#include "UObject/Interface.h"
#include "RuleRangerActionContext.generated.h"

class URuleRangerRuleSet;
class URuleRangerConfig;
class URuleRangerRule;

UENUM(BlueprintType)
enum class ERuleRangerActionTrigger : uint8
{
    /** The action was invoked as a result of an import. */
    AT_Import UMETA(DisplayName = "Import"),
    /** The action was invoked as a result of a re-import. */
    AT_Reimport UMETA(DisplayName = "ReImport"),
    /** The action was invoked as a result of a validate. */
    AT_Validate UMETA(DisplayName = "Validate"),
    /** The action was invoked as a result of a validate. */
    AT_Save UMETA(DisplayName = "Save"),
    /** The action was invoked as an explicit "report" action from a user. */
    AT_Report UMETA(DisplayName = "Report"),
    /** The action was invoked as an explicit "fix" action from a user. */
    AT_Fix UMETA(DisplayName = "Fix"),

    AT_Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ERuleRangerActionState : uint8
{
    /** The action was successful and produced no warnings or errors. */
    AS_Success UMETA(DisplayName = "Success"),
    /** The action produced one or more warnings but no errors. */
    AS_Warning UMETA(DisplayName = "Warning"),
    /** The action produced one or more errors but the errors are not fatal. */
    AS_Error UMETA(DisplayName = "Error"),
    /** The action produced one or more fatal errors and further rule processing should not occur for object. */
    AS_Fatal UMETA(DisplayName = "Fatal"),

    AS_Max UMETA(Hidden)
};

/**
 * Context object passed to an action so that the action can be provided context.
 */
UCLASS(BlueprintType, Transient)
class RULERANGER_API URuleRangerActionContext : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Return the current state of the action.
     *
     * @return the current state of the action.
     */
    UFUNCTION(BlueprintCallable, Category = "Rule Ranger")
    virtual ERuleRangerActionState GetState();

    /**
     * Return the trigger for the current action. i.e. Why was this action invoked.
     *
     * @return the trigger for the current action
     */
    UFUNCTION(BlueprintCallable, Category = "Rule Ranger")
    virtual ERuleRangerActionTrigger GetActionTrigger();

    /**
     * Return true if this action is a "Dry" run and should just issue warnings on non-compliance
     * and info on action that would take to fix compliance.
     *
     * @return true if the action should be a dry run.
     */
    UFUNCTION(BlueprintCallable, Category = "Rule Ranger")
    virtual bool IsDryRun();

    /**
     * Generate an informational message from action.
     *
     * @param InMessage the message.
     */
    UFUNCTION(BlueprintCallable, Category = "Rule Ranger")
    virtual void Info(const FText& InMessage);

    /**
     * Generate a warning message from the action.
     *
     * @param InMessage the message.
     */
    UFUNCTION(BlueprintCallable, Category = "Rule Ranger")
    virtual void Warning(const FText& InMessage);

    /**
     * Generate an error message from the action.
     *
     * @param InMessage the message.
     */
    UFUNCTION(BlueprintCallable, Category = "Rule Ranger")
    virtual void Error(const FText& InMessage);

    /**
     * Generate an error message from the action.
     *
     * @param InMessage the message.
     */
    UFUNCTION(BlueprintCallable, Category = "Rule Ranger")
    virtual void Fatal(const FText& InMessage);

    void ResetContext(URuleRangerConfig* const InConfig,
                      URuleRangerRuleSet* const InRuleSet,
                      URuleRangerRule* InRule,
                      UObject* InObject,
                      ERuleRangerActionTrigger InActionTrigger);
    void ClearContext();

    void EmitMessageLogs();

private:
    /** A reference to the RuleRangerConfig that transitively included this Action. */
    UPROPERTY(Transient)
    TObjectPtr<URuleRangerConfig> Config;

    /** A reference to the RuleRangerRuleSet that directly included this Action. */
    UPROPERTY(Transient)
    TObjectPtr<URuleRangerRuleSet> RuleSet;

    /** The rule that contains the associated action that is using the context. */
    UPROPERTY(Transient)
    TObjectPtr<URuleRangerRule> Rule;

    /** The object that the associated action is acting upon. */
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UObject> Object;

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

    FText ToMessage(const FText& InMessage) const;

public:
    FORCEINLINE URuleRangerConfig const* GetOwnerConfig() const { return Config; }
    FORCEINLINE URuleRangerRuleSet const* GetOwnerRuleSet() const { return RuleSet; }

    FORCEINLINE TArray<FText>& GetInfoMessages() { return InfoMessages; }
    FORCEINLINE TArray<FText>& GetWarningMessages() { return WarningMessages; }
    FORCEINLINE TArray<FText>& GetErrorMessages() { return ErrorMessages; }
    FORCEINLINE TArray<FText>& GetFatalMessages() { return FatalMessages; }
};
