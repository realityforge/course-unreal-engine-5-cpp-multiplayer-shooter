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

#include "RuleRangerContentBrowserExtensions.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "RuleRanger/RuleRangerEditorSubsystem.h"
#include "RuleRangerCommands.h"
#include "RuleRangerLogging.h"
#include "RuleRangerMessageLog.h"

static void OnScanSelectedAssets(const TArray<FAssetData>& Assets)
{
    FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
    MessageLog.NewPage(FText::FromString(TEXT("Scan Selected Assets")));
    MessageLog.Info()->AddToken(
        FTextToken::Create(NSLOCTEXT("RuleRanger", "ScanAssetsStarting", "Rule Ranger will scan the selected assets")));
    MessageLog.Open(EMessageSeverity::Info, true);

    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        for (auto& Asset : Assets)
        {
            // Object can be null if it is a redirect
            if (UObject* Object = Asset.GetAsset())
            {
                Subsystem->ScanObject(Object);
            }
        }
    }
    MessageLog.Info()->AddToken(FTextToken::Create(
        NSLOCTEXT("RuleRanger", "ScanAssetsCompleted", "Rule Ranger has completed scanning of the selected assets")));
}

static void OnFixSelectedAssets(const TArray<FAssetData>& Assets)
{
    FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
    MessageLog.NewPage(FText::FromString(TEXT("Fix Selected Assets")));
    MessageLog.Info()->AddToken(FTextToken::Create(
        NSLOCTEXT("RuleRanger", "ScanAndFixAssetsStarting", "Rule Ranger will scan and fix the selected assets")));
    MessageLog.Open(EMessageSeverity::Info, true);

    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        for (auto& Asset : Assets)
        {
            // Object can be null if it is a redirect
            if (UObject* Object = Asset.GetAsset())
            {
                Subsystem->ScanAndFixObject(Object);
            }
        }
    }
    MessageLog.Info()->AddToken(
        FTextToken::Create(NSLOCTEXT("RuleRanger",
                                     "ScanAndFixAssetsCompleted",
                                     "Rule Ranger has completed scanning of the selected assets")));
}
static void OnScanSelectedPaths(const TArray<FString>& AssetPaths)
{
    FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
    MessageLog.NewPage(FText::FromString(TEXT("Scan Selected Paths")));
    MessageLog.Info()->AddToken(
        FTextToken::Create(NSLOCTEXT("RuleRanger", "ScanPathsStarting", "Rule Ranger will scan the selected paths")));
    MessageLog.Open(EMessageSeverity::Info, true);

    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        const auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
        for (auto& AssetPath : AssetPaths)
        {
            const auto& AssetData = AssetRegistry.GetAssetByObjectPath(AssetPath);
            if (AssetData.IsRedirector())
            {
                // Ignore
            }
            else if (AssetData.IsUAsset())
            {
                if (UObject* Object = AssetData.GetAsset())
                {
                    Subsystem->ScanObject(Object);
                }
            }
            else
            {
                // Assuming this is a folder
                TArray<FAssetData> Assets;
                AssetRegistry.GetAssetsByPath(*AssetPath, Assets, true);

                for (auto& Asset : Assets)
                {
                    if (UObject* Object = Asset.GetAsset())
                    {
                        Subsystem->ScanObject(Object);
                    }
                }
            }
        }
    }
    MessageLog.Info()->AddToken(FTextToken::Create(
        NSLOCTEXT("RuleRanger", "ScanPathsCompleted", "Rule Ranger has completed scanning of the selected paths")));
}

static void OnFixSelectedPaths(const TArray<FString>& AssetPaths)
{
    FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
    MessageLog.NewPage(FText::FromString(TEXT("Fix Selected Paths")));
    MessageLog.Info()->AddToken(FTextToken::Create(
        NSLOCTEXT("RuleRanger", "ScanAndFixPathsStarting", "Rule Ranger will scan and fix the selected paths")));
    MessageLog.Open(EMessageSeverity::Info, true);

    if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
    {
        const auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
        for (auto& AssetPath : AssetPaths)
        {
            const auto& AssetData = AssetRegistry.GetAssetByObjectPath(AssetPath);
            if (AssetData.IsRedirector())
            {
                // Ignore
            }
            else if (AssetData.IsUAsset())
            {
                if (UObject* Object = AssetData.GetAsset())
                {
                    Subsystem->ScanObject(Object);
                }
            }
            else
            {
                // Assuming this is a folder
                TArray<FAssetData> Assets;
                AssetRegistry.GetAssetsByPath(*AssetPath, Assets, true);

                for (auto& Asset : Assets)
                {
                    if (UObject* Object = Asset.GetAsset())
                    {
                        Subsystem->ScanAndFixObject(Object);
                    }
                }
            }
        }
    }
    MessageLog.Info()->AddToken(
        FTextToken::Create(NSLOCTEXT("RuleRanger",
                                     "ScanAndFixPathsCompleted",
                                     "Rule Ranger has completed scanning and fixing of the selected paths")));
}

// ReSharper disable once CppPassValueParameterByConstReference
static void OnExtendForSelectedPaths(FMenuBuilder& MenuBuilder)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("FRuleRangerContentBrowserExtensions: OnExtendContentBrowser() invoked."));

    MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                             NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));

    MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().ScanSelectedPaths);
    MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().FixSelectedPaths);

    MenuBuilder.AddSeparator();
    MenuBuilder.EndSection();
}

static TSharedRef<FExtender> OnExtendSelectedPathsMenu(const TArray<FString>& Paths)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("OnExtendSelectedPathsMenu() invoked."));

    const TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList);
    CommandList->MapAction(FRuleRangerCommands::Get().ScanSelectedPaths,
                           FExecuteAction::CreateLambda(([Paths]() { OnScanSelectedPaths(Paths); })),
                           FCanExecuteAction());
    CommandList->MapAction(FRuleRangerCommands::Get().FixSelectedPaths,
                           FExecuteAction::CreateLambda(([Paths]() { OnFixSelectedPaths(Paths); })),
                           FCanExecuteAction());
    auto Extender = MakeShared<FExtender>();
    const auto MenuExtensionDelegate = FMenuExtensionDelegate::CreateStatic(&OnExtendForSelectedPaths);
    Extender->AddMenuExtension("PathContextBulkOperations", EExtensionHook::After, CommandList, MenuExtensionDelegate);
    return Extender;
}

// ReSharper disable once CppPassValueParameterByConstReference
static void OnExtendForSelectedAssets(FMenuBuilder& MenuBuilder)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("OnExtendForSelectedAssets() invoked."));

    MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                             NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));

    MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().ScanSelectedAssets);
    MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().FixSelectedAssets);

    MenuBuilder.AddSeparator();
    MenuBuilder.EndSection();
}

static TSharedRef<FExtender> OnExtendForSelectedAssetsMenu(const TArray<FAssetData>& Assets)
{
    UE_LOG(RuleRanger, VeryVerbose, TEXT("OnExtendForSelectedAssetsMenu() invoked."));

    const TSharedPtr<FUICommandList> CommandList = MakeShareable(new FUICommandList);
    CommandList->MapAction(FRuleRangerCommands::Get().ScanSelectedAssets,
                           FExecuteAction::CreateLambda(([Assets]() { OnScanSelectedAssets(Assets); })),
                           FCanExecuteAction());
    CommandList->MapAction(FRuleRangerCommands::Get().FixSelectedAssets,
                           FExecuteAction::CreateLambda(([Assets]() { OnFixSelectedAssets(Assets); })),
                           FCanExecuteAction());

    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    const auto MenuExtensionDelegate = FMenuExtensionDelegate::CreateStatic(&OnExtendForSelectedAssets);
    Extender->AddMenuExtension("CommonAssetActions", EExtensionHook::After, CommandList, MenuExtensionDelegate);

    return Extender;
}

FContentBrowserMenuExtender_SelectedPaths FRuleRangerContentBrowserExtensions::SelectedPathsDelegate;
FDelegateHandle FRuleRangerContentBrowserExtensions::SelectedPathsDelegateHandle;
FContentBrowserMenuExtender_SelectedAssets FRuleRangerContentBrowserExtensions::SelectedAssetsDelegate;
FDelegateHandle FRuleRangerContentBrowserExtensions::SelectedAssetsDelegateHandle;

void FRuleRangerContentBrowserExtensions::Initialize()
{
    auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

    UE_LOG(RuleRanger,
           VeryVerbose,
           TEXT("FRuleRangerContentBrowserExtensions::Shutdown(): Registering ContentBrowser Extensions."));
    SelectedPathsDelegate = FContentBrowserMenuExtender_SelectedPaths::CreateStatic(&OnExtendSelectedPathsMenu);
    Module.GetAllPathViewContextMenuExtenders().Add(SelectedPathsDelegate);
    SelectedPathsDelegateHandle = SelectedPathsDelegate.GetHandle();
    check(SelectedPathsDelegateHandle.IsValid());

    // Asset extenders
    SelectedAssetsDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&OnExtendForSelectedAssetsMenu);
    Module.GetAllAssetViewContextMenuExtenders().Add(SelectedAssetsDelegate);
    SelectedAssetsDelegateHandle = SelectedAssetsDelegate.GetHandle();
    check(SelectedAssetsDelegateHandle.IsValid());
}

void FRuleRangerContentBrowserExtensions::Shutdown()
{
    // ReSharper disable once CppTooWideScopeInitStatement
    const FName ContentBrowserModuleName{ TEXT("ContentBrowser") };
    if (FModuleManager::Get().IsModuleLoaded(ContentBrowserModuleName))
    {
        auto& Module = FModuleManager::LoadModuleChecked<FContentBrowserModule>(ContentBrowserModuleName);
        if (SelectedPathsDelegateHandle.IsValid())
        {
            UE_LOG(RuleRanger,
                   VeryVerbose,
                   TEXT("FRuleRangerContentBrowserExtensions::Shutdown(): Deregistering ContentBrowser Extensions."));
            auto Target = SelectedPathsDelegateHandle;
            Module.GetAllPathViewContextMenuExtenders().RemoveAll(
                [&Target](const auto& Delegate) { return Delegate.GetHandle() == Target; });
            Target.Reset();
        }
        else
        {
            UE_LOG(RuleRanger,
                   Verbose,
                   TEXT("FRuleRangerContentBrowserExtensions::Shutdown(): Skipping deregister of "
                        "ContentBrowserExtensions as handle is Invalid."));
        }
    }
}
