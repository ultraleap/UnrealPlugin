// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class LeapMotion : ModuleRules
	{
		private bool IsEnginePlugin()
		{
			return Path.GetFullPath(ModuleDirectory).EndsWith("UnrealEngine\\Engine\\Plugins\\Runtime\\LeapMotion\\Source\\LeapMotion");
		}

		private string ModulePath
		{
			get { return ModuleDirectory; }
		}

		private string EngineDirectory
		{
			get { return Path.GetFullPath("../"); }
		}

		private string ThirdPartyPath
		{
			get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
		}

		private string BinariesPath
		{
			get {
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
			get {
				if (IsEnginePlugin())
				{
					return Path.GetFullPath(Path.Combine(EngineDirectory, "Source/ThirdParty/Leap/Lib"));
				}
				else
				{
					return Path.GetFullPath(Path.Combine(ThirdPartyPath, "LeapSDK", "Lib"));
				}
			}
		}

		public LeapMotion(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicIncludePaths.AddRange(
				new string[] {
					"LeapMotion/Public",
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"LeapMotion/Private",
					Path.Combine(ThirdPartyPath, "LeapSDK", "Include"),
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

		private void CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
		{
			System.Console.WriteLine("uprojectpath is: " + Path.GetFullPath(GetUProjectPath()));

			string binariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
			string filename = Path.GetFileName(Filepath);

			//convert relative path
			string fullBinariesDir = Path.GetFullPath(binariesDir);

			if (!Directory.Exists(fullBinariesDir))
				Directory.CreateDirectory(fullBinariesDir);

			if (!File.Exists(Path.Combine(fullBinariesDir, filename)))
			{
				System.Console.WriteLine("LeapPlugin: Copied from " + Filepath + ", to " + Path.Combine(fullBinariesDir, filename));
				File.Copy(Filepath, Path.Combine(fullBinariesDir, filename), true);
			}
		}

		public bool LoadLeapLib(ReadOnlyTargetRules Target)
		{
			bool isLibrarySupported = false;

			if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
			{
				isLibrarySupported = true;

				string PlatformString = Target.Platform.ToString();

				//Lib
				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));

				System.Console.WriteLine("plugin using lib at " + Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));

				if (IsEnginePlugin())
				{
					string PluginDLLPath = Path.Combine(BinariesPath, PlatformString, "LeapC.dll");

					System.Console.WriteLine("Engine plugin detected, using dll at " + PluginDLLPath);

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
				isLibrarySupported = false;	//Not supported since Leap SDK 3.0

				string PlatformString = "Mac";
				PublicAdditionalLibraries.Add(Path.Combine(BinariesPath, PlatformString, "libLeap.dylib"));

			}
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
				isLibrarySupported = true;

				System.Console.WriteLine(Target.Architecture);    //doesn't work

				string PlatformString = "Android";

				//For now comment/uncomment platform architectures
				//PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "armeabi-v7a", "libLeapC.so"));
				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "arm64-v8a", "libLeapC.so"));

				AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "LeapMotion_APL.xml"));
			}

			return isLibrarySupported;
		}
	}
}