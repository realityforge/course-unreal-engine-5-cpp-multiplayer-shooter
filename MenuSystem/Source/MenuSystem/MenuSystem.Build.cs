// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MenuSystem : ModuleRules
{
    public MenuSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_2;

        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay",
                                                     "EnhancedInput", "OnlineSubsystem", "OnlineSubsystemSteam" });
    }
}
