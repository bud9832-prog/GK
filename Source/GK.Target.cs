// Copyright Ashen Ossuary. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class GKTarget : TargetRules
{
	public GKTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("GK");
	}
}
