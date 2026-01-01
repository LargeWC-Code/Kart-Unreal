// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MagicXCore : ModuleRules
{
	public MagicXCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public"),
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"RHI",
				"Renderer",
				"UMG",
				"InputCore",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"ApplicationCore",
				"ImageWrapper",
				// ... add private dependencies that you statically link with here ...	
			}
			);

		// MagicX SDK 路径
		string MagicXSDKPath = System.Environment.GetEnvironmentVariable("MAGICX_SDK");
		if (!string.IsNullOrEmpty(MagicXSDKPath))
		{
			PublicIncludePaths.Add(Path.Combine(MagicXSDKPath, "winexe", "include"));

			if (Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				PublicDefinitions.Add("UNREAL_DEBUG=1");
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "ucoredu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "ucscriptdu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "uccompiledu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "ucsharpdu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "ucgeometrydu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "uc3ddu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "ucnetbasedu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "ucuidu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "ucdevicedu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "DebugU64", "ucgamebasedu64.lib"));
			}
			if (Target.Configuration == UnrealTargetConfiguration.Development || Target.Configuration == UnrealTargetConfiguration.Test || Target.Configuration == UnrealTargetConfiguration.Shipping)
			{
				PublicDefinitions.Add("UNREAL_DEBUG=0");
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "ucoreu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "ucscriptu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "uccompileu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "ucsharpu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "ucgeometryu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "uc3du64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "ucnetbaseu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "ucuiu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "ucdeviceu64.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(MagicXSDKPath, "winexe", "lib", "PublishU64", "ucgamebaseu64.lib"));
			}
		}
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

