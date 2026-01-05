/********************************************************************
created:	2024/12/XX
filename: 	VoxelTerrain.h
author:		Auto Generated

purpose:	VoxelTerrain - 体素地形渲染类，参考UCPixelWorld实现
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VoxelTerrain.generated.h"

// 前向声明
class AVoxelTile;
struct UCVoxelMapData;
// 顶点数据结构（用于自定义shader）
USTRUCT()
struct VOXELCORE_API FVoxelVertex
{
	GENERATED_BODY()

	FVector Position;
	FVector Normal;
	FVector2D UV;
	FColor Color;

	FVoxelVertex()
		: Position(0, 0, 0)
		, Normal(0, 0, 1)
		, UV(0, 0)
		, Color(255, 255, 255, 255)
	{
	}

	FVoxelVertex(const FVector& InPos, const FVector& InNormal, const FVector2D& InUV, const FColor& InColor)
		: Position(InPos)
		, Normal(InNormal)
		, UV(InUV)
		, Color(InColor)
	{
	}
};

/**
 * UVoxelTerrain - 体素地形管理类
 * 管理多个地块（AVoxelTile），每个地块包含32*32*64的单元格
 * 
 * 使用说明：
 * 1. 在VoxelWorldBase中创建VoxelTerrain管理对象
 * 2. 通过ActivateTile/DeactivateTile来激活/取消激活地块
 * 3. 通过GetOrCreateTile来获取或创建地块
 */
UCLASS(BlueprintType, Blueprintable)
class VOXELCORE_API UVoxelTerrain : public UObject
{
	GENERATED_BODY()

public:
	UVoxelTerrain();

	// ========== 地块管理 ==========
	
	/**
	 * 获取或创建地块
	 * @param TileX, TileY 地块坐标
	 * @param World 世界对象（用于生成Actor）
	 * @return 地块Actor
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	class AVoxelTile* GetTile(int32 TileX, int32 TileY, UWorld* World, bool AutoCreate = false);

	/**
	* 检查地块是否激活
	* @param TileX, TileY 地块坐标
	* @return 是否激活
	*/
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	bool IsTileActive(int32 TileX, int32 TileY) const;

	/**
	 * 激活地块
	 * @param TileX, TileY 地块坐标
	 * @param World 世界对象（用于创建和销毁Actor）
	 * @param bActive 是否激活
	 * @param MapData 可选的地图数据（如果提供，将从地图数据中加载TileData）
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	void SetTileActive(int32 TileX, int32 TileY, UWorld* World, bool bActive);

	/**
	 * 移除地块
	 * @param TileX, TileY 地块坐标
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	void RemoveTile(int32 TileX, int32 TileY);

	/**
	 * 填充区域内的体素
	 * @param Min 区域最小坐标（世界坐标，厘米）
	 * @param Max 区域最大坐标（世界坐标，厘米）
	 * @param VoxelType 体素类型
	 * @param Layer 体素层
	 * @param World 世界对象
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	void FillRegion(const FVector& Min, const FVector& Max, uint8 VoxelType, uint8 Layer = 1, UWorld* World = nullptr);

	/**
	 * 在世界坐标位置设置/删除一个体素
	 * @param WorldPosition 世界坐标位置（厘米）
	 * @param VoxelType 体素类型（0表示删除）
	 * @param Layer 体素层
	 * @param World 世界对象
	 * @param bUpdateMesh 是否立即更新网格
	 * @return 是否成功设置
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	bool SetVoxelAtWorldPosition(const FIntVector& WorldPosition, uint8 VoxelType, uint8 Layer = 1, UWorld* World = nullptr, bool bUpdateMesh = true);

	/**
	 * 射线与Terrain的交集检测（参考UCPixelWorld::Intersect）
	 * @param RayOrigin 射线起点（世界坐标）
	 * @param RayDirection 射线方向（归一化）
	 * @param OutHitPosition 输出的hit位置（世界坐标）
	 * @param OutHitNormal 输出的hit法线（世界坐标）
	 * @return 是否找到hit
	 */
	bool Intersect(const FVector& RayOrigin, const FVector& RayDirection, FIntVector& OutHitVoxelPosition, FVector& OutHitPosition, FVector& OutHitNormal) const;

	// ========== 配置属性 ==========

	/** 体素大小（世界单位，默认100厘米=1米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTerrain")
	float VoxelSize;

	/** 材质（如果为空则使用默认材质） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTerrain")
	UMaterialInterface* Material;

private:
	// ========== 内部方法 ==========

	/** 将地块坐标转换为键值 */
	FIntPoint GetTileKey(int32 TileX, int32 TileY) const { return FIntPoint(TileX, TileY); }

	/** 计算地块的世界位置 */
	FVector GetTileWorldPosition(int32 TileX, int32 TileY) const;

	/** 计算地块的世界位置 */
	FIntVector GetTileWorldGridPosition(int32 TileX, int32 TileY) const;

private:
	// ========== 数据 ==========

	/** 地块映射表（坐标 -> 地块Actor） */
	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<class AVoxelTile>> TileMap;

	/** 激活的地块集合 */
	TSet<FIntPoint> ActiveTiles;
};

