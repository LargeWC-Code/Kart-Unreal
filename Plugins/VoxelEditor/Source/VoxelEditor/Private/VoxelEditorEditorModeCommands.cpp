// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelEditorEditorModeCommands.h"
#include "VoxelEditorEditorMode.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "VoxelEditorEditorModeCommands"

FVoxelEditorEditorModeCommands::FVoxelEditorEditorModeCommands()
	: TCommands<FVoxelEditorEditorModeCommands>("VoxelEditorEditorMode",
		NSLOCTEXT("VoxelEditorEditorMode", "VoxelEditorEditorModeCommands", "VoxelEditor Editor Mode"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FVoxelEditorEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(MapsTool, "Maps", "Display all maps in the world", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(MapsTool);

	UI_COMMAND(EditTool, "Edit", "Edit voxel terrain and prefabs", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(EditTool);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FVoxelEditorEditorModeCommands::GetCommands()
{
	return FVoxelEditorEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
