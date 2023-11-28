// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Subocean : ModuleRules
{
	public Subocean(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
