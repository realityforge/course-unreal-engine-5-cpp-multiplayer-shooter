using UnrealBuildTool;

public class BlasterEditor : ModuleRules
{
    public BlasterEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore" });
        PrivateDependencyModuleNames.AddRange(new[] { "RuleRanger", "Blaster" });
        PrivateIncludePathModuleNames.AddRange(new[] { "Blaster" });
    }
}
