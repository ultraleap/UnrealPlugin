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
	public class UltraleapTracking : ModuleRules
	{
		private bool IsEnginePlugin()
		{
			return Path.GetFullPath(ModuleDirectory).EndsWith("Engine\\Plugins\\Runtime\\UltraleapTracking\\Source\\UltraleapTracking");
		}

		private string ModulePath
		{
			get { return ModuleDirectory; }
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

		private string BinariesPath
		{
			get
			{
				if (IsEnginePlugin())
				{
					return Path.GetFullPath(Path.Combine(EngineDirectory, "Binaries/ThirdParty/UltraleapTracking"));
				}
				else
				{ 
					return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/"));
				}
			}
		}

		private string LibraryPath
		{
			get
			{
				if (IsEnginePlugin())
				{
					return Path.GetFullPath(Path.Combine(ThirdPartyPath, "Leap/Lib"));
				}
				else
				{
					return Path.GetFullPath(Path.Combine(ThirdPartyPath, "LeapSDK/Lib"));
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

		public UltraleapTracking(ReadOnlyTargetRules Target) : base(Target)
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
					IncludePath,
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "ApplicationCore",
                    "Engine",
					"Core",
					"CoreUObject",
					"InputCore",
					"InputDevice",
					"Slate",
					"SlateCore",
					"HeadMountedDisplay",
					"RHI",
					"RenderCore",
					"Projects",
					"LiveLinkInterface",
					"LiveLinkMessageBusFramework",
					"BodyState",
					"PhysicsCore",
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);

			LoadLeapLib(Target);
		}

		public string GetUProjectPath()
		{
			return Path.Combine(ModuleDirectory, "../../../..");
		}

		private int HashFile(string FilePath)
		{
			string DLLString = File.ReadAllText(FilePath);
			return DLLString.GetHashCode() + DLLString.Length;	//ensure both hash and file lengths match
		}


		public bool LoadLeapLib(ReadOnlyTargetRules Target)
		{
			bool IsLibrarySupported = false;

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				IsLibrarySupported = true;

				string PlatformString = Target.Platform.ToString();

				//Lib
				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));

				//System.Console.WriteLine("plugin using lib at " + Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));

				if (IsEnginePlugin())
				{
					PublicDelayLoadDLLs.Add("LeapC.dll");
					RuntimeDependencies.Add(Path.Combine(BinariesPath, PlatformString, "LeapC.dll"));
				}
				//Engine plugin, just add the dependency path
				else
				{
					//DLL
					string PluginDLLPath = Path.Combine(BinariesPath, PlatformString, "LeapC.dll");
				
					System.Console.WriteLine("Project plugin detected, using dll at " + PluginDLLPath);

					RuntimeDependencies.Add(PluginDLLPath);
					if (!Target.bBuildEditor)
					{
						PublicDelayLoadDLLs.Add("LeapC.dll");
					}
				}
			}
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				IsLibrarySupported = false;	//Not supported since Leap SDK 3.0

				string PlatformString = "Mac";
				PublicAdditionalLibraries.Add(Path.Combine(BinariesPath, PlatformString, "libLeap.dylib"));

			}
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
				IsLibrarySupported = true;

				string PlatformString = "Android";

				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "arm64-v8a", "libLeapC.so"));

				AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "UltraleapTracking_APL.xml"));
			}
			else if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				IsLibrarySupported = true;

				string PlatformString = "Linux";

				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "x64", "libLeapC.so"));
			}

			return IsLibrarySupported;
		}
	}
}