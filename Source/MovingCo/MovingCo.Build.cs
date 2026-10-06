// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MovingCo : ModuleRules
{
	public MovingCo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MovingCo",
            "MovingCo/Destruction",
            "MovingCo/Furniture",
			"MovingCo/Variant_Horror",
			"MovingCo/Variant_Horror/UI",
			"MovingCo/Variant_Shooter",
			"MovingCo/Variant_Shooter/AI",
			"MovingCo/Variant_Shooter/UI",
			"MovingCo/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
