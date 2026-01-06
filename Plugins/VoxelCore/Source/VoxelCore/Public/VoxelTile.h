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

	/** 地块尺寸（固定为32*32*64） */
	static constexpr int32 TileSizeX = VOXEL_TILE_SIZE_X;
	static constexpr int32 TileSizeY = VOXEL_TILE_SIZE_Y;
	static constexpr int32 TileSizeZ = VOXEL_TILE_SIZE_Z;

	// ========== 地块坐标 ==========
	
	/** 地块坐标（X, Y） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTile")
	FIntPoint TileCoord;

	// ========== 体素数据操作 ==========
	
	/**
	 * 设置体素数据
	 * @param X, Y, Z 体素坐标（相对于地块，0-31, 0-31, 0-63）
	 * @param Type 体素类型（0表示空）
	 * @param Layer 体素层
	 * @param bUpdateMesh 是否立即更新网格
	 */
	void SetVoxel(int32 X, int32 Y, int32 Z, uint8 Type, uint8 Layer = 0, bool bUpdateMesh = true);

	/**
	 * 设置体素数据（支持砖块类型和旋转）
	 * @param X, Y, Z 体素坐标（相对于地块，0-31, 0-31, 0-63）
	 * @param TextureID 纹理ID
	 * @param Layer 体素层
	 * @param BlockType 砖块类型（0=方块, 1=斜面, 2=三角斜面）
	 * @param RotationX 旋转X (0-3，对应0°, 90°, 180°, 270°)
	 * @param RotationY 旋转Y (0-3)
	 * @param RotationZ 旋转Z (0-3)
	 * @param bUpdateMesh 是否立即更新网格
	 */
	void SetVoxelWithBlockType(int32 X, int32 Y, int32 Z, uint8 TextureID, uint8 Layer, uint8 BlockType, uint8 RotationX, uint8 RotationY, uint8 RotationZ, bool bUpdateMesh = true);

	/**
	 * 获取体素数据
	 * @param X, Y, Z 体素坐标（相对于地块）
	 * @return 体素数据，如果坐标无效返回空体素
	 */
	UCVoxelData GetVoxel(int32 X, int32 Y, int32 Z) const;

	/**
	 * 初始化TileData（从地图数据中加载）
	 * @param TileDataFromMap 从地图数据中获取的TileData
	 */
	void SetTileData(struct UCVoxelTileData* TileDataFromMap);

	/**
	 * 更新网格渲染（重建所有可见面）
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
	
	/** 添加方斜面面（根据旋转） */
	void AddSquareSlopeFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, uint8 RotationX, uint8 RotationY, uint8 RotationZ);
	
	/** 添加三角斜面面（根据旋转） */
	void AddTriangularSlopeFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, uint8 RotationX, uint8 RotationY, uint8 RotationZ);
	
	/** 应用旋转变换到顶点 */
	FVector ApplyRotation(const FVector& Vertex, uint8 RotationX, uint8 RotationY, uint8 RotationZ, const FVector& Center);
	
	/** 检查相邻斜面的面是否可以连接（如果连接则不应渲染该面） */
	bool ShouldSkipSlopeFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, uint8 BlockType, uint8 RotationZ, int32 AdjX, int32 AdjY, int32 AdjZ) const;

	/** 为指定面添加顶点 */
	void AddFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, bool bFlat = true);

	/** 检查面的四个角是否都有相邻体素在同一高度（可用于合并） */
	bool IsFaceFlat(int32 X, int32 Y, int32 Z, int32 FaceIndex) const;

	// 六个面的方向向量（从VoxelTerrain复制）
	static const FIntVector FaceDirections[6];
	static const FVector FaceNormals[6];
	static const FVector FaceVertices[6][4];

private:
	// ========== 组件 ==========
	
	/** 程序化网格组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProceduralMeshComponent* ProceduralMesh;

	// ========== 数据 ==========

	/** 体素数据数组（线性存储：Index = Z * SizeY * SizeX + Y * SizeX + X） */
	UCVoxelTileData*	TileData;

	/** 网格重建时使用的临时数据 */
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
};

