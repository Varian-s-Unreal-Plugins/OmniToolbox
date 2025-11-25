using UnrealBuildTool;

public class OmniToolboxVanguard : ModuleRules
{
    public OmniToolboxVanguard(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "OmniToolbox",
                "FunctionalTesting",
                "DeveloperSettings",
                "UnrealEdMessages",
                "AutomationController",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                
            }
        );
        
        // if (Target.bBuildEditor)
        // {
        //     PrivateDependencyModuleNames.AddRange(new string[]
        //     {
        //         "UnrealEd", 
        //     });
        // }
    }
}