// Copyright (C) Varian Daemon 2025. All Rights Reserved.

using UnrealBuildTool;
using System.IO;       // Required for Directory and Path

public class OmniToolbox : ModuleRules
{
    public OmniToolbox(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrecompileForTargets = PrecompileTargetsType.Any;
        
        if(Plugins.GetPlugin("ObjectTags") != null)
        {
            PublicDefinitions.Add("ObjectTags_Enabled=1");
            PublicDependencyModuleNames.Add("ObjectTags");
        }
        else
        {
            PublicDefinitions.Add("ObjectTags_Enabled=0");
        }

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "GameFeatures",
                //Uncomment if you want a scalable float provider
                //(Check inside FloatProvider.h -> FScalableFloatProvider)
                // "GameplayAbilities", 
                "UMG",
                "Slate",
                "SlateCore", 
                "GameFeatures"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "UMG",
                "Slate",
                "SlateCore", 
                "GameplayTags",
                "Projects",
            }
        );
        
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd", 
                "GameplayTagsEditor",
                "ToolMenus",
                "PlacementMode", 
            });
        }
    }
}