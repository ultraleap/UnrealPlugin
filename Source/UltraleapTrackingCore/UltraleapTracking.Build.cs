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

        private void Setlib(string platformStr, string libStr)
        {
            PublicAdditionalLibraries.Add(Path.Combine(BinariesPath, platformStr, libStr));
            PublicDelayLoadDLLs.Add(Path.Combine(BinariesPath, platformStr, libStr));
            RuntimeDependencies.Add(Path.Combine(BinariesPath, platformStr, libStr));
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

            PublicIncludePathModuleNames.AddRange(new string[] { "Launch" });

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

        private void CopyToBinaries(string Filepath)
        {
            string binariesDir = Path.Combine(BinariesPath, Target.Platform.ToString());
            string filename = Path.GetFileName(Filepath);

            if (!Directory.Exists(binariesDir))
                Directory.CreateDirectory(binariesDir);

            if (!File.Exists(Path.Combine(binariesDir, filename)))
                File.Copy(Filepath, Path.Combine(binariesDir, filename), true);
        }

        public bool LoadLeapLib(ReadOnlyTargetRules Target)
		{
			bool IsLibrarySupported = false;

			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				IsLibrarySupported = true;

				string PlatformString = Target.Platform.ToString();

				string ThirdPartyDllPath = Path.Combine(LibraryPath, PlatformString, "LeapC.dll");
				string ThirdPartyDllManifPath = Path.Combine(LibraryPath, PlatformString, "LeapC.dll.manifest");
                string BinDLLPath = Path.Combine(BinariesPath, PlatformString, "LeapC.dll");
				string BinDLLManifPath = Path.Combine(BinariesPath, PlatformString, "LeapC.dll.manifest");
                //Lib
                PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));
                //System.Console.WriteLine("plugin using lib at " + Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));
                // Copy third party DLLs to the BinariesPath 
                RuntimeDependencies.Add(BinDLLPath, ThirdPartyDllPath);
                RuntimeDependencies.Add(BinDLLManifPath, ThirdPartyDllManifPath);

                // This will copy dlls if not copied already
                CopyToBinaries(ThirdPartyDllPath);
                CopyToBinaries(ThirdPartyDllManifPath);

                if (IsEnginePlugin())
				{
					PublicDelayLoadDLLs.Add("LeapC.dll");
                }
				//Engine plugin, just add the dependency path
				else
				{
					//DLL
					System.Console.WriteLine("Project plugin detected, using dll at " + BinDLLPath);

                    if (!Target.bBuildEditor)
					{
						PublicDelayLoadDLLs.Add("LeapC.dll");
					}
				}
			}
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				//IsLibrarySupported = true;	
                //Setlib("Mac", "libLeapC.5.dylib");
                //Setlib("Mac", "libLeapC.5_intel.dylib");
            }
			else if (Target.Platform == UnrealTargetPlatform.Android)
			{
				IsLibrarySupported = true;

				string PlatformString = "Android";

				PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "arm64-v8a", "libLeapC.so"));

				AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModulePath, "UltraleapTracking_APL.xml"));

                PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
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
