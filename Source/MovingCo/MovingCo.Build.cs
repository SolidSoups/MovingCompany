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
			"GameplayTasks",
			"NavigationSystem",
			"ProceduralMeshComponent",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
            "Paper2D",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"CommonUI",
			"CommonInput",
            "GeometryCache"
		});


		PrivateDependencyModuleNames.AddRange(new string[] {
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });

		PublicIncludePaths.Add("MovingCo");

        bool bShipping = Target.Configuration == UnrealTargetConfiguration.Shipping;
        PublicDefinitions.Add(bShipping ? "MCO_DEBUG=0" : "MCO_DEBUG=1");
	}
}
