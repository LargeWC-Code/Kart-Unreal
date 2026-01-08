// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "InteractiveToolBuilder.h"
#include "BaseTools/ClickDragTool.h"
#include "VoxelBlockTypes.h"
#include "VoxelEditorEditTool.generated.h"

// Forward declarations
class UVoxelTerrain;


/**
 * Builder for UVoxelEditorEditTool
 */
UCLASS()
class VOXELEDITOR_API UVoxelEditorEditToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override { return true; }
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};


/**
 * Property set for the UVoxelEditorEditTool
 */
UCLASS(Transient)
class VOXELEDITOR_API UVoxelEditorEditToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	UVoxelEditorEditToolProperties();

	/** First point of measurement */
	UPROPERTY(EditAnywhere, Category = Options)
	FVector StartPoint;

	/** Second point of measurement */
	UPROPERTY(EditAnywhere, Category = Options)
	FVector EndPoint;
	
	/** Current distance measurement */
	UPROPERTY(EditAnywhere, Category = Options)
	double Distance;
};



/**
 * UVoxelEditorEditTool is a Tool that allows the user to edit voxel terrain and prefabs.
 * The first point is set by click-dragging the mouse, and
 * the second point is set by shift-click-dragging the mouse.
 */
UCLASS()
class VOXELEDITOR_API UVoxelEditorEditTool : public UInteractiveTool, public IClickDragBehaviorTarget
{
	GENERATED_BODY()

public:
	virtual void SetWorld(UWorld* World);

	/** UInteractiveTool overrides */
	virtual void Setup() override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;
	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;
	virtual void OnTick(float DeltaTime) override;

	/** IClickDragBehaviorTarget implementation */
	virtual FInputRayHit CanBeginClickDragSequence(const FInputDeviceRay& PressPos) override;
	virtual void OnClickPress(const FInputDeviceRay& PressPos) override;
	virtual void OnClickDrag(const FInputDeviceRay& DragPos) override;
	virtual void OnClickRelease(const FInputDeviceRay& ReleasePos) override;
	virtual void OnTerminateDragSequence() override {}

	/** IModifierToggleBehaviorTarget implementation (inherited via IClickDragBehaviorTarget) */
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn) override;


protected:
	/** Properties of the tool are stored here */
	UPROPERTY()
	TObjectPtr<UVoxelEditorEditToolProperties> Properties;


protected:
	UWorld* TargetWorld = nullptr;		// target World we will raycast into

	static const int ShiftKeyModifierID = 1;	// identifier for Shift key
	bool bShiftKeyDown = false;					// flag to track Shift key state
	
	/** Track key states for one-time trigger */
	bool bSpaceKeyPressed = false;				// track Space key state
	bool bDeleteKeyPressed = false;				// track Delete key state

	/** Pending voxel placement position (for "摆放砖块" mode) */
	FIntVector PendingPlacementPos;
	FVector PendingPlacementHitNormal;  // Store the hit normal for slope rotation calculation (the bottom face)
	FVector PendingPlacementRayDirection;  // Store the ray direction for slope rotation calculation (the front face direction)
	bool bHasPendingPlacement = false;
	
	/** Current block type for placement */
	int32 CurrentBlockType = VOXEL_BLOCK_TYPE_SELECT;
	
	/** Drag start position (for drag selection) */
	FIntVector DragStartPos;
	bool bIsDragging = false;
	
	/** Drag start hit position and normal (for plane-based drag calculation) */
	FIntVector DragStartHitPos;
	FIntVector DragStartHitNormal;
	bool bHasDragStartPlane = false;
	
	/** Drag end position (current mouse position during drag) */
	FIntVector DragEndPos;
	
	/** Selected region (for persistent selection after drag ends) */
	FIntVector SelectedMinPos;
	FIntVector SelectedMaxPos;
	bool bHasSelectedRegion = false;
		
	/** Helper function to convert voxel coordinates to world box */
	TTuple<FVector, FVector> VoxelCoordsToWorldBox(const FIntVector& MinPos, const FIntVector& MaxPos, float VoxelSize) const;
	
	/** Update VoxelEditVolume position and size to match the drag selection box */
	void UpdateVoxelEditVolume(UWorld* World, UVoxelTerrain* Terrain);
	
	/** Fill or delete selected voxels (bDelete: true=delete, false=fill) */
	void ModifySelectedVoxels(bool bDelete);
};



