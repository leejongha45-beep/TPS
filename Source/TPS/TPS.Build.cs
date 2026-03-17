// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TPS : ModuleRules
{
	public TPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;
		bUseUnity = false;
		bEnforceIWYU = false;

		PublicIncludePaths.AddRange(new string[] { ModuleDirectory });

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "GameplayTags" });

		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime", "AnimationLocomotionLibraryRuntime", "AnimationWarpingRuntime", "Slate", "SlateCore", "Niagara", "PhysicsCore", "DeveloperSettings", "MediaAssets" });

		// Mass Entity — 대군단 적 시스템
		PublicDependencyModuleNames.AddRange(new string[] { "MassEntity", "MassCommon", "MassSpawner", "StructUtils" });
	}
}