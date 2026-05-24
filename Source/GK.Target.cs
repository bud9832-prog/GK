// Copyright Ashen Ossuary. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class GKTarget : TargetRules
{
	public GKTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("GK");
	}
}
