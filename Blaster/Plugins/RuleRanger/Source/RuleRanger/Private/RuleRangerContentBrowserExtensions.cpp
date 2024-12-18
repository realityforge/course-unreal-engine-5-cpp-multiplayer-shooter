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
#include "RuleRangerDeveloperSettings.h"
#include "RuleRangerLogging.h"
#include "RuleRangerMessageLog.h"

static void MaybeOpenMessageLog(FMessageLog& MessageLog)
{
    const auto DeveloperSettings = GetMutableDefault<URuleRangerDeveloperSettings>();
    check(IsValid(DeveloperSettings));
    MessageLog.Open(DeveloperSettings->bAlwaysShowMessageLog ? EMessageSeverity::Info : EMessageSeverity::Warning);
}

static void TickTask(FScopedSlowTask& SlowTask)
{
    SlowTask.EnterProgressFrame();
    SlowTask.TickProgress();
}

static void OnScanSelectedAssets(const TArray<FAssetData>& Assets)
{
    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanAssetsStarting", "Rule Ranger: Scan the selected assets"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(
            FTextToken::Create(FText::Format(NSLOCTEXT("RuleRanger",
                                                       "ScanSelectedAssetsStartingAt",
                                                       "Rule Ranger has started scanning the selected assets at {0}"),
                                             FText::AsDateTime(FDateTime::UtcNow()))));

        if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
        {
            if (SlowTask.ShouldCancel())
            {
                MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                    NSLOCTEXT("RuleRanger",
                              "CancelScanSelectedAssets",
                              "User requested that Rule Ranger cancel the scanning of the selected assets at {0}"),
                    FText::AsDateTime(FDateTime::UtcNow()))));
                MaybeOpenMessageLog(MessageLog);
                return;
            }
            for (const auto& Asset : Assets)
            {
                // Object can be null if it is a redirect
                if (const auto Object = Asset.GetAsset())
                {
                    Subsystem->ScanObject(Object);
                }
                TickTask(SlowTask);
            }
        }
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanSelectedAssetsCompleted",
                                    "Rule Ranger has completed scanning of the selected assets at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

static void OnFixSelectedAssets(const TArray<FAssetData>& Assets)
{
    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanAndFixAssetsStarting", "Rule Ranger: Scan and fix the selected assets"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixSelectedAssetsStartingAt",
                                    "Rule Ranger has started scanning and fixing of the selected assets at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));

        if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
        {
            if (SlowTask.ShouldCancel())
            {
                MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                    NSLOCTEXT(
                        "RuleRanger",
                        "CancelScanAndFixSelectedAssets",
                        "User requested that Rule Ranger cancel the scanning and fixing of the selected assets at {0}"),
                    FText::AsDateTime(FDateTime::UtcNow()))));
                MaybeOpenMessageLog(MessageLog);
                return;
            }
            for (const auto& Asset : Assets)
            {
                // Object can be null if it is a redirect
                if (const auto Object = Asset.GetAsset())
                {
                    Subsystem->ScanAndFixObject(Object);
                }
                TickTask(SlowTask);
            }
        }
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixSelectedAssetsCompleted",
                                    "Rule Ranger has completed scanning and fixing of the selected assets at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

static void CollectAssetsFromPaths(const TArray<FString>& AssetPaths, TArray<FAssetData>& Assets)
{
    const auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    for (const auto& AssetPath : AssetPaths)
    {
        // ReSharper disable once CppTooWideScopeInitStatement
        const auto& AssetData = AssetRegistry.GetAssetByObjectPath(AssetPath);
        if (AssetData.IsRedirector())
        {
            // Ignore
        }
        else if (EName::None != AssetData.AssetName)
        {
            Assets.Add(AssetData);
        }
        else
        {
            TArray<FAssetData> OutAssets;
            AssetRegistry.GetAssetsByPath(*AssetPath, OutAssets, true);
            for (auto& OutAsset : OutAssets)
            {
                if (AssetData.IsTopLevelAsset())
                {
                    Assets.Add(OutAsset);
                }
            }
        }
    }
}

static void OnScanSelectedPaths(const TArray<FString>& AssetPaths)
{
    TArray<FAssetData> Assets;
    CollectAssetsFromPaths(AssetPaths, Assets);

    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanSelectedPathsStarting", "Rule Ranger: Scan the selected paths"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(
            FTextToken::Create(FText::Format(NSLOCTEXT("RuleRanger",
                                                       "ScanSelectedPathsStartingAt",
                                                       "Rule Ranger has started scanning the selected paths at {0}"),
                                             FText::AsDateTime(FDateTime::UtcNow()))));

        if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
        {
            for (const auto& AssetData : Assets)
            {
                if (SlowTask.ShouldCancel())
                {
                    MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                        NSLOCTEXT("RuleRanger",
                                  "CancelScanSelectedPaths",
                                  "User requested that Rule Ranger cancel the scanning of the selected paths at {0}"),
                        FText::AsDateTime(FDateTime::UtcNow()))));
                    MaybeOpenMessageLog(MessageLog);
                    return;
                }

                if (UObject* Object = AssetData.GetAsset())
                {
                    Subsystem->ScanObject(Object);
                }
                TickTask(SlowTask);
            }
        }
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanSelectedPathsCompleted",
                                    "Rule Ranger has completed scanning of the selected paths at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

static void OnFixSelectedPaths(const TArray<FString>& AssetPaths)
{
    TArray<FAssetData> Assets;
    CollectAssetsFromPaths(AssetPaths, Assets);

    if (Assets.Num() > 0)
    {
        FScopedSlowTask SlowTask(
            Assets.Num(),
            NSLOCTEXT("RuleRanger", "ScanAndFixSelectedPathsStarting", "Rule Ranger: Scan and fix the selected paths"));
        SlowTask.MakeDialogDelayed(.5f, true);
        FMessageLog MessageLog(FRuleRangerMessageLog::GetMessageLogName());
        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixSelectedPathsStartingAt",
                                    "Rule Ranger has started scanning and fixing of the selected paths at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));

        if (const auto Subsystem = GEditor->GetEditorSubsystem<URuleRangerEditorSubsystem>())
        {
            for (const auto& AssetData : Assets)
            {
                if (SlowTask.ShouldCancel())
                {
                    MessageLog.Info()->AddToken(FTextToken::Create(FText::Format(
                        NSLOCTEXT(
                            "RuleRanger",
                            "CancelScanAndFixSelectedPaths",
                            "User requested that Rule Ranger cancel the scanning and fixing of the selected paths at {0}"),
                        FText::AsDateTime(FDateTime::UtcNow()))));
                    MaybeOpenMessageLog(MessageLog);
                    return;
                }

                if (const auto Object = AssetData.GetAsset())
                {
                    Subsystem->ScanAndFixObject(Object);
                }
                TickTask(SlowTask);
            }
        }

        MessageLog.Info()->AddToken(FTextToken::Create(
            FText::Format(NSLOCTEXT("RuleRanger",
                                    "ScanAndFixSelectedPathsCompleted",
                                    "Rule Ranger has completed scanning and fixing of the selected paths at {0}"),
                          FText::AsDateTime(FDateTime::UtcNow()))));
        MaybeOpenMessageLog(MessageLog);
    }
}

// ReSharper disable once CppPassValueParameterByConstReference
static void OnExtendForSelectedPaths(FMenuBuilder& MenuBuilder)
{
    RR_VERY_VERBOSE_ALOG("FRuleRangerContentBrowserExtensions: OnExtendContentBrowser() invoked.");

    MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                             NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));

    MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().ScanSelectedPaths);
    MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().FixSelectedPaths);

    MenuBuilder.AddSeparator();
    MenuBuilder.EndSection();
}

static TSharedRef<FExtender> OnExtendSelectedPathsMenu(const TArray<FString>& Paths)
{
    RR_VERY_VERBOSE_ALOG("OnExtendSelectedPathsMenu() invoked.");

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
    RR_VERY_VERBOSE_ALOG("OnExtendForSelectedAssets() invoked.");

    MenuBuilder.BeginSection("RuleRangerContentBrowserContext",
                             NSLOCTEXT("RuleRanger", "ContextMenuSectionName", "Rule Ranger"));

    MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().ScanSelectedAssets);
    MenuBuilder.AddMenuEntry(FRuleRangerCommands::Get().FixSelectedAssets);

    MenuBuilder.AddSeparator();
    MenuBuilder.EndSection();
}

static TSharedRef<FExtender> OnExtendForSelectedAssetsMenu(const TArray<FAssetData>& Assets)
{
    RR_VERY_VERBOSE_ALOG("OnExtendForSelectedAssetsMenu() invoked.");

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

    RR_VERY_VERBOSE_ALOG("FRuleRangerContentBrowserExtensions::Shutdown(): Registering ContentBrowser Extensions.");
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
            RR_VERY_VERBOSE_ALOG(
                "FRuleRangerContentBrowserExtensions::Shutdown(): Deregistering ContentBrowser Extensions.");
            auto Target = SelectedPathsDelegateHandle;
            Module.GetAllPathViewContextMenuExtenders().RemoveAll(
                [&Target](const auto& Delegate) { return Delegate.GetHandle() == Target; });
            Target.Reset();
        }
        else
        {
            RR_VERBOSE_ALOG("FRuleRangerContentBrowserExtensions::Shutdown(): Skipping deregister of "
                            "ContentBrowserExtensions as handle is Invalid.");
        }
    }
}
