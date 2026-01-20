// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Sandbox : ModuleRules
{
	public Sandbox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[] { "Sandbox" });

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
