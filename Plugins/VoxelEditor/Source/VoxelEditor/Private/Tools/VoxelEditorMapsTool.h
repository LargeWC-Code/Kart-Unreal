// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/NoExportTypes.h"
#include "InteractiveToolBuilder.h"
#include "BaseTools/SingleClickTool.h"
#include "VoxelEditorMapsTool.generated.h"

/**
 * Builder for UVoxelEditorMapsTool
 */
UCLASS()
class VOXELEDITOR_API UVoxelEditorMapsToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override { return true; }
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};




/**
 * Settings UObject for UVoxelEditorMapsTool. This UClass inherits from UInteractiveToolPropertySet,
 * which provides an OnModified delegate that the Tool will listen to for changes in property values.
 */
UCLASS(Transient)
class VOXELEDITOR_API UVoxelEditorMapsToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UVoxelEditorMapsToolProperties();

	/** If enabled, dialog should display extended information about the actor clicked on. Otherwise, only basic info will be shown. */
	UPROPERTY(EditAnywhere, Category = Options, meta = (DisplayName = "Show Extended Info"))
	bool ShowExtendedInfo;
};




/**
 * UVoxelEditorMapsTool is a Tool that displays all maps in the world.
 * All the action is in the ::OnClicked handler.
 */
UCLASS()
class VOXELEDITOR_API UVoxelEditorMapsTool : public USingleClickTool
{
	GENERATED_BODY()

public:
	UVoxelEditorMapsTool();

	virtual void SetWorld(UWorld* World);

	virtual void Setup() override;

	virtual void OnClicked(const FInputDeviceRay& ClickPos);


protected:
	UPROPERTY()
	TObjectPtr<UVoxelEditorMapsToolProperties> Properties;


protected:
	/** target World we will raycast into to find actors */
	UWorld* TargetWorld;
};












