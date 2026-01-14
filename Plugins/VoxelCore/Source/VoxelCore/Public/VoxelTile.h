/********************************************************************
created:	2024/12/XX
filename: 	VoxelTile.h
author:		Auto Generated

purpose:	VoxelTile - 体素地块Actor，每个地块包含32*32*64的单元格
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VoxelMap.h"
#include "VoxelTile.generated.h"

/**
 * VoxelTile - 体素地块Actor
 * 每个地块包含32*32*64的单元格
 * 地块是AActor，可以在世界中放置和管理
 */
UCLASS(BlueprintType, Blueprintable)
class VOXELCORE_API AVoxelTile : public AActor
{
	GENERATED_BODY()

public:
	AVoxelTile(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 是否激活 */
	bool bIsActive;

	// ========== 地块坐标 ==========
	
	/** 地块坐标（X, Y） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTile")
	FIntPoint TileCoord;

	// ========== 体素数据操作 ==========
	
	/**
	 * 设置体素数据（支持砖块类型和旋转）
	 * @param X, Y, Z 体素坐标（相对于地块，0-31, 0-31, 0-63）
	 * @param TextureID 纹理ID
	 * @param Layer 体素层
	 * @param BlockType 砖块类型（0=方块, 1=斜面, 2=三角斜面）
	 * @param Roll 绕头顶垂线旋转 (0-3，对应0°, 90°, 180°, 270°)
	 * @param Pitch 俯仰角 (Slope: 0-2, TriSlope: 0-1)
	 * @param bUpdateMesh 是否立即更新网格
	 */
	void SetVoxelWithBlockType(int32 X, int32 Y, int32 Z, uint8 TextureID, uint8 Layer, uint8 BlockType, uint8 Roll, uint8 Pitch, bool bUpdateMesh = true);

	/**
	 * 获取体素数据
	 * @param X, Y, Z 体素坐标（相对于地块）
	 * @return 体素数据，如果坐标无效返回空体素
	 */
	UCVoxelData GetVoxel(int32 X, int32 Y, int32 Z) const;

	/**
	 * 将本地存储坐标转换为世界网格坐标
	 * @param LocalGridPosition 本地存储坐标（FIntVector，X/Y: 0-31, Z: 0-63）
	 * @return 世界网格坐标（FIntVector，整数坐标）
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTile")
	inline FIntVector LocalToWorldPosition(const FIntVector& LocalGridPosition) const
	{
		static const int32 HalfTileSizeX = VOXEL_TILE_SIZE_X / 2; // 16
		static const int32 HalfTileSizeY = VOXEL_TILE_SIZE_Y / 2; // 16
		static const int32 HalfTileSizeZ = VOXEL_TILE_SIZE_Z / 2; // 32

		// 计算体素的世界网格坐标（Tile世界网格位置 + 局部偏移）
		return FIntVector(
			TileCoord.Y * VOXEL_TILE_SIZE_X + LocalGridPosition.X - HalfTileSizeX,
			TileCoord.X * VOXEL_TILE_SIZE_Y + LocalGridPosition.Y - HalfTileSizeY,
			LocalGridPosition.Z - HalfTileSizeZ
		);
	}

	/**
	 * 对三个顶点进行排序（按 Z、X、Y 顺序）
	 * @param V0, V1, V2 三个顶点坐标
	 * @return 排序后的三角形（FIntTriangle）
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTile")
	static FIntTriangle SortTriangleVertices(const FIntVector& V0, const FIntVector& V1, const FIntVector& V2);

	/**
	 * 获取相邻体素的对应面信息
	 * @param FaceIndex 当前面ID (0=Left, 1=Front, 2=Right, 3=Back, 4=Top, 5=Bottom)
	 * @param OutNeighborOffset 输出：相邻体素的偏移量（相对于当前体素）
	 * @return 相邻体素对应面的ID
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTile")
	static int32 GetNeighborFaceInfo(int32 FaceIndex);

	/**
	 * 初始化TileData（从地图数据中加载）
	 * @param TileDataFromMap 从地图数据中获取的TileData
	 */
	void SetTileData(struct UCVoxelTileData* TileDataFromMap)	{	TileData = TileDataFromMap;	}

	/**
	 * 获取TileData（用于修改纹理信息等）
	 * @return TileData指针，如果未初始化则返回nullptr
	 */
	struct UCVoxelTileData* GetTileData() const { return TileData; }

	/**
	 * 更新网格渲染（重建所有可见面）
	 * 注意：此函数使用节流机制，1秒内只能激活一次
	 */
	void UpdateMesh(bool Active);

	/**
	 * 激活/取消激活地块
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTile")
	void SetActive(bool bActive);

	/**
	 * 检查地块是否激活
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTile")
	bool IsActive() const { return bIsActive; }

	/**
	 * 快速AABB检测：判断射线是否与Tile的AABB相交
	 * @param RayOrigin 射线起点（世界坐标）
	 * @param RayDirection 射线方向（归一化）
	 * @param OutTMin 输出的最小t值（如果相交）
	 * @return 是否与AABB相交
	 */
	bool IntersectAABB(const FVector& RayOrigin, const FVector& RayDirection, float& OutTMin) const;

	/**
	 * 射线与Tile的交集检测（参考UCPixelBlock::LayerIntersect）
	 * @param RayOrigin 射线起点（世界坐标）
	 * @param RayDirection 射线方向（归一化）
	 * @param OutHitDistance 输出的hit距离
	 * @param OutLocalVoxelPos 输出的hit体素位置（局部坐标，存储坐标0-31/0-31/0-63）
	 * @return 是否找到hit
	 */
	bool Intersect(const FVector& RayOrigin, const FVector& RayDirection, float& OutHitDistance, FIntVector& OutLocalVoxelPos) const;

	// ========== 配置属性 ==========

	/** 体素大小（世界单位，默认100厘米=1米） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTile")
	float VoxelSize;

	/** 材质（如果为空则使用默认材质） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTile")
	UMaterialInterface* Material;

	/** 设置 Terrain 引用（用于获取纹理列表） */
	void SetTerrain(class UVoxelTerrain* InTerrain) { Terrain = InTerrain; }

	/**
	 * 设置纹理到材质（使用 Material Instance Dynamic）
	 * @param TextureID 纹理ID（从 AryTextureID 获取）
	 * @param Texture 要设置的纹理
	 */
	void SetTextureToMaterial(int32 TextureID, UTexture2D* Texture);

	/**
	 * 根据 TextureID 获取对应的纹理
	 * @param TextureID 纹理ID
	 * @return 对应的纹理，如果ID无效或Terrain为null则返回nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTile")
	UTexture2D* GetTextureByID(int32 TextureID) const;

	/**
	 * 根据 TextureID 获取对应的 War3 纹理结构信息
	 * @param TextureID 纹理ID
	 * @return War3 纹理结构信息
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTile")
	struct FWar3TextureInfo GetTextureInfoByID(int32 TextureID) const;

private:
	// ========== 内部方法 ==========

	/** 检查体素坐标是否有效 */
	bool IsValidVoxelCoord(int32 X, int32 Y, int32 Z) const;

	/** 检查体素是否为空（坐标超出边界或Layer为Null） */
	bool IsVoxelEmpty(int32 X, int32 Y, int32 Z) const;

	/** 将体素坐标转换为数组索引 */
	int32 VoxelCoordToIndex(int32 X, int32 Y, int32 Z) const;

	void ClearMeshData();
	/** 构建网格数据 */
	void BuildMeshData();
	
	/** 获取面的所有三角形（FIntTriangle数组） */
	void GetFaceTriangles(int32 X, int32 Y, int32 Z, int32 FaceIndex, uint8 BlockType, int32 DirectionIndex, const UCVoxelData& Voxel, bool bFlat, TArray<FIntVertex>& OutVertices, FVector& Normal, TArray<FIntVector>& OutFaceIndexes) const;

	/** 检查面的四个角是否都有相邻体素在同一高度（可用于合并） */
	bool IsFaceFlat(int32 X, int32 Y, int32 Z, int32 FaceIndex) const;
	
	/** 实际执行网格更新的函数（由定时器调用） */
	void ExecuteMeshUpdate();

private:
	// ========== 组件 ==========
	
	/** 程序化网格组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProceduralMeshComponent* ProceduralMesh;

	// ========== 数据 ==========

	/** 体素数据数组（线性存储：Index = Z * SizeY * SizeX + Y * SizeX + X） */
	UCVoxelTileData*	TileData;

	/** Terrain 引用（用于获取纹理列表） */
	UPROPERTY()
	TObjectPtr<class UVoxelTerrain> Terrain;

	/** Material Instance Dynamic（用于运行时设置纹理） */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic;

	/** 网格重建时使用的临时数据（按TextureID分组） */
	struct FMeshSectionData
	{
		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UVs0; // UV0 - 第一层
		TArray<FVector2D> UVs1; // UV1 - 第二层
		TArray<FVector2D> UVs2; // UV2 - 第三层
		TArray<FVector2D> UVs3; // UV3 - 第四层
		TArray<FColor> VertexColors;
		TArray<FProcMeshTangent> Tangents;
		
		void Clear()
		{
			Vertices.Empty();
			Triangles.Empty();
			Normals.Empty();
			UVs0.Empty();
			UVs1.Empty();
			UVs2.Empty();
			UVs3.Empty();
			VertexColors.Empty();
			Tangents.Empty();
		}
	};
	
	/** 按TextureID分组的网格数据（key是4个顶点TextureID排序后组合成的int64） */
	TMap<int64, FMeshSectionData> MeshSections;

	// ========== 网格更新节流 ==========
	
	/** 是否已请求刷新（激活标志） */
	bool bMeshUpdatePending;
		
	/** 定时器句柄（用于延迟刷新） */
	FTimerHandle MeshUpdateTimerHandle;
};

