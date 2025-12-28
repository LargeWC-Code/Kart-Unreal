/********************************************************************
created:	2024/12/XX
filename: 	VoxelWorldGame.h
author:		Auto Generated

purpose:	AVoxelWorldGame - 游戏专用体素世界类
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldBase.h"
#include "VoxelWorldGame.generated.h"

/**
 * AVoxelWorldGame - 游戏专用体素世界类
 * 
 * 优化功能：
 * - 运行时性能优化
 * - 流式加载
 * - 游戏专用功能
 */
UCLASS(BlueprintType, Blueprintable)
class KARTGAME_API AVoxelWorldGame : public AVoxelWorldBase
{
	GENERATED_BODY()

public:
	AVoxelWorldGame(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ========== 游戏专用功能 ==========

	/** 流式加载地图块 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld|Game")
	void StreamLoadChunk(const FIntVector& ChunkCoord);

	/** 流式卸载地图块 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld|Game")
	void StreamUnloadChunk(const FIntVector& ChunkCoord);
};

