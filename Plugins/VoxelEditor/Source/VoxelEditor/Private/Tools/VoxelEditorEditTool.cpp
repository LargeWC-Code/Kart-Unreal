// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelEditorEditTool.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"
#include "BaseBehaviors/ClickDragBehavior.h"
#include "VoxelEditorEditorMode.h"
#include "VoxelEditorEditorModeToolkit.h"
#include "VoxelWorldEditor.h"
#include "VoxelTerrain.h"
#include "VoxelTile.h"
#include "VoxelMap.h"
#include "VoxelBlockTypes.h"
#include "VoxelEditVolume.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"

// for raycast into World
#include "Engine/World.h"
#include "EngineUtils.h"
#include "SceneManagement.h"

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
	bHasPendingPlacement = false;
	PendingPlacementPos = FIntVector::ZeroValue;
	CurrentBlockType = VOXEL_BLOCK_TYPE_SELECT;
	bIsDragging = false;
	DragStartPos = FIntVector::ZeroValue;
	DragStartHitPos = FIntVector::ZeroValue;
	DragStartHitNormal = FIntVector::ZeroValue;
	bHasDragStartPlane = false;
	DragEndPos = FIntVector::ZeroValue;
	bHasSelectedRegion = false;
	SelectedMinPos = FIntVector::ZeroValue;
	SelectedMaxPos = FIntVector::ZeroValue;
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
	
	// Get the selected block type from toolkit
	int32 BlockType = VOXEL_BLOCK_TYPE_SELECT;
	TSharedPtr<FVoxelEditorEditorModeToolkit> Toolkit = VoxelMode->GetToolkit();
	if (Toolkit.IsValid())
	{
		BlockType = Toolkit->GetSelectedBlockType();
	}
	
	// Store current block type
	CurrentBlockType = BlockType;
	
	// Handle different block types
	if (BlockType == VOXEL_BLOCK_TYPE_SELECT)
	{
		// Start drag selection: store start position and clear previous selection
		DragStartPos = HitVoxelPos;
		DragEndPos = HitVoxelPos;
		bIsDragging = true;
		bHasSelectedRegion = false; // Clear previous selection when starting new drag
		bHasPendingPlacement = false;
		
		// Store the initial hit position and normal for plane-based drag calculation
		// 保存第一个点击到的盒子的碰撞面，拖动过程中使用这个面来判断碰撞，不再检测新的碰撞盒子
		// 使用体素坐标位置和整数法线方向
		DragStartHitPos = HitVoxelPos;
		DragStartHitNormal = FIntVector(
			FMath::RoundToInt(HitNormal.X),
			FMath::RoundToInt(HitNormal.Y),
			FMath::RoundToInt(HitNormal.Z)
		);
		bHasDragStartPlane = true;
		
		UE_LOG(LogTemp, Log, TEXT("Started drag selection at: (%d, %d, %d)"), HitVoxelPos.X, HitVoxelPos.Y, HitVoxelPos.Z);
	}
	else if (BlockType == VOXEL_BLOCK_TYPE_PLACE)
	{
		// Check Shift key state for deletion (immediate deletion on press)
		bool bDelete = false;
		if (FSlateApplication::IsInitialized())
		{
			FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();
			bDelete = ModifierKeys.IsShiftDown();
		}
		
		if (bDelete)
		{
			// Delete immediately on press (Shift key)
			Terrain->SetVoxelAtWorldPosition(HitVoxelPos, 0, 0, TargetWorld);
			bHasPendingPlacement = false;
		}
		else
		{
			// For placement: show preview but don't place yet (will place on release)
			// Calculate the adjacent voxel position in the direction of the normal
			FIntVector VoxelWorldPos = HitVoxelPos;
			VoxelWorldPos.X += FMath::RoundToInt(HitNormal.X);
			VoxelWorldPos.Y += FMath::RoundToInt(HitNormal.Y);
			VoxelWorldPos.Z += FMath::RoundToInt(HitNormal.Z);
			
			// Store the preview position to show yellow box
			PendingPlacementPos = VoxelWorldPos;
			bHasPendingPlacement = true;
		}
	}
	// TODO: Handle "摆放方斜面" and "摆放三角斜面" in the future
}

void UVoxelEditorEditTool::OnClickDrag(const FInputDeviceRay& DragPos)
{
	// Update drag end position if we're dragging
	if (bIsDragging && CurrentBlockType == VOXEL_BLOCK_TYPE_SELECT)
	{
		// Get the editor mode to access VoxelWorldEditor
		UVoxelEditorEditorMode* VoxelMode = UVoxelEditorEditorMode::GetActiveEditorMode();
		if (!VoxelMode)
		{
			return;
		}

		// Get VoxelWorldEditor instance
		AVoxelWorldEditor* WorldEditor = VoxelMode->GetVoxelWorldEditorInstance();
		if (!WorldEditor || !IsValid(WorldEditor))
		{
			return;
		}

		// Get Terrain
		UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
		if (!Terrain)
		{
			return;
		}

		// 使用第一个点击盒子的碰撞面来判断碰撞，不再检测新的碰撞盒子
		if (bHasDragStartPlane)
		{
			// 计算射线与保存的碰撞面的交点
			// 平面方程: Normal · (P - PointOnPlane) = 0
			// 射线方程: P = Origin + t * Direction
			// 求解 t: t = -Normal · (Origin - PointOnPlane) / (Normal · Direction)
			const float VoxelSize = Terrain->VoxelSize;
			const int32 HalfTileSizeX = VOXEL_TILE_SIZE_X / 2;
			const int32 HalfTileSizeY = VOXEL_TILE_SIZE_Y / 2;
			const int32 HalfTileSizeZ = VOXEL_TILE_SIZE_Z / 2;
			
			// 将体素坐标转换为世界坐标
			FVector PointOnPlane = FVector(
				(DragStartHitPos.X - HalfTileSizeX) * VoxelSize,
				(DragStartHitPos.Y - HalfTileSizeY) * VoxelSize,
				(DragStartHitPos.Z - HalfTileSizeZ) * VoxelSize
			);
			
			// 将整数法线转换为向量
			FVector PlaneNormal = FVector(
				(float)DragStartHitNormal.X,
				(float)DragStartHitNormal.Y,
				(float)DragStartHitNormal.Z
			).GetSafeNormal();
			
			FVector RayOrigin = DragPos.WorldRay.Origin;
			FVector RayDirection = DragPos.WorldRay.Direction;
			
			float Denom = FVector::DotProduct(PlaneNormal, RayDirection);
			
			// 检查射线是否与平面平行（分母接近零）
			if (FMath::Abs(Denom) > 0.0001f)
			{
				FVector OriginToPlane = RayOrigin - PointOnPlane;
				float T = -FVector::DotProduct(PlaneNormal, OriginToPlane) / Denom;
				
				// 只考虑正向的交点
				if (T >= 0.0f)
				{
					FVector IntersectionPoint = RayOrigin + RayDirection * T;
					
					// 将世界坐标转换为voxel坐标
					// 根据 VoxelCoordsToWorldBox 的反向转换公式: VoxelPos = WorldPos / VoxelSize + VOXEL_TILE_SIZE / 2
					DragEndPos = FIntVector(
						FMath::RoundToInt((IntersectionPoint.X) / VoxelSize + HalfTileSizeX),
						FMath::RoundToInt((IntersectionPoint.Y) / VoxelSize + HalfTileSizeY),
						FMath::RoundToInt((IntersectionPoint.Z) / VoxelSize + HalfTileSizeZ)
					);
					UE_LOG(LogTemp, VeryVerbose, TEXT("Drag end position (plane): (%d, %d, %d)"), DragEndPos.X, DragEndPos.Y, DragEndPos.Z);
					
					// 更新 VoxelEditVolume 的位置和大小以匹配黄色框
					UpdateVoxelEditVolume(WorldEditor->GetWorld(), Terrain);
				}
			}
		}
	}
}

void UVoxelEditorEditTool::OnClickRelease(const FInputDeviceRay& ReleasePos)
{
	// Place the pending voxel if we have one
	if (bHasPendingPlacement && CurrentBlockType == VOXEL_BLOCK_TYPE_PLACE)
	{
		// Get the editor mode to access VoxelWorldEditor
		UVoxelEditorEditorMode* VoxelMode = UVoxelEditorEditorMode::GetActiveEditorMode();
		if (!VoxelMode)
		{
			return;
		}

		// Get VoxelWorldEditor instance
		AVoxelWorldEditor* WorldEditor = VoxelMode->GetVoxelWorldEditorInstance();
		if (!WorldEditor || !IsValid(WorldEditor))
		{
			return;
		}

		// Get Terrain
		UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
		if (!Terrain)
		{
			return;
		}

		// Place the voxel
		Terrain->SetVoxelAtWorldPosition(PendingPlacementPos, 0, 1, TargetWorld);
		UE_LOG(LogTemp, Log, TEXT("Placed voxel at: (%d, %d, %d)"), PendingPlacementPos.X, PendingPlacementPos.Y, PendingPlacementPos.Z);
		
		// Keep the selection to show the placed voxel
		// bHasPendingPlacement is cleared but bHasSelectedVoxel remains true
		bHasPendingPlacement = false;
	}
	
	// End drag selection and save the selected region
	if (bIsDragging && CurrentBlockType == VOXEL_BLOCK_TYPE_SELECT)
	{
		// Calculate and save the selected region
		SelectedMinPos = FIntVector(
			FMath::Min(DragStartPos.X, DragEndPos.X),
			FMath::Min(DragStartPos.Y, DragEndPos.Y),
			FMath::Min(DragStartPos.Z, DragEndPos.Z)
		);
		SelectedMaxPos = FIntVector(
			FMath::Max(DragStartPos.X, DragEndPos.X),
			FMath::Max(DragStartPos.Y, DragEndPos.Y),
			FMath::Max(DragStartPos.Z, DragEndPos.Z)
		);
		bHasSelectedRegion = true;
		bIsDragging = false;
		UE_LOG(LogTemp, Log, TEXT("Ended drag selection from (%d, %d, %d) to (%d, %d, %d)"), 
			DragStartPos.X, DragStartPos.Y, DragStartPos.Z,
			DragEndPos.X, DragEndPos.Y, DragEndPos.Z);
	}
}

void UVoxelEditorEditTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
	// Property modifications are not used for voxel editing
}

TTuple<FVector, FVector> UVoxelEditorEditTool::VoxelCoordsToWorldBox(const FIntVector& MinPos, const FIntVector& MaxPos, float VoxelSize) const
{
	FVector BoxMin = FVector(
		(MinPos.X - VOXEL_TILE_SIZE_X / 2) * VoxelSize,
		(MinPos.Y - VOXEL_TILE_SIZE_Y / 2) * VoxelSize,
		(MinPos.Z - VOXEL_TILE_SIZE_Z / 2) * VoxelSize
	);
	FVector BoxMax = FVector(
		(MaxPos.X + 1 - VOXEL_TILE_SIZE_X / 2) * VoxelSize,  // +1 to include the last voxel
		(MaxPos.Y + 1 - VOXEL_TILE_SIZE_Y / 2) * VoxelSize,
		(MaxPos.Z + 1 - VOXEL_TILE_SIZE_Z / 2) * VoxelSize
	);
	return TTuple<FVector, FVector>(BoxMin, BoxMax);
}

void UVoxelEditorEditTool::UpdateVoxelEditVolume(UWorld* World, UVoxelTerrain* Terrain)
{
	if (!World || !Terrain)
	{
		return;
	}
	
	// 查找 VoxelEditVolume
	AVoxelEditVolume* EditVolume = nullptr;
	for (TActorIterator<AVoxelEditVolume> ActorItr(World); ActorItr; ++ActorItr)
	{
		AVoxelEditVolume* Volume = *ActorItr;
		if (IsValid(Volume) && Volume->BoxComponent)
		{
			EditVolume = Volume;
			break;
		}
	}
	
	if (!EditVolume || !EditVolume->BoxComponent)
	{
		return;
	}
	
	// 计算拖拽选择框的最小和最大位置
	FIntVector MinPos(
		FMath::Min(DragStartPos.X, DragEndPos.X),
		FMath::Min(DragStartPos.Y, DragEndPos.Y),
		FMath::Min(DragStartPos.Z, DragEndPos.Z)
	);
	FIntVector MaxPos(
		FMath::Max(DragStartPos.X, DragEndPos.X),
		FMath::Max(DragStartPos.Y, DragEndPos.Y),
		FMath::Max(DragStartPos.Z, DragEndPos.Z)
	);
	
	// 将 voxel 坐标转换为世界坐标盒子
	auto [BoxMin, BoxMax] = this->VoxelCoordsToWorldBox(MinPos, MaxPos, Terrain->VoxelSize);
	
	// 计算 VoxelEditVolume 的位置（盒子中心）
	FVector VolumePosition = (BoxMin + BoxMax) * 0.5f;
	
	// 计算 VoxelEditVolume 的大小（SetBoxExtent 使用半尺寸）
	FVector BoxExtent = (BoxMax - BoxMin) * 0.5f;
	
	// 更新位置和大小
	EditVolume->SetActorLocation(VolumePosition);
	EditVolume->BoxComponent->SetBoxExtent(BoxExtent);
}

void UVoxelEditorEditTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	// 不再绘制黄色线框
}


#undef LOCTEXT_NAMESPACE



