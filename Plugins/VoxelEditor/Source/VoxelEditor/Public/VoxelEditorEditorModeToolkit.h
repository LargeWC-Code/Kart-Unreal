// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Toolkits/BaseToolkit.h"
#include "VoxelEditorEditorMode.h"
#include "VoxelBlockTypes.h"
#include "Widgets/SWidget.h"
#include "Widgets/Layout/SUniformGridPanel.h"

class AVoxelWorldEditor;

template<typename T>
class SComboBox;

/**
 * This FModeToolkit just creates a basic UI panel that allows various InteractiveTools to
 * be initialized, and a DetailsView used to show properties of the active Tool.
 */
class FVoxelEditorEditorModeToolkit : public FModeToolkit
{
public:
	FVoxelEditorEditorModeToolkit();

	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;
	virtual TSharedPtr<SWidget> GetInlineContent() const override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;

	/** Update the edit tool grid from map dimensions */
	void UpdateEditToolGridFromMap(int32 MapWidth, int32 MapHeight) const;

	/** Get the selected block type */
	int32 GetSelectedBlockType() const;

private:
	/** 工具包的主 Widget */
	TSharedPtr<SWidget> ToolkitWidget;

	/** Main content widget */
	mutable TSharedPtr<SWidget> MainContentWidget;

	/** Edit tool widget */
	mutable TSharedPtr<SWidget> EditToolWidget;

	/** Current map dimensions */
	mutable int32 CurrentMapWidth;
	mutable int32 CurrentMapHeight;
	mutable bool bMapLoaded;

	/** Edit tool button states */
	mutable TArray<bool> EditToolButtonStates;

	/** Block type combo box options and selected index */
	mutable TSharedPtr<TArray<TSharedPtr<FString>>> BlockTypeOptions;
	mutable int32 SelectedBlockTypeIndex = VOXEL_BLOCK_TYPE_SELECT;
	mutable TSharedPtr<class SComboBox<TSharedPtr<FString>>> BlockTypeComboBox;

	/** Owning editor mode */
	TWeakObjectPtr<UEdMode> OwningMode;

	/** VoxelWorldEditor instance */
	TWeakObjectPtr<AVoxelWorldEditor> VoxelWorldEditorInstance;

	/** Edit tool grid panel */
	mutable TSharedPtr<SUniformGridPanel> EditToolGridPanel;

	/** Create the tool content widget */
	TSharedPtr<SWidget> CreateToolContentWidget() const;

	/** Get the active tool index */
	int32 GetActiveToolIndex() const;

	/** Get the maps tool widget */
	TSharedPtr<SWidget> GetMapsToolWidget() const;

	/** Get the edit tool widget */
	TSharedPtr<SWidget> GetEditToolWidget() const;

	/** Apply the edit volume (finds and applies the first available volume) */
	void ApplyEditVolume() const;

	/** Clear the edit volume region (sets voxels to type 0) */
	void ClearEditVolume() const;

	/** Clear the current tile (the tile containing the edit volume) */
	void ClearCurrentTile() const;
};
