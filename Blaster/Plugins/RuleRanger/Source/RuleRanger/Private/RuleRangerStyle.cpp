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
#include "RuleRangerStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "RuleRangerLogging.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FRuleRangerStyle::StyleInstance = nullptr;

void FRuleRangerStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        RR_VERY_VERBOSE_ALOG("FRuleRangerStyle::Initialize(): Creating StyleSet.");
        StyleInstance = Create();
        RR_VERY_VERBOSE_ALOG("FRuleRangerStyle::Initialize(): Registering StyleSet.");
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
    else
    {
        RR_VERY_VERBOSE_ALOG("FRuleRangerStyle::Initialize(): Skipping "
                             "creation and registration of StyleSheet as already valid.");
    }
}

void FRuleRangerStyle::Shutdown()
{
    RR_VERY_VERBOSE_ALOG("FRuleRangerStyle::Shutdown: Deregistering StyleSet.");
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
    ensure(StyleInstance.IsUnique());
    StyleInstance.Reset();
}

FName FRuleRangerStyle::GetStyleSetName()
{
    static FName StyleSetName(TEXT("RuleRangerStyle"));
    return StyleSetName;
}

TSharedRef<FSlateStyleSet> FRuleRangerStyle::Create()
{
    const FName RuleRangerModuleName = FName(TEXT("RuleRanger"));
    const FVector2D Icon16x16(16.0f, 16.0f);

    // The resources directory of the RuleRanger Plugin
    const auto ResourcesDir =
        IPluginManager::Get().FindPlugin(RuleRangerModuleName.ToString())->GetBaseDir() / TEXT("Resources");

    TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    Style->SetContentRoot(ResourcesDir);

    Style->Set(TEXT("RuleRanger.ScanSelectedPaths"),
               new FSlateImageBrush(ResourcesDir / TEXT("Icons/magnifying_glass.png"), Icon16x16));
    Style->Set(TEXT("RuleRanger.FixSelectedPaths"),
               new FSlateImageBrush(ResourcesDir / TEXT("Icons/magnifying_glass.png"), Icon16x16));
    Style->Set(TEXT("RuleRanger.ScanSelectedAssets"),
               new FSlateImageBrush(ResourcesDir / TEXT("Icons/magnifying_glass.png"), Icon16x16));
    Style->Set(TEXT("RuleRanger.FixSelectedAssets"),
               new FSlateImageBrush(ResourcesDir / TEXT("Icons/magnifying_glass.png"), Icon16x16));

    return Style;
}

void FRuleRangerStyle::ReloadTextures()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
    }
}

const ISlateStyle& FRuleRangerStyle::Get()
{
    return *StyleInstance;
}
