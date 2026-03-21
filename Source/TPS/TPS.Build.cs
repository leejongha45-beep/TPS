// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TPS : ModuleRules
{
	public TPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;
		bUseUnity = false;
		IWYUSupport = IWYUSupport.None;

		PublicIncludePaths.AddRange(new string[] { ModuleDirectory });

		// EnTT (header-only ECS 라이브러리)
		PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "ThirdParty", "EnTT", "include"));

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "GameplayTags" });

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime", "AnimationLocomotionLibraryRuntime", "AnimationWarpingRuntime", "Slate", "SlateCore", "Niagara", "PhysicsCore", "DeveloperSettings", "MediaAssets", "NavigationSystem", "AIModule", "AIModule", "Landscape" });
	}
}