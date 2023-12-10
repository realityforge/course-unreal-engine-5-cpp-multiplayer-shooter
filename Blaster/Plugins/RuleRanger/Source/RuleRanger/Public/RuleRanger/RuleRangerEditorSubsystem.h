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
#include "EditorSubsystem.h"
#include "Factories/Factory.h"
#include "UObject/Object.h"
#include <functional>
#include "RuleRangerEditorSubsystem.generated.h"

class URuleRangerRule;
class URuleRangerActionContext;
class URuleRangerRuleSetScope;
class URuleRangerContentBrowserExtensions;

// Shape of function called to check whether rule will run or actually execute rule.
// The actual function is determined by where it is used.
using FRuleRangerRuleFn = std::function<bool(URuleRangerRule* Rule, UObject* InObject)>;

/**
 * The subsystem responsible for managing callbacks to other subsystems such as ImportSubsystem callbacks.
 */
UCLASS(BlueprintType, CollapseCategories)
class RULERANGER_API URuleRangerEditorSubsystem final : public UEditorSubsystem
{
    GENERATED_BODY()

    friend URuleRangerContentBrowserExtensions;

public:
    /** Implement this for initialization of instances of the system */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Implement this for deinitialization of instances of the system */
    virtual void Deinitialize() override;

    void ScanObject(UObject* InObject);
    void ScanAndFixObject(UObject* InObject);

    void ProcessRule(UObject* Object, const FRuleRangerRuleFn& ProcessRuleFunction);
    bool IsMatchingRulePresent(UObject* InObject, const FRuleRangerRuleFn& ProcessRuleFunction);

    /** Return the set of ActiveRuleSetScopes that are active for the current project. */
    TArray<TSoftObjectPtr<URuleRangerRuleSetScope>> GetActiveRuleSetScopes();

private:
    UPROPERTY(Transient)
    URuleRangerActionContext* ActionContext{ nullptr };

    // Handle for delegate that has been registered.
    FDelegateHandle OnAssetPostImportDelegateHandle;

    void OnAssetPostImport(UFactory* Factory, UObject* Object);

    /**
     * Function invoked when each rule is applied to an object during import.
     *
     * @param bIsReimport A flag indicating whether it is an import or re-import action.
     * @param Rule The rule to apply.
     * @param InObject the object to apply rule to.
     * @return true to keep processing, false if no more rules should be applied to object.
     */
    bool ProcessOnAssetPostImportRule(const bool bIsReimport, URuleRangerRule* Rule, UObject* InObject);

    /**
     * Function invoked when each rule is applied to an object when user requested an explicit scan.
     *
     * @param Rule The rule to apply.
     * @param InObject the object to apply rule to.
     * @return true to keep processing, false if no more rules should be applied to object.
     */
    bool ProcessDemandScan(URuleRangerRule* Rule, UObject* InObject);

    /**
     * Function invoked when each rule is applied to an object when user requested an explicit scan and autofix.
     *
     * @param Rule The rule to apply.
     * @param InObject the object to apply rule to.
     * @return true to keep processing, false if no more rules should be applied to object.
     */
    bool ProcessDemandScanAndFix(URuleRangerRule* Rule, UObject* InObject);
};
