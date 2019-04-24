// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class LeapMotionEditor : ModuleRules
	{

		public LeapMotionEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"LeapMotionEditor/Private",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add other public dependencies that you statically link with here ...
					"Engine",
					"Core",
					"CoreUObject",
					"LeapMotion",
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"BlueprintGraph",
					"AnimGraph",
					"AnimGraphRuntime",
					"BodyState"
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}