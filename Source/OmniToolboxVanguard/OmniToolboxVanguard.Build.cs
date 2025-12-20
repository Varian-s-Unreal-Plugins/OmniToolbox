using UnrealBuildTool;

public class OmniToolboxVanguard : ModuleRules
{
    public OmniToolboxVanguard(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrecompileForTargets = PrecompileTargetsType.Any;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "OmniToolbox",
                "DeveloperSettings",
                // "AutomationController",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "TraceUtilities",
            }
        );
        
        if (Target.Configuration != UnrealTargetConfiguration.Shipping || Target.bBuildEditor)
        {
            PublicDefinitions.Add("WITH_INSIGHTS_TRACE=1");
        }
        else
        {
            PublicDefinitions.Add("WITH_INSIGHTS_TRACE=0");
        }

        if(Target.bBuildDeveloperTools)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "FunctionalTesting", 
            });
            
            PublicDefinitions.Add("FunctionalTestingEnabled=1");
        }
        else
        {
            PublicDefinitions.Add("FunctionalTestingEnabled=0");
        }
        
        if(Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd", 
            });
        }
    }
}