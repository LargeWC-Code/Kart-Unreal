// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;  // ������һ��

public class KartGame : ModuleRules
{
    public KartGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "ChaosVehicles",
            "PhysicsCore",
            "UMG",
			"Slate",
			"SlateCore",
			"ApplicationCore",
            "RenderCore",
            "RHI",
            "Renderer"
        });

        PublicIncludePaths.AddRange(new string[] {
            "KartGame",
            "KartGame/KartCar"
        });

        // MagicX SDK 相关配置已移至 MagicXCore 插件
        PublicDependencyModuleNames.AddRange(new string[] {
            "MagicXCore",
            "VoxelCore"  // Added to access VoxelWorldBase for VoxelWorldGame
        });
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
