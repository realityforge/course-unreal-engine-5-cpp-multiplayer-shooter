using UnrealBuildTool;
using System.Collections.Generic;

public class BlasterEditorTarget : TargetRules
{
	public BlasterEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Blaster");
		ExtraModuleNames.Add("BlasterEditor");
	}
}