/********************************************************************
created:	2024/12/XX
filename: 	VoxelWorldEditor.h
author:		Auto Generated

purpose:	AVoxelWorldEditor - 编辑器专用体素世界类
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldBase.h"
#include "VoxelWorldEditor.generated.h"

/**
 * AVoxelWorldEditor - 编辑器专用体素世界类
 * 
 * 扩展功能：
 * - 编辑器工具和可视化
 * - 调试功能
 * - 编辑器专用操作
 */
UCLASS(BlueprintType, Blueprintable)
class VOXELEDITOR_API AVoxelWorldEditor : public AVoxelWorldBase
{
	GENERATED_BODY()

public:
	AVoxelWorldEditor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	// ========== 编辑器专用功能 ==========

	/** 编辑器中的可视化调试 */
	UFUNCTION(CallInEditor, Category = "VoxelWorld|Editor")
	void EditorVisualizeDebug();

	/** 编辑器中的快速测试 */
	UFUNCTION(CallInEditor, Category = "VoxelWorld|Editor")
	void EditorQuickTest();

	/** 显示地图管理窗口 */
	UFUNCTION(CallInEditor, Category = "VoxelWorld|Editor")
	void ShowMapsWindow();

	/** 获取地图管理器（用于UI访问） */
	UCVoxelMapManager& GetMapManager() { return MapManager; }

	/** 设置当前加载的地图文件路径 */
	void SetCurrentMapFilePath(const FString& FilePath) { CurrentMapFilePath = FilePath; }

	/** 获取当前加载的地图文件路径 */
	FString GetCurrentMapFilePath() const { return CurrentMapFilePath; }

	/** 获取 VoxelWorld 地图目录路径（位于 ExternalData 目录） */
	static FString GetVoxelWorldMapsDirectory();

	/** 获取 VoxelWorld 资源目录路径（位于 ExternalData 目录） */
	static FString GetVoxelWorldDirectory();

private:
	/** 当前加载的地图文件路径 */
	FString CurrentMapFilePath;
#endif
};

