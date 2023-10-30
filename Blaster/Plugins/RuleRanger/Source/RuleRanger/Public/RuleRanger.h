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

#include "ContentBrowserDelegates.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FRuleRangerModule : public IModuleInterface
{
public:
    // -------------------------------------------------------------------------------------------
    // Core Module functionality
    // -------------------------------------------------------------------------------------------

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    // -------------------------------------------------------------------------------------------
    // MessageLog
    // -------------------------------------------------------------------------------------------

    void ReregisterMessageLogLogs();
    void DeregisterStyleSet();

    // -------------------------------------------------------------------------------------------
    // StyleSet
    // -------------------------------------------------------------------------------------------

    // StyleSet used for UI elements
    TSharedPtr<FSlateStyleSet> StyleSet{ nullptr };

    // Initialize the StyleSet. Called during StartupModule
    void RegisterStyleSet();

    // -------------------------------------------------------------------------------------------
    // Content browser extensions
    // -------------------------------------------------------------------------------------------

    // Delegate called when extending ContextBrowser SelectedPaths Context menu
    FContentBrowserMenuExtender_SelectedPaths ContentBrowserMenuExtender_SelectedPathsDelegate;
    FDelegateHandle ContentBrowserMenuExtender_SelectedPathsDelegateHandle;

    // Delegate called when extending ContextBrowser SelectedAssets Context menu
    FContentBrowserMenuExtender_SelectedAssets ContentBrowserMenuExtender_SelectedAssetsDelegate;
    FDelegateHandle ContentBrowserMenuExtender_SelectedAssetsDelegateHandle;

    void RegisterContentBrowserExtensions();
    void RegisterMessageLogLogs();
    void DeregisterContentBrowserExtensions() const;
};
