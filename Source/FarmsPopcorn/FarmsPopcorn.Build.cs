using UnrealBuildTool;

public class FarmsPopcorn : ModuleRules
{
	public FarmsPopcorn(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicIncludePaths.Add(ModuleDirectory);
		
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
            "OnlineSubsystem",          // 멀티플레이 세션용
            "OnlineSubsystemUtils"
        });
        PublicIncludePaths.AddRange(new string[]
        {
            "PopcornRace",  // 헤더 인클루드 경로
        });
    }
}
