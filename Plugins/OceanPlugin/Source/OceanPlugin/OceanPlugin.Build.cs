// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OceanPlugin : ModuleRules
{
	public OceanPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateIncludePaths.AddRange(new string[]
        		{
        			"OceanPlugin/Private"
        		});
        
        if (Target.bBuildEditor == true)
        {
        	PrivateDependencyModuleNames.Add("TargetPlatform");
        }
        PublicDependencyModuleNames.Add("Core");
        PublicDependencyModuleNames.Add("Engine");
        PublicDependencyModuleNames.Add("EnhancedInput");
        PublicDependencyModuleNames.Add("MaterialShaderQualitySettings");
        
        PrivateDependencyModuleNames.AddRange(new string[]
        {
	        "EnhancedInput",
        	"CoreUObject",
        	"Renderer",
        	"RenderCore",
        	"RHI",
        	"Projects"
        });
        
        if (Target.bBuildEditor == true)
        {
        
        	PrivateDependencyModuleNames.AddRange(
        		new string[] {
			        "EnhancedInput",
        			"UnrealEd",
        			"MaterialUtilities",
        			"SlateCore",
        			"Slate"
        		}
        	);
        }
	}
}
