// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelEditorEditTool.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"
#include "BaseBehaviors/ClickDragBehavior.h"
#include "VoxelEditorEditorMode.h"
#include "VoxelWorldEditor.h"
#include "VoxelTerrain.h"
#include "VoxelTile.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"

// for raycast into World
#include "Engine/World.h"
#include "EngineUtils.h"

// localization namespace
#define LOCTEXT_NAMESPACE "UVoxelEditorEditTool"

/*
 * ToolBuilder
 */

UInteractiveTool* UVoxelEditorEditToolBuilder::BuildTool(const FToolBuilderState & SceneState) const
{
	UVoxelEditorEditTool* NewTool = NewObject<UVoxelEditorEditTool>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	return NewTool;
}


// JAH TODO: update comments
/*
 * Tool
 */

UVoxelEditorEditToolProperties::UVoxelEditorEditToolProperties()
{
	// 保留这些属性用于兼容性，但不再使用
	StartPoint = FVector(0,0,0);
	EndPoint = FVector(0,0,100);
	Distance = 100;
}


void UVoxelEditorEditTool::SetWorld(UWorld* World)
{
	check(World);
	this->TargetWorld = World;
}


void UVoxelEditorEditTool::Setup()
{
	UInteractiveTool::Setup();

	// Add default mouse input behavior
	UClickDragInputBehavior* MouseBehavior = NewObject<UClickDragInputBehavior>();
	// Register Shift key modifier for deletion
	MouseBehavior->Modifiers.RegisterModifier(ShiftKeyModifierID, FInputDeviceState::IsShiftKeyDown);
	MouseBehavior->Initialize(this);
	AddInputBehavior(MouseBehavior);

	// Create the property set and register it with the Tool
	Properties = NewObject<UVoxelEditorEditToolProperties>(this, "VoxelEditToolProperties");
	AddToolPropertySource(Properties);
	
	bShiftKeyDown = false;
}


void UVoxelEditorEditTool::OnUpdateModifierState(int ModifierID, bool bIsOn)
{
	// keep track of the Shift key modifier
	if (ModifierID == ShiftKeyModifierID)
	{
		bShiftKeyDown = bIsOn;
	}
}


FInputRayHit UVoxelEditorEditTool::CanBeginClickDragSequence(const FInputDeviceRay& PressPos)
{
	// Get the editor mode to access VoxelWorldEditor
	UVoxelEditorEditorMode* VoxelMode = UVoxelEditorEditorMode::GetActiveEditorMode();
	if (!VoxelMode)
	{
		return FInputRayHit();
	}

	// Get VoxelWorldEditor instance
	AVoxelWorldEditor* WorldEditor = VoxelMode->GetVoxelWorldEditorInstance();
	if (!WorldEditor || !IsValid(WorldEditor))
	{
		return FInputRayHit();
	}

	// Get Terrain
	UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
	if (!Terrain)
	{
		return FInputRayHit();
	}

// 	// Use Terrain::Intersect to check if we can hit
	FIntVector HitVoxelPos;
	FVector HitPos;
	FVector HitNormal;
	if (Terrain->Intersect(PressPos.WorldRay.Origin, PressPos.WorldRay.Direction, HitVoxelPos, HitPos, HitNormal))
	{
		float HitDist = (FVector(HitPos.X, HitPos.Y, HitPos.Z) - PressPos.WorldRay.Origin).Size();
		return FInputRayHit(HitDist);
	}
	
	return FInputRayHit();
}


void UVoxelEditorEditTool::OnClickPress(const FInputDeviceRay& PressPos)
{
	// Get the editor mode to access VoxelWorldEditor
	UVoxelEditorEditorMode* VoxelMode = UVoxelEditorEditorMode::GetActiveEditorMode();
	if (!VoxelMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("VoxelEditorEditTool: Cannot get active VoxelEditorEditorMode"));
		return;
	}

	// Get VoxelWorldEditor instance
	AVoxelWorldEditor* WorldEditor = VoxelMode->GetVoxelWorldEditorInstance();
	if (!WorldEditor || !IsValid(WorldEditor))
	{
		UE_LOG(LogTemp, Warning, TEXT("VoxelEditorEditTool: VoxelWorldEditor instance is not valid"));
		return;
	}

	// Get Terrain
	UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
	if (!Terrain)
	{
		Terrain = WorldEditor->CreateTerrain();
	}

	if (!Terrain)
	{
		UE_LOG(LogTemp, Warning, TEXT("VoxelEditorEditTool: Failed to get or create Terrain"));
		return;
	}

	// Use Terrain::Intersect to find hit
	FIntVector HitVoxelPos;
	FVector HitPos;
	FVector HitNormal;
	if (!Terrain->Intersect(PressPos.WorldRay.Origin, PressPos.WorldRay.Direction, HitVoxelPos, HitPos, HitNormal))
	{
		return; // No hit found
	}
	
	// Handle voxel placement or deletion based on Shift key state
	// Check Shift key state directly at click time for reliability
	bool bDelete = false;
	if (FSlateApplication::IsInitialized())
	{
		FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();
		bDelete = ModifierKeys.IsShiftDown();
	}
	FIntVector VoxelWorldPos = HitVoxelPos;
	if (!bDelete)
	{
		// For placement, move to the adjacent voxel in the direction of the normal
		FVector AbsNormal = HitNormal.GetAbs();

		// Calculate world position of the adjacent voxel center
		VoxelWorldPos.X += FMath::RoundToInt(HitNormal.X);
		VoxelWorldPos.Y += FMath::RoundToInt(HitNormal.Y);
		VoxelWorldPos.Z += FMath::RoundToInt(HitNormal.Z);
	}

	// Set or delete the voxel (0 = delete, 1 = place)
	uint8 VoxelType = bDelete ? 0 : 1;
	Terrain->SetVoxelAtWorldPosition(VoxelWorldPos, VoxelType, 0, TargetWorld);
}

void UVoxelEditorEditTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
	// Property modifications are not used for voxel editing
}


void UVoxelEditorEditTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	// 禁用测量线条的绘制，Edit工具现在只用于UI操作，不需要可视化线条
	// FPrimitiveDrawInterface* PDI = RenderAPI->GetPrimitiveDrawInterface();
	// draw a thin line that shows through objects
	// PDI->DrawLine(Properties->StartPoint, Properties->EndPoint,
	// 	FColor(240, 16, 16), SDPG_Foreground, 2.0f, 0.0f, true);
	// draw a thicker line that is depth-tested
	// PDI->DrawLine(Properties->StartPoint, Properties->EndPoint,
	// 	FColor(240, 16, 16), SDPG_World, 4.0f, 0.0f, true);
}


#undef LOCTEXT_NAMESPACE



