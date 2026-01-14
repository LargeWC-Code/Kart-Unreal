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

// War3 纹理结构信息
USTRUCT(BlueprintType)
struct VOXELCORE_API FWar3TextureInfo
{
	GENERATED_BODY()

	/** 格子总数（16 或 32） */
	UPROPERTY(BlueprintReadOnly, Category = "War3Texture")
	int32 TotalCells;

	/** 是否有随机种子格子 */
	UPROPERTY(BlueprintReadOnly, Category = "War3Texture")
	bool bHasRandomVariants;

	/** 行数 */
	UPROPERTY(BlueprintReadOnly, Category = "War3Texture")
	int32 Rows;

	/** 列数 */
	UPROPERTY(BlueprintReadOnly, Category = "War3Texture")
	int32 Columns;

	/** 每个格子的 UV 大小（归一化，0-1） */
	UPROPERTY(BlueprintReadOnly, Category = "War3Texture")
	FVector2D CellUVSize;

	/** 纹理宽度 */
	UPROPERTY(BlueprintReadOnly, Category = "War3Texture")
	int32 TextureWidth;

	/** 纹理高度 */
	UPROPERTY(BlueprintReadOnly, Category = "War3Texture")
	int32 TextureHeight;

	FWar3TextureInfo()
		: TotalCells(16)
		, bHasRandomVariants(false)
		, Rows(4)
		, Columns(4)
		, CellUVSize(0.25f, 0.25f)
		, TextureWidth(0)
		, TextureHeight(0)
	{
	}

	FWar3TextureInfo(int32 InWidth, int32 InHeight)
		: TextureWidth(InWidth)
		, TextureHeight(InHeight)
	{
		// 根据宽高比自动判断结构
		if (InWidth > 0 && InHeight > 0)
		{
			float AspectRatio = (float)InWidth / (float)InHeight;
			
			// 如果横向是纵向的两倍（2:1），表示有随机种子格子，一共32格（8x4）
			if (FMath::IsNearlyEqual(AspectRatio, 2.0f, 0.1f))
			{
				TotalCells = 32;
				bHasRandomVariants = true;
				Rows = 4;
				Columns = 8;
				CellUVSize = FVector2D(1.0f / 8.0f, 1.0f / 4.0f);
			}
			// 如果横竖相等（1:1），就是16格，没有随机种子格子（4x4）
			else if (FMath::IsNearlyEqual(AspectRatio, 1.0f, 0.1f))
			{
				TotalCells = 16;
				bHasRandomVariants = false;
				Rows = 4;
				Columns = 4;
				CellUVSize = FVector2D(0.25f, 0.25f);
			}
			else
			{
				// 默认使用16格结构
				TotalCells = 16;
				bHasRandomVariants = false;
				Rows = 4;
				Columns = 4;
				CellUVSize = FVector2D(0.25f, 0.25f);
			}
		}
		else
		{
			// 默认值
			TotalCells = 16;
			bHasRandomVariants = false;
			Rows = 4;
			Columns = 4;
			CellUVSize = FVector2D(0.25f, 0.25f);
		}
	}

	/**
	 * 获取指定格子的 UV 坐标（左上角）
	 * @param CellIndex 格子索引（0-15 或 0-31）
	 * @param VariantIndex 随机变体索引（0-1，仅当 bHasRandomVariants=true 时有效）
	 * @return UV 坐标（归一化，0-1）
	 */
	FVector2D GetCellUV(int32 CellIndex, int32 VariantIndex = 0) const
	{
		if (CellIndex < 0 || CellIndex >= TotalCells)
			return FVector2D(0, 0);

		int32 Row = CellIndex / Columns;
		int32 Col = CellIndex % Columns;

		// 如果有随机变体，需要调整列索引
		if (bHasRandomVariants && VariantIndex > 0)
		{
			// 随机变体在相邻列
			Col += Columns / 2;
		}

		return FVector2D(Col * CellUVSize.X, Row * CellUVSize.Y);
	}

	/**
	 * 获取指定格子的 UV 范围
	 * @param CellIndex 格子索引（0-15 或 0-31）
	 * @param VariantIndex 随机变体索引（0-1，仅当 bHasRandomVariants=true 时有效）
	 * @param OutMinUV 输出的最小 UV 坐标
	 * @param OutMaxUV 输出的最大 UV 坐标
	 */
	void GetCellUVRange(int32 CellIndex, int32 VariantIndex, FVector2D& OutMinUV, FVector2D& OutMaxUV) const
	{
		OutMinUV = GetCellUV(CellIndex, VariantIndex);
		OutMaxUV = OutMinUV + CellUVSize;
	}
};

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
	 * 在世界坐标位置设置体素（支持砖块类型和旋转）
	 * @param WorldPosition 世界坐标位置（厘米）
	 * @param TextureID 纹理ID（0表示删除）
	 * @param Layer 体素层
	 * @param BlockType 砖块类型（0=方块, 1=斜面, 2=三角斜面）
	 * @param Roll 绕头顶垂线旋转 (0-3，对应0°, 90°, 180°, 270°)
	 * @param Pitch 俯仰角 (Slope: 0-2, TriSlope: 0-1)
	 * @param World 世界对象
	 * @param bUpdateMesh 是否立即更新网格
	 * @return 是否成功设置
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	bool SetVoxelAtWorldPosition(const FIntVector& WorldPosition, uint8 TextureID = 0, uint8 Layer = 1, uint8 BlockType = 0, uint8 Roll = 0, uint8 Pitch = 0, UWorld* World = nullptr, bool bUpdateMesh = true);

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

	/**
	 * 设置纹理列表（从 MapManager 的 TextureConfig 加载）
	 * @param TexturePaths 纹理路径列表
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	void SetTextureList(const TArray<FString>& TexturePaths);

	/**
	 * 获取纹理列表
	 * @return 纹理列表（UTexture2D* 数组）
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	TArray<UTexture2D*> GetTextureList() const { return TextureList; }

	/**
	 * 根据 TextureID 获取对应的纹理
	 * @param TextureID 纹理ID（从 AryTextureID 获取）
	 * @return 对应的纹理，如果ID无效则返回nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	UTexture2D* GetTextureByID(int32 TextureID) const;

	/**
	 * 根据 TextureID 获取对应的 War3 纹理结构信息
	 * @param TextureID 纹理ID
	 * @return War3 纹理结构信息
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	FWar3TextureInfo GetTextureInfoByID(int32 TextureID) const;

	/**
	 * 获取纹理结构信息列表
	 * @return 纹理结构信息列表
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	TArray<FWar3TextureInfo> GetTextureInfoList() const { return TextureInfoList; }

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

	/** 纹理列表（从 MapManager 的 TextureConfig 加载） */
	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> TextureList;

	/** 纹理结构信息列表（与 TextureList 一一对应） */
	UPROPERTY()
	TArray<FWar3TextureInfo> TextureInfoList;
};

