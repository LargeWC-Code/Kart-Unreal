// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelEditorModule.h"
#include "VoxelEditorEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "VoxelEditorModule"

void FVoxelEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FVoxelEditorEditorModeCommands::Register();
}

void FVoxelEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FVoxelEditorEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVoxelEditorModule, VoxelEditor)