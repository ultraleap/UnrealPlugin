// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class LeapMotion : ModuleRules
	{
		private bool IsEnginePlugin()
		{
			return Path.GetFullPath(ModuleDirectory).EndsWith("Engine\\Plugins\\Runtime\\LeapMotion\\Source\\LeapMotion");
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
					return Path.GetFullPath(Path.Combine(EngineDirectory, "Binaries/ThirdParty/LeapMotion"));
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

		public LeapMotion(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"LeapMotion/Private",
					IncludePath,
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
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
					"BodyState"
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

		private void CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
		{
			//System.Console.WriteLine("uprojectpath is: " + Path.GetFullPath(GetUProjectPath()));

			string BinariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
			string Filename = Path.GetFileName(Filepath);

			//convert relative path 
			string FullBinariesDir = Path.GetFullPath(BinariesDir);

			if (!Directory.Exists(FullBinariesDir))
			{
				Directory.CreateDirectory(FullBinariesDir);
			}

			string FullExistingPath = Path.Combine(FullBinariesDir, Filename);
			bool ValidFile = false;

			//File exists, check if they're the same
			if (File.Exists(FullExistingPath))
			{
				int ExistingFileHash = HashFile(FullExistingPath);
				int TargetFileHash = HashFile(Filepath);
				ValidFile = ExistingFileHash == TargetFileHash;
				if (!ValidFile)
				{
					System.Console.WriteLine("LeapPlugin: outdated dll detected.");
				}
			}

			//No valid existing file found, copy new dll
			if(!ValidFile)
			{
				System.Console.WriteLine("LeapPlugin: Copied from " + Filepath + ", to " + Path.Combine(FullBinariesDir, Filename));
				File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
			}
		}

		public bool LoadLeapLib(ReadOnlyTargetRules Target)
		{
			bool IsLibrarySupported = false;

			if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
			{
				IsLibrarySupported = true;

				string PlatformString = Target.Platform.ToString();

				//Lib
				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));

				//System.Console.WriteLine("plugin using lib at " + Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));

				if (IsEnginePlugin())
				{
					//string PluginDLLPath = Path.Combine(BinariesPath, PlatformString, "LeapC.dll");

					//System.Console.WriteLine("Engine plugin detected, using dll at " + PluginDLLPath);

					PublicDelayLoadDLLs.Add("LeapC.dll");
					//RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/LeapMotion/" + PlatformString + "/LeapC.dll");
					RuntimeDependencies.Add(Path.Combine(BinariesPath, PlatformString, "LeapC.dll"));
				}
				//Engine plugin, just add the dependency path
				else
				{
					//DLL
					string PluginDLLPath = Path.Combine(BinariesPath, PlatformString, "LeapC.dll");

					System.Console.WriteLine("Project plugin detected, using dll at " + PluginDLLPath);

					//For project plugins, copy the dll to the project if needed
					CopyToProjectBinaries(PluginDLLPath, Target);

					string DLLPath = Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", PlatformString, "LeapC.dll"));
					RuntimeDependencies.Add(DLLPath);
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

				System.Console.WriteLine(Target.Architecture);    //doesn't work

				string PlatformString = "Android";

				//For now comment/uncomment platform architectures
				//PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "armeabi-v7a", "libLeapC.so"));
				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "arm64-v8a", "libLeapC.so"));

				AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "LeapMotion_APL.xml"));
			}

			return IsLibrarySupported;
		}
	}
}