using UnrealBuildTool;

public class MultiplayerSessions : ModuleRules
{
    public MultiplayerSessions(ReadOnlyTargetRules Target) : base(Target)
    {
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        PublicIncludePaths.AddRange(new string[] {});
        PrivateIncludePaths.AddRange(new string[] {});

        // Public dependencies are available in "Public" and "Private" directories
        PublicDependencyModuleNames.AddRange(
            new[] { "Core", "OnlineSubsystem", "OnlineSubsystemSteam", "UMG", "Slate", "SlateCore" });

        // Private dependencies are only available in "Private" directories
        PrivateDependencyModuleNames.AddRange(new[] { "CoreUObject", "Engine", "Slate", "SlateCore" });
        DynamicallyLoadedModuleNames.AddRange(new string[] {});
    }
}
