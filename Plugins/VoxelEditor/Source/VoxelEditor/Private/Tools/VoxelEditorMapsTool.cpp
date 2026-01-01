// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelEditorMapsTool.h"
#include "VoxelEditorEditorMode.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Engine/HitResult.h"
#include "Misc/MessageDialog.h"
#include "VoxelWorldBase.h"
#include "VoxelWorldEditor.h"
#include "EngineUtils.h"

// localization namespace
#define LOCTEXT_NAMESPACE "VoxelEditorMapsTool"

/*
 * ToolBuilder implementation
 */

UInteractiveTool* UVoxelEditorMapsToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UVoxelEditorMapsTool* NewTool = NewObject<UVoxelEditorMapsTool>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	return NewTool;
}



/*
 * ToolProperties implementation
 */

UVoxelEditorMapsToolProperties::UVoxelEditorMapsToolProperties()
{
	ShowExtendedInfo = true;
}


/*
 * Tool implementation
 */

UVoxelEditorMapsTool::UVoxelEditorMapsTool()
{
}


void UVoxelEditorMapsTool::SetWorld(UWorld* World)
{
	this->TargetWorld = World;
}


void UVoxelEditorMapsTool::Setup()
{
	USingleClickTool::Setup();

	Properties = NewObject<UVoxelEditorMapsToolProperties>(this);
	AddToolPropertySource(Properties);
}


void UVoxelEditorMapsTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	// Maps tool: Find VoxelWorldEditor instance to mark map as loaded
	if (!TargetWorld)
	{
		return;
	}

#if WITH_EDITOR
	// Find VoxelWorldEditor instance in the world
	AVoxelWorldEditor* VoxelWorldEditor = nullptr;
	for (TActorIterator<AVoxelWorldEditor> ActorItr(TargetWorld); ActorItr; ++ActorItr)
	{
		VoxelWorldEditor = *ActorItr;
		if (VoxelWorldEditor)
		{
			break;
		}
	}

	if (VoxelWorldEditor)
	{
		// Mark map as loaded (enable Edit tool)
		UVoxelEditorEditorMode::SetMapLoaded(true);
	}
	else
	{
		// No VoxelWorldEditor found - mark map as not loaded (disable Edit tool)
		UVoxelEditorEditorMode::SetMapLoaded(false);
	}
#endif
}


#undef LOCTEXT_NAMESPACE




