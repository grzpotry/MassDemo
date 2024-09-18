// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MassDemo : ModuleRules
{
	public MassDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "MassEntity", "MassAIBehavior", "MassCommon",
			"StateTreeModule", "MassSignals",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "MassEntity", "MassCommon", "MassNavigation", "StructUtils", "MassGameplayDebug", "MassRepresentation" });
		
		PublicDefinitions.Add("WITH_MASSGAMEPLAY_DEBUG=1");

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
