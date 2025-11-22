// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UESteamMultiplayer : ModuleRules
{
	public UESteamMultiplayer(ReadOnlyTargetRules Target) : base(Target)
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
			"UESteamMultiplayer",
			"UESteamMultiplayer/Variant_Platforming",
			"UESteamMultiplayer/Variant_Platforming/Animation",
			"UESteamMultiplayer/Variant_Combat",
			"UESteamMultiplayer/Variant_Combat/AI",
			"UESteamMultiplayer/Variant_Combat/Animation",
			"UESteamMultiplayer/Variant_Combat/Gameplay",
			"UESteamMultiplayer/Variant_Combat/Interfaces",
			"UESteamMultiplayer/Variant_Combat/UI",
			"UESteamMultiplayer/Variant_SideScrolling",
			"UESteamMultiplayer/Variant_SideScrolling/AI",
			"UESteamMultiplayer/Variant_SideScrolling/Gameplay",
			"UESteamMultiplayer/Variant_SideScrolling/Interfaces",
			"UESteamMultiplayer/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
