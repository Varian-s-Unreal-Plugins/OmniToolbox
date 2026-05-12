using UnrealBuildTool;

public class OmniToolboxValidation : ModuleRules
{
    public OmniToolboxValidation(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "DeveloperSettings",
                "BlueprintGraph",
                "Kismet",
            }
        );
        
        if(Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "DataValidation",
                "UnrealEd",
            });
        }
    }
}