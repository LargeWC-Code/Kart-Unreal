/********************************************************************
created:	2024/12/XX
filename: 	VoxelTerrain.h
author:		Auto Generated

purpose:	VoxelTerrain - 体素地形渲染类，参考UCPixelWorld实现
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VoxelTerrain.generated.h"

// 体素数据结构
USTRUCT(BlueprintType)
struct VOXELCORE_API FVoxelData
{
	GENERATED_BODY()

	// 体素类型（0表示空，非0表示有体素）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Type;

	// 体素层（可用于多层地形）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Layer;

	FVoxelData()
		: Type(0)
		, Layer(0)
	{
	}

	FVoxelData(uint8 InType, uint8 InLayer)
		: Type(InType)
		, Layer(InLayer)
	{
	}

	bool IsEmpty() const { return Type == 0; }
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
 * VoxelTerrain - 体素地形Actor
 * 参考UCPixelWorld实现，使用UProceduralMeshComponent进行网格渲染
 * 
 * 使用说明：
 * 1. 在编辑器中创建VoxelTerrain Actor
 * 2. 设置TerrainSize和VoxelSize属性
 * 3. 可以选择性地设置Material（如果为空，会使用默认材质）
 * 4. 在代码或蓝图中调用SetVoxel或FillRegion来填充体素数据
 * 5. 调用UpdateMesh来更新网格渲染
 * 
 * 自定义Shader：
 * 由于使用UProceduralMeshComponent，shader通过Material来应用。
 * 可以在编辑器中创建Material，并在Material Editor中使用Custom节点来编写自定义shader代码。
 * 或者使用Material Parameters来传递额外的数据（如体素类型、光照等）。
 */
UCLASS(BlueprintType, Blueprintable)
class VOXELCORE_API AVoxelTerrain : public AActor
{
	GENERATED_BODY()

public:
	AVoxelTerrain(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ========== 体素数据操作 ==========
	
	/**
	 * 设置体素数据
	 * @param X, Y, Z 体素坐标
	 * @param Type 体素类型（0表示空）
	 * @param Layer 体素层
	 * @param bUpdateMesh 是否立即更新网格
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	void SetVoxel(int32 X, int32 Y, int32 Z, uint8 Type, uint8 Layer = 0, bool bUpdateMesh = true);

	/**
	 * 获取体素数据
	 * @param X, Y, Z 体素坐标
	 * @return 体素数据，如果坐标无效返回空体素
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	FVoxelData GetVoxel(int32 X, int32 Y, int32 Z) const;

	/**
	 * 填充区域
	 * @param MinPos 最小位置
	 * @param MaxPos 最大位置
	 * @param Type 体素类型
	 * @param Layer 体素层
	 * @param bUpdateMesh 是否立即更新网格
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	void FillRegion(const FIntVector& MinPos, const FIntVector& MaxPos, uint8 Type, uint8 Layer = 0, bool bUpdateMesh = true);

	/**
	 * 更新网格渲染（重建所有可见面）
	 */
	UFUNCTION(BlueprintCallable, Category = "VoxelTerrain")
	void UpdateMesh();

	// ========== 配置属性 ==========

	/** 地形尺寸（体素数量） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTerrain")
	FIntVector TerrainSize;

	/** 体素大小（世界单位） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTerrain")
	float VoxelSize;

	/** 材质（如果为空则使用默认材质） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelTerrain")
	UMaterialInterface* Material;

private:
	// ========== 内部方法 ==========

	/** 初始化地形数据 */
	void InitializeTerrain();

	/** 检查体素坐标是否有效 */
	bool IsValidVoxelCoord(int32 X, int32 Y, int32 Z) const;

	/** 将体素坐标转换为数组索引 */
	int32 VoxelCoordToIndex(int32 X, int32 Y, int32 Z) const;

	/** 将数组索引转换为体素坐标 */
	FIntVector IndexToVoxelCoord(int32 Index) const;

	/** 检查指定位置的体素是否为空（包括边界检查） */
	bool IsVoxelEmpty(int32 X, int32 Y, int32 Z) const;

	/** 构建网格数据 */
	void BuildMeshData();

	/** 为指定面添加顶点（简化版本，先实现基本立方体） */
	void AddFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, const FVoxelData& Voxel);

	// 六个面的方向向量
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
	TArray<FVoxelData> VoxelData;

	/** 是否需要在下一帧更新网格 */
	bool bNeedsMeshUpdate;

	/** 网格重建时使用的临时数据 */
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
};

