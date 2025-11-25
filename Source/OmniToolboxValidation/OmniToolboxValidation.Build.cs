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
                "DataValidation",
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
                "UnrealEd"
            }
        );
    }
}