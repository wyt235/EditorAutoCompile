using UnrealBuildTool;

public class EditorAssetAutoCompile : ModuleRules
{
    public EditorAssetAutoCompile(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "Kismet",
                "KismetCompiler",
                "BlueprintGraph"
            }
        );

        // For message dialogs / editor utilities
        PrivateDependencyModuleNames.AddRange(new string[] { "ApplicationCore" });

        // Editor-only module
        if (Target.bBuildEditor)
        {
            PrivateDefinitions.Add("WITH_EDITOR=1");
        }
    }
}
