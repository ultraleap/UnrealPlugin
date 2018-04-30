// Some copyright should be here...

using UnrealBuildTool;

public class BodyState : ModuleRules
{
	public BodyState(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				"BodyState/Public"
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"BodyState/Private",
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"AnimGraphRuntime",
				"InputCore",
				"InputDevice",
				"HeadMountedDisplay",
				"Slate",
				"SlateCore"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
