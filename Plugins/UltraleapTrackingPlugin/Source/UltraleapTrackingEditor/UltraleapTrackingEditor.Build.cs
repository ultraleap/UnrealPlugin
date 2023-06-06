/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class UltraleapTrackingEditor : ModuleRules
	{
		private string ModulePath
		{
			get { return ModuleDirectory; }
		}
		private bool IsEnginePlugin()
		{
			return Path.GetFullPath(ModuleDirectory).EndsWith("Engine\\Plugins\\Runtime\\UltraleapTracking\\Source\\UltraleapTracking");
		}
		private string ThirdPartyPath
		{
			get
			{
				if (IsEnginePlugin())
				{
					return Path.GetFullPath(Path.Combine(EngineDirectory, "Source/ThirdParty"));
				}
				else
				{
					return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty/"));
				}
			}
		}

		private string IncludePath
		{
			get
			{
				if (IsEnginePlugin())
				{
					return Path.GetFullPath(Path.Combine(ThirdPartyPath, "Leap/Include"));
				}
				else
				{
					return Path.GetFullPath(Path.Combine(ThirdPartyPath, "LeapSDK/Include"));
				}
			}
		}


		public UltraleapTrackingEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			//OptimizeCode = CodeOptimization.Never;
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"UltraleapTrackingEditor/Private",
					IncludePath,
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add other public dependencies that you statically link with here ...
					"Engine",
					"Core",
					"CoreUObject",
					"UltraleapTracking",
					"PropertyEditor",
					"Slate",
					"SlateCore",
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"BlueprintGraph",
					"AnimGraph",
					"AnimGraphRuntime",
					"BodyState",
					
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