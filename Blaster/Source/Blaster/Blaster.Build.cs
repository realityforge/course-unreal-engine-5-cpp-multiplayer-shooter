using UnrealBuildTool;

public class Blaster : ModuleRules
{
	public Blaster(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara" });
		PrivateDependencyModuleNames.AddRange(new[] { "EnhancedInput" });

		// This ensures we can load header files from the same hierarchy.
		// This just makes it a lot easier when doing development
		PublicIncludePaths.AddRange( new[] { "Blaster" } );
	}
}