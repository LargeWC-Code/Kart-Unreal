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
#include "InputCoreTypes.h"
#include "Input/Events.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformApplicationMisc.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

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
	bSpaceKeyPressed = false;
	bDeleteKeyPressed = false;
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
		DragEndPos = HitVoxelPos;
		DragStartHitNormal = FIntVector(
			FMath::RoundToInt(HitNormal.X),
			FMath::RoundToInt(HitNormal.Y),
			FMath::RoundToInt(HitNormal.Z)
		);
		bHasDragStartPlane = true;
		
		// 更新 VoxelEditVolume 的位置和大小以匹配选择框
		UpdateVoxelEditVolume(WorldEditor->GetWorld(), Terrain);
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
			Terrain->SetVoxelAtWorldPosition(HitVoxelPos, 0, 0, 0, 0, 0, TargetWorld);
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
	else if (BlockType == VOXEL_BLOCK_TYPE_PLACE_SQUARE_SLOPE || BlockType == VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_SLOPE || BlockType == VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_COMPLEMENT)
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
			Terrain->SetVoxelAtWorldPosition(HitVoxelPos, 0, 0, 0, 0, 0, TargetWorld);
			bHasPendingPlacement = false;
		}
		else
		{
			// For slope placement: show preview but don't place yet (will place on release)
			// Calculate the adjacent voxel position in the direction of the normal
			FIntVector VoxelWorldPos = HitVoxelPos;
			VoxelWorldPos.X += FMath::RoundToInt(HitNormal.X);
			VoxelWorldPos.Y += FMath::RoundToInt(HitNormal.Y);
			VoxelWorldPos.Z += FMath::RoundToInt(HitNormal.Z);
			
			// Store the preview position, hit normal (bottom face), and ray direction (front face direction) for rotation calculation
			PendingPlacementPos = VoxelWorldPos;
			PendingPlacementHitNormal = HitNormal;
			PendingPlacementRayDirection = PressPos.WorldRay.Direction.GetSafeNormal();
			bHasPendingPlacement = true;
		}
	}
}

void UVoxelEditorEditTool::OnClickDrag(const FInputDeviceRay& DragPos)
{
	// Update drag end position if we're dragging
	if (bIsDragging && CurrentBlockType == VOXEL_BLOCK_TYPE_SELECT)
	{
		// Get the editor mode to access VoxelWorldEditor
		UVoxelEditorEditorMode* VoxelMode = UVoxelEditorEditorMode::GetActiveEditorMode();
		if (!VoxelMode)
			return;

		AVoxelWorldEditor* WorldEditor = VoxelMode->GetVoxelWorldEditorInstance();
		if (!WorldEditor || !IsValid(WorldEditor))
			return;

		UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
		if (!Terrain)
			return;

		// 使用第一个点击盒子的碰撞面来判断碰撞，不再检测新的碰撞盒子
		if (bHasDragStartPlane)
		{
			// 检查是否按Ctrl键（高度编辑模式，需要已有选择区域）
			FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();
			bool bCtrlDown = ModifierKeys.IsControlDown();

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
			FVector RayDirection = DragPos.WorldRay.Direction.GetSafeNormal(); // 确保归一化
			// 检查射线是否与平面平行
			float Denom = FVector::DotProduct(PlaneNormal, RayDirection);
			if (FMath::Abs(Denom) <= 0.0001f)
				return;

			int32 PlaneNormalX = ucMAX(DragStartHitNormal.X, 0);
			int32 PlaneNormalY = ucMAX(DragStartHitNormal.Y, 0);
			int32 PlaneNormalZ = ucMAX(DragStartHitNormal.Z, 0);
			// 使用Unreal的API计算射线与平面的交点
			FPlane Plane(PointOnPlane + FVector(PlaneNormalX, PlaneNormalY, PlaneNormalZ) * VoxelSize, PlaneNormal);

			// 计算交点
			FVector IntersectionPoint = FMath::RayPlaneIntersection(RayOrigin, RayDirection, Plane);

			// 检查交点是否在射线的正方向上
			FVector ToMainIntersection = IntersectionPoint - RayOrigin;
			float T = FVector::DotProduct(ToMainIntersection, RayDirection);

			if (T < 0.0f)
				return;

			if (bCtrlDown)
			{
				// Ctrl键按下：高度编辑模式
				// 找到法线方向的主要轴
				FVector AbsNormal = PlaneNormal.GetAbs();
				int32 MainAxis = 0;
				if (AbsNormal.Y > AbsNormal.X)
					MainAxis = (AbsNormal.Z > AbsNormal.Y) ? 2 : 1;
				else
					MainAxis = (AbsNormal.Z > AbsNormal.X) ? 2 : 0;

				// 找到另外两个轴作为垂直平面的法线方向（垂直于MainAxis的两个轴）
				int32 Axis1 = (MainAxis + 1) % 3;  // 第一个垂直轴
				int32 Axis2 = (MainAxis + 2) % 3;  // 第二个垂直轴

				// 创建垂直于MainAxis的平面，通过初始点击的体素位置
				FVector BaseWorldPos = FVector(
					(DragStartPos.X - HalfTileSizeX) * VoxelSize,
					(DragStartPos.Y - HalfTileSizeY) * VoxelSize,
					(DragStartPos.Z - HalfTileSizeZ) * VoxelSize
				);

				// 尝试第一个垂直平面（法线沿Axis1方向）
				FVector PerpPlaneNormal1 = FVector::ZeroVector;
				PerpPlaneNormal1[Axis1] = 1.0f;
				FPlane PerpPlane1(BaseWorldPos, PerpPlaneNormal1);

				FVector MouseIntersectionPoint;
				bool bFoundIntersection = false;

				// 检查第一个平面是否与射线相交
				float Denom1 = FVector::DotProduct(PerpPlaneNormal1, RayDirection);
				if (FMath::Abs(Denom1) > 0.0001f)
				{
					MouseIntersectionPoint = FMath::RayPlaneIntersection(RayOrigin, RayDirection, PerpPlane1);
					FVector ToPerpIntersection = MouseIntersectionPoint - RayOrigin;
					float T1 = FVector::DotProduct(ToPerpIntersection, RayDirection);
					if (T1 >= 0.0f)
						bFoundIntersection = true;
				}

				// 如果第一个平面平行，尝试第二个垂直平面（法线沿Axis2方向）
				if (!bFoundIntersection)
				{
					FVector PerpPlaneNormal2 = FVector::ZeroVector;
					PerpPlaneNormal2[Axis2] = 1.0f;
					FPlane PerpPlane2(BaseWorldPos, PerpPlaneNormal2);

					float Denom2 = FVector::DotProduct(PerpPlaneNormal2, RayDirection);
					if (FMath::Abs(Denom2) > 0.0001f)
					{
						MouseIntersectionPoint = FMath::RayPlaneIntersection(RayOrigin, RayDirection, PerpPlane2);
						FVector ToPerpIntersection2 = MouseIntersectionPoint - RayOrigin;
						float T2 = FVector::DotProduct(ToPerpIntersection2, RayDirection);
						if (T2 >= 0.0f)
							bFoundIntersection = true;
					}
				}

				if (bFoundIntersection)
				{
					// 将MouseIntersectionPoint投影到原始平面上，然后计算高度
					FVector PointToPlane = MouseIntersectionPoint - PointOnPlane;
					float Height = FVector::DotProduct(PointToPlane, PlaneNormal);
					int32 HeightInt = FMath::RoundToInt(Height / VoxelSize);

					// 保留其他两个轴的当前值，只修改法线方向的轴
					DragEndPos[MainAxis] = DragStartPos[MainAxis] + HeightInt * DragStartHitNormal[MainAxis];
				}
			}
			else
			{
				DragEndPos = FIntVector(
					FMath::RoundToInt((IntersectionPoint.X - (1.0 - FMath::Abs(PlaneNormal.X)) * VoxelSize / 2) / VoxelSize + HalfTileSizeX) - PlaneNormalX,
					FMath::RoundToInt((IntersectionPoint.Y - (1.0 - FMath::Abs(PlaneNormal.Y)) * VoxelSize / 2) / VoxelSize + HalfTileSizeY) - PlaneNormalY,
					FMath::RoundToInt((IntersectionPoint.Z - (1.0 - FMath::Abs(PlaneNormal.Z)) * VoxelSize / 2) / VoxelSize + HalfTileSizeZ) - PlaneNormalZ
				);

				FIntVector IntDistance = DragEndPos - DragStartHitPos;
				FVector Distance = FVector(IntDistance.X, IntDistance.Y, IntDistance.Z);
			}

			// 更新 VoxelEditVolume 的位置和大小以匹配选择框
			UpdateVoxelEditVolume(WorldEditor->GetWorld(), Terrain);
		}
	}
}

void UVoxelEditorEditTool::OnClickRelease(const FInputDeviceRay& ReleasePos)
{
	// Place the pending voxel if we have one
	if (bHasPendingPlacement)
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

		if (CurrentBlockType == VOXEL_BLOCK_TYPE_PLACE)
		{
			// Place the voxel (cube)
			Terrain->SetVoxelAtWorldPosition(PendingPlacementPos, 0, 1, UCVoxelBlockType_Cube, 0, 0, TargetWorld);
			UE_LOG(LogTemp, Log, TEXT("Placed cube voxel at: (%d, %d, %d)"), PendingPlacementPos.X, PendingPlacementPos.Y, PendingPlacementPos.Z);
		}
		else if (CurrentBlockType == VOXEL_BLOCK_TYPE_PLACE_SQUARE_SLOPE || CurrentBlockType == VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_SLOPE || CurrentBlockType == VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_COMPLEMENT)
		{
			// Place slope: calculate rotation based on hit normal (bottom face) and ray direction (front face direction)
			// 方斜面：正面正对点击方向（射线方向的反方向），底面是被点击的面（HitNormal方向）
			
			uint8 BlockType;
			if (CurrentBlockType == VOXEL_BLOCK_TYPE_PLACE_SQUARE_SLOPE)
				BlockType = UCVoxelBlockType_SquareSlope;
			else if (CurrentBlockType == VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_SLOPE)
				BlockType = UCVoxelBlockType_TriangularSlope;
			else // VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_COMPLEMENT
				BlockType = UCVoxelBlockType_TriangularComplement;
			
			uint8 RotationX = 0;
			uint8 RotationY = 0;
			uint8 RotationZ = 0;
			
			// 1. 确定底面：HitNormal指向被点击面的方向
			// 2. 确定正面：射线方向的反方向（因为射线是从相机指向物体的）
			// 方斜面的正面是FaceIndex=3（Back面，面向-X方向），底面是FaceIndex=5（Bottom面，面向-Z方向）
			// 根据HitNormal和RayDirection计算旋转
			
			// 将法线向量转换为最接近的面索引（用于确定底面）
			FVector NormalizedHitNormal = PendingPlacementHitNormal.GetSafeNormal();
			FVector NormalizedRayDir = -PendingPlacementRayDirection.GetSafeNormal(); // 取反，因为射线是从相机指向物体，而我们需要的是从物体指向相机的方向
			
			// 定义六个面的法向量（与FaceNormals一致）
			FVector FaceNormals[6] = {
				FVector( 0, -1,  0), // 0: Left (面向-Y方向)
				FVector( 1,  0,  0), // 1: Front (面向+X方向)
				FVector( 0,  1,  0), // 2: Right (面向+Y方向)
				FVector(-1,  0,  0), // 3: Back (面向-X方向) - 这是方斜面的正面
				FVector( 0,  0,  1), // 4: Top (面向+Z方向) - 这是方斜面的顶面
				FVector( 0,  0, -1)  // 5: Bottom (面向-Z方向) - 这是方斜面的底面
			};
			
			// 找到最接近HitNormal的面（底面）
			int32 BottomFaceIndex = 5; // 默认为Bottom面
			float MaxDotBottom = -1.0f;
			for (int32 i = 0; i < 6; ++i)
			{
				float Dot = FVector::DotProduct(NormalizedHitNormal, FaceNormals[i]);
				if (Dot > MaxDotBottom)
				{
					MaxDotBottom = Dot;
					BottomFaceIndex = i;
				}
			}
			
			// 找到最接近RayDirection的面（正面，但不应该是Top/Bottom）
			// 正面应该是侧面之一（0-3），不能是顶面或底面
			int32 FrontFaceIndex = 3; // 默认Back面作为正面
			float MaxDotFront = -1.0f;
			for (int32 i = 0; i < 4; ++i) // 只考虑侧面（0-3）
			{
				float Dot = FVector::DotProduct(NormalizedRayDir, FaceNormals[i]);
				if (Dot > MaxDotFront)
				{
					MaxDotFront = Dot;
					FrontFaceIndex = i;
				}
			}
			
			// 根据底面和正面计算RotationZ
			// 方斜面的斜面在顶面（FaceIndex=4），根据RotationZ决定斜面的方向
			// RotationZ: 0=从-X到+X, 1=从-Y到+Y, 2=从+X到-X, 3=从+Y到-Y
			
			// 如果底面是Top或Bottom面，需要根据正面方向来确定RotationZ
			if (BottomFaceIndex == 4 || BottomFaceIndex == 5)
			{
				// 底面是Top或Bottom，正面决定了斜面的方向
				if (FrontFaceIndex == 0) // Left面（-Y方向）
				{
					RotationZ = 1; // 从-Y到+Y
				}
				else if (FrontFaceIndex == 1) // Front面（+X方向）
				{
					RotationZ = 0; // 从-X到+X
				}
				else if (FrontFaceIndex == 2) // Right面（+Y方向）
				{
					RotationZ = 3; // 从+Y到-Y
				}
				else // FrontFaceIndex == 3: Back面（-X方向）
				{
					RotationZ = 2; // 从+X到-X
				}
			}
			else
			{
				// 底面是侧面，需要更复杂的计算
				// 这里简化处理：根据正面方向
				if (FrontFaceIndex == 0) // Left面
				{
					RotationZ = 1;
				}
				else if (FrontFaceIndex == 1) // Front面
				{
					RotationZ = 0;
				}
				else if (FrontFaceIndex == 2) // Right面
				{
					RotationZ = 3;
				}
				else // Back面
				{
					RotationZ = 2;
				}
			}
			
			// 计算YawRoll: Roll * 4 + Yaw
			uint8 YawRoll = (RotationX & 0x03) * 4 + (RotationY & 0x03);
			Terrain->SetVoxelAtWorldPosition(PendingPlacementPos, 0, 1, BlockType, RotationX & 0x03, RotationY & 0x03, TargetWorld);
			UE_LOG(LogTemp, Log, TEXT("Placed slope voxel at: (%d, %d, %d), Type: %d, BottomFace: %d, FrontFace: %d, RotationZ: %d"), 
				PendingPlacementPos.X, PendingPlacementPos.Y, PendingPlacementPos.Z, BlockType, BottomFaceIndex, FrontFaceIndex, RotationZ);
		}
		
		// Keep the selection to show the placed voxel
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

void UVoxelEditorEditTool::OnTick(float DeltaTime)
{
	UInteractiveTool::OnTick(DeltaTime);
	
	// 检查是否有选中的区域
	if (!bHasSelectedRegion)
	{
		return;
	}
	
	// 检测按键输入
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}
	
	// 使用平台API检测按键状态
	// 注意：这种方法在Windows上有效，其他平台可能需要不同的实现
	bool bSpaceCurrentlyPressed = false;
	bool bDeleteCurrentlyPressed = false;
	
#if PLATFORM_WINDOWS
	// 使用Windows API检测按键
	bSpaceCurrentlyPressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
	bDeleteCurrentlyPressed = (GetAsyncKeyState(VK_DELETE) & 0x8000) != 0;
#else
	// 其他平台的实现
	// 暂时使用 FSlateApplication 的方法（如果可用）
	FSlateApplication& SlateApp = FSlateApplication::Get();
	// 这里可以添加其他平台的实现
#endif
	
	// 检测 Space 键（填充）- 只在按键从释放变为按下时触发一次
	if (bSpaceCurrentlyPressed && !bSpaceKeyPressed)
	{
		ModifySelectedVoxels(false); // false = fill
	}
	bSpaceKeyPressed = bSpaceCurrentlyPressed;
	
	// 检测 Delete 键（删除）- 只在按键从释放变为按下时触发一次
	if (bDeleteCurrentlyPressed && !bDeleteKeyPressed)
	{
		ModifySelectedVoxels(true); // true = delete
	}
	bDeleteKeyPressed = bDeleteCurrentlyPressed;
}

void UVoxelEditorEditTool::ModifySelectedVoxels(bool bDelete)
{
	if (!bHasSelectedRegion)
	{
		return;
	}
	
	// Get the editor mode to access Toolkit
	UVoxelEditorEditorMode* VoxelMode = UVoxelEditorEditorMode::GetActiveEditorMode();
	if (!VoxelMode)
	{
		return;
	}

	TSharedPtr<FVoxelEditorEditorModeToolkit> Toolkit = VoxelMode->GetToolkit();
	if (!Toolkit.IsValid())
	{
		return;
	}
	
	// 调用 Toolkit 的函数：填充或删除
	if (bDelete)
	{
		Toolkit->ClearEditVolume();
	}
	else
	{
		Toolkit->ApplyEditVolume();
	}
}


#undef LOCTEXT_NAMESPACE



