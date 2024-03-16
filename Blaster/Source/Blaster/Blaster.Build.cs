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

		// Generate compile errors if using DrawDebug functions in test/shipping builds.
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");
	}
}