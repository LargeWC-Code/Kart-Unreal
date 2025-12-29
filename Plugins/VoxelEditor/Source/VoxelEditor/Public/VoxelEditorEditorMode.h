// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Tools/UEdMode.h"
#include "VoxelEditorEditorMode.generated.h"

/**
 * This class provides an example of how to extend a UEdMode to add some simple tools
 * using the InteractiveTools framework. The various UEdMode input event handlers (see UEdMode.h)
 * forward events to a UEdModeInteractiveToolsContext instance, which
 * has all the logic for interacting with the InputRouter, ToolManager, etc.
 * The functions provided here are the minimum to get started inserting some custom behavior.
 * Take a look at the UEdMode markup for more extensibility options.
 */
UCLASS()
class UVoxelEditorEditorMode : public UEdMode
{
	GENERATED_BODY()

public:
	const static FEditorModeID EM_VoxelEditorEditorModeId;

	static FString MapsToolName;
	static FString EditToolName;

	/** Whether a map is currently loaded (enables Edit tool) */
	static bool bMapLoaded;

	UVoxelEditorEditorMode();
	virtual ~UVoxelEditorEditorMode();

	/** UEdMode interface */
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void ActorSelectionChangeNotify() override;
	virtual void CreateToolkit() override;
	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override;

	/** Set map loaded state (enables/disables Edit tool) */
	static void SetMapLoaded(bool bLoaded) { bMapLoaded = bLoaded; }

	/** Get map loaded state */
	static bool IsMapLoaded() { return bMapLoaded; }

	/** 获取 VoxelWorldEditor 实例 */
	class AVoxelWorldEditor* GetVoxelWorldEditorInstance() const { return VoxelWorldEditorInstance; }

	/** 设置 VoxelWorldEditor 实例 */
	void SetVoxelWorldEditorInstance(class AVoxelWorldEditor* Instance) { VoxelWorldEditorInstance = Instance; }

	/** 获取 Toolkit（用于更新UI） */
	TSharedPtr<class FVoxelEditorEditorModeToolkit> GetToolkit() const;

private:
	/** 当前编辑器模式中的 VoxelWorldEditor 实例 */
	UPROPERTY()
	TObjectPtr<class AVoxelWorldEditor> VoxelWorldEditorInstance;
};