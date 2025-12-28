/********************************************************************
created:	2024/12/XX
filename: 	VoxelWorld.h
author:		Auto Generated

purpose:	VoxelWorld - 体素世界管理类层次结构
			WorldBase: 基类，包含通用功能
			WorldEditor: 编辑器专用派生类
			WorldGame: 游戏专用派生类
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelData.h"
#include "VoxelTerrain.h"
#include "VoxelWorldBase.generated.h"

/**
 * AVoxelWorldBase - 体素世界基类
 * 
 * 功能：
 * 1. 管理一个 AVoxelTerrain 地形对象
 * 2. 管理多个预制件对象（UCVoxelPrefabData）
 * 3. 使用 UCVoxelMapManager 加载和保存地图数据
 * 
 * 派生类：
 * - AVoxelWorldEditor: 编辑器专用，包含编辑器工具和功能
 * - AVoxelWorldGame: 游戏专用，优化运行时性能
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class VOXELCORE_API AVoxelWorldBase : public AActor
{
	GENERATED_BODY()

public:
	AVoxelWorldBase(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ========== 地图加载/保存 ==========

	/**
	 * 加载地图文件
	 * @param Filename 地图文件路径
	 * @return 是否加载成功
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld")
	bool LoadMap(const FString& Filename);

	/**
	 * 保存地图文件
	 * @param Filename 地图文件路径
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld")
	void SaveMap(const FString& Filename);

	// ========== 地形访问 ==========

	/**
	 * 获取地形对象
	 * @return 地形 Actor，如果未创建则返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld")
	AVoxelTerrain* GetTerrain() const { return TerrainActor; }

	/**
	 * 创建或获取地形对象
	 * @return 地形 Actor
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld")
	AVoxelTerrain* CreateTerrain();

	// ========== 预制件管理 ==========

	/**
	 * 添加预制件
	 * @param PrefabData 预制件数据
	 * @return 预制件索引
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld")
	int32 AddPrefab(const FString& Name, int32 Type);

	/**
	 * 移除预制件
	 * @param Index 预制件索引
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld")
	void RemovePrefab(int32 Index);

	/**
	 * 获取预制件数量
	 * @return 预制件数量
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld")
	int32 GetPrefabCount() const;

	/**
	 * 获取预制件数据
	 * @param Index 预制件索引
	 * @return 预制件名称和类型（通过输出参数返回）
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelWorld")
	void GetPrefab(int32 Index, FString& OutName, int32& OutType) const;

	// ========== 配置属性 ==========

	/** 地图管理器（用于加载/保存地图数据） */
	// 注意：UCVoxelMapManager 不是 UObject，不能使用 UPROPERTY
	UCVoxelMapManager MapManager;

	/** 地形 Actor（子对象） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VoxelWorld")
	AVoxelTerrain* TerrainActor;

	/** 是否在 BeginPlay 时自动创建地形 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelWorld")
	bool bAutoCreateTerrain;

	/** 地形初始尺寸（体素数量） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelWorld")
	FIntVector InitialTerrainSize;

	/** 体素大小（世界单位） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelWorld")
	float VoxelSize;

private:
	// ========== 内部方法 ==========

	/** 初始化地图管理器 */
	void InitializeMapManager();

	/** 从 UCString 转换为 FString */
	FString UCStringToFString(const UCString& UCStr) const;

	/** 从 FString 转换为 UCString */
	UCString FStringToUCString(const FString& FStr) const;

private:
	// ========== 数据 ==========

	/** 预制件数据列表 */
	TArray<UCVoxelPrefabData> PrefabList;

	/** 预制件 Actor 列表（可选，用于在场景中显示预制件） */
	TArray<AActor*> PrefabActors;
};

