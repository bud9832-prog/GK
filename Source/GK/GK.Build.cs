// Copyright Ashen Ossuary. All Rights Reserved.

using UnrealBuildTool;

public class GK : ModuleRules
{
	public GK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"PhysicsCore",
			"AkAudio"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
