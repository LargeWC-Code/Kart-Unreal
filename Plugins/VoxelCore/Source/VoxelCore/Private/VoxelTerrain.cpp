/********************************************************************
created:	2024/12/XX
filename: 	VoxelTerrain.cpp
author:		Auto Generated

purpose:	VoxelTerrain 实现
*********************************************************************/
#include "VoxelTerrain.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"

// 六个面的方向向量（Unreal坐标系统：X=Forward, Y=Right, Z=Up）
const FIntVector AVoxelTerrain::FaceDirections[6] = {
	FIntVector( 0, -1,  0), // Left (面向-Y方向)
	FIntVector( 1,  0,  0), // Front (面向+X方向)
	FIntVector( 0,  1,  0), // Right (面向+Y方向)
	FIntVector(-1,  0,  0), // Back (面向-X方向)
	FIntVector( 0,  0,  1), // Top (面向+Z方向)
	FIntVector( 0,  0, -1)  // Bottom (面向-Z方向)
};

// 六个面的法向量
const FVector AVoxelTerrain::FaceNormals[6] = {
	FVector( 0, -1,  0), // Left
	FVector( 1,  0,  0), // Front
	FVector( 0,  1,  0), // Right
	FVector(-1,  0,  0), // Back
	FVector( 0,  0,  1), // Top
	FVector( 0,  0, -1)  // Bottom
};

// 六个面的顶点位置（相对于体素原点，单位立方体）
// Unreal坐标系统：X=Forward, Y=Right, Z=Up
const FVector AVoxelTerrain::FaceVertices[6][4] = {
	// Left face (Y=0，面向-Y方向)
	{
		FVector(0, 0, 0), // 左下
		FVector(1, 0, 0), // 右下
		FVector(1, 0, 1), // 右上
		FVector(0, 0, 1)  // 左上
	},
	// Front face (X=1，面向+X方向)
	{
		FVector(1, 0, 0), // 左下
		FVector(1, 1, 0), // 右下
		FVector(1, 1, 1), // 右上
		FVector(1, 0, 1)  // 左上
	},
	// Right face (Y=1，面向+Y方向)
	{
		FVector(1, 1, 0), // 左下
		FVector(0, 1, 0), // 右下
		FVector(0, 1, 1), // 右上
		FVector(1, 1, 1)  // 左上
	},
	// Back face (X=0，面向-X方向)
	{
		FVector(0, 1, 0), // 左下
		FVector(0, 0, 0), // 右下
		FVector(0, 0, 1), // 右上
		FVector(0, 1, 1)  // 左上
	},
	// Top face (Z=1，面向+Z方向)
	{
		FVector(0, 0, 1), // 左下
		FVector(1, 0, 1), // 右下
		FVector(1, 1, 1), // 右上
		FVector(0, 1, 1)  // 左上
	},
	// Bottom face (Z=0，面向-Z方向)
	{
		FVector(0, 1, 0), // 左下
		FVector(1, 1, 0), // 右下
		FVector(1, 0, 0), // 右上
		FVector(0, 0, 0)  // 左上
	}
};

AVoxelTerrain::AVoxelTerrain(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TerrainSize(32, 32, 32)
	, VoxelSize(100.0f)
	, Material(nullptr)
	, bNeedsMeshUpdate(false)
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建程序化网格组件
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;
	
	// 设置碰撞
	ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AVoxelTerrain::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeTerrain();
	UpdateMesh();
}

void AVoxelTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bNeedsMeshUpdate)
	{
		UpdateMesh();
		bNeedsMeshUpdate = false;
	}
}

void AVoxelTerrain::InitializeTerrain()
{
	int32 TotalVoxels = TerrainSize.X * TerrainSize.Y * TerrainSize.Z;
	VoxelData.SetNum(TotalVoxels);
	
	// 初始化为空
	for (int32 i = 0; i < TotalVoxels; ++i)
	{
		VoxelData[i] = FVoxelData(0, 0);
	}
}

bool AVoxelTerrain::IsValidVoxelCoord(int32 X, int32 Y, int32 Z) const
{
	return X >= 0 && X < TerrainSize.X &&
		   Y >= 0 && Y < TerrainSize.Y &&
		   Z >= 0 && Z < TerrainSize.Z;
}

int32 AVoxelTerrain::VoxelCoordToIndex(int32 X, int32 Y, int32 Z) const
{
	return Z * TerrainSize.Y * TerrainSize.X + Y * TerrainSize.X + X;
}

FIntVector AVoxelTerrain::IndexToVoxelCoord(int32 Index) const
{
	FIntVector Coord;
	Coord.X = Index % TerrainSize.X;
	Coord.Y = (Index / TerrainSize.X) % TerrainSize.Y;
	Coord.Z = Index / (TerrainSize.X * TerrainSize.Y);
	return Coord;
}

bool AVoxelTerrain::IsVoxelEmpty(int32 X, int32 Y, int32 Z) const
{
	if (!IsValidVoxelCoord(X, Y, Z))
		return true; // 边界外视为空
	
	int32 Index = VoxelCoordToIndex(X, Y, Z);
	return VoxelData[Index].IsEmpty();
}

void AVoxelTerrain::SetVoxel(int32 X, int32 Y, int32 Z, uint8 Type, uint8 Layer, bool bUpdateMesh)
{
	if (!IsValidVoxelCoord(X, Y, Z))
		return;

	int32 Index = VoxelCoordToIndex(X, Y, Z);
	VoxelData[Index] = FVoxelData(Type, Layer);

	if (bUpdateMesh)
	{
		bNeedsMeshUpdate = true;
	}
}

FVoxelData AVoxelTerrain::GetVoxel(int32 X, int32 Y, int32 Z) const
{
	if (!IsValidVoxelCoord(X, Y, Z))
		return FVoxelData(0, 0);

	int32 Index = VoxelCoordToIndex(X, Y, Z);
	return VoxelData[Index];
}

void AVoxelTerrain::FillRegion(const FIntVector& MinPos, const FIntVector& MaxPos, uint8 Type, uint8 Layer, bool bUpdateMesh)
{
	for (int32 X = MinPos.X; X <= MaxPos.X; ++X)
	{
		for (int32 Y = MinPos.Y; Y <= MaxPos.Y; ++Y)
		{
			for (int32 Z = MinPos.Z; Z <= MaxPos.Z; ++Z)
			{
				SetVoxel(X, Y, Z, Type, Layer, false);
			}
		}
	}

	if (bUpdateMesh)
	{
		bNeedsMeshUpdate = true;
	}
}

void AVoxelTerrain::AddFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, const FVoxelData& Voxel)
{
	if (FaceIndex < 0 || FaceIndex >= 6)
		return;

	// 计算体素在世界空间中的位置（Unreal中Z向上）
	FVector BasePos = FVector(X * VoxelSize, Y * VoxelSize, Z * VoxelSize);

	// 获取当前面的法向量
	FVector Normal = FaceNormals[FaceIndex];

	// 获取当前面的四个顶点
	const FVector* FaceVerts = FaceVertices[FaceIndex];

	// 计算当前面的基础索引（用于三角形索引）
	int32 BaseIndex = Vertices.Num();

	// 添加四个顶点
	for (int32 i = 0; i < 4; ++i)
	{
		FVector VertexOffset = FaceVerts[i] * VoxelSize;
		FVector VertexPos = BasePos + VertexOffset;
		Vertices.Add(VertexPos);
		Normals.Add(Normal);
		
		// UV坐标（简单的平铺，可以根据体素类型调整）
		// 将局部坐标转换为UV（0-1范围）
		FVector2D UV;
		if (FaceIndex < 4) // 侧面
		{
			// 侧面：根据面类型使用不同的坐标轴
			if (FaceIndex == 0 || FaceIndex == 2) // Left/Right面：使用X和Z
			{
				UV = FVector2D(VertexOffset.X / VoxelSize, VertexOffset.Z / VoxelSize);
			}
			else // Front/Back面：使用Y和Z
			{
				UV = FVector2D(VertexOffset.Y / VoxelSize, VertexOffset.Z / VoxelSize);
			}
		}
		else // Top/Bottom面：使用X和Y
		{
			UV = FVector2D(VertexOffset.X / VoxelSize, VertexOffset.Y / VoxelSize);
		}
		UVs.Add(UV);

		// 顶点颜色（可以根据体素类型设置不同颜色）
		FColor Color = FColor::White;
		if (Voxel.Type > 0)
		{
			// 根据类型设置颜色（简单的示例）
			uint8 Gray = FMath::Clamp(Voxel.Type * 255 / 255, (uint8)64, (uint8)255);
			Color = FColor(Gray, Gray, Gray, 255);
		}
		VertexColors.Add(Color);

		// 切线（简化为零）
		Tangents.Add(FProcMeshTangent(0, 0, 1));
	}

	// 添加两个三角形（0-1-2 和 0-2-3）
	Triangles.Add(BaseIndex + 0);
	Triangles.Add(BaseIndex + 1);
	Triangles.Add(BaseIndex + 2);

	Triangles.Add(BaseIndex + 0);
	Triangles.Add(BaseIndex + 2);
	Triangles.Add(BaseIndex + 3);
}

void AVoxelTerrain::BuildMeshData()
{
	// 清空之前的网格数据
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UVs.Empty();
	VertexColors.Empty();
	Tangents.Empty();

	// 遍历所有体素，为每个可见面生成几何体
	for (int32 X = 0; X < TerrainSize.X; ++X)
	{
		for (int32 Y = 0; Y < TerrainSize.Y; ++Y)
		{
			for (int32 Z = 0; Z < TerrainSize.Z; ++Z)
			{
				FVoxelData Voxel = GetVoxel(X, Y, Z);
				if (Voxel.IsEmpty())
					continue;

				// 检查六个面，只生成朝向空的面的几何体
				for (int32 FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
				{
					FIntVector Direction = FaceDirections[FaceIndex];
					int32 AdjX = X + Direction.X;
					int32 AdjY = Y + Direction.Y;
					int32 AdjZ = Z + Direction.Z;

					// 如果相邻体素为空，则添加这个面
					if (IsVoxelEmpty(AdjX, AdjY, AdjZ))
					{
						AddFace(X, Y, Z, FaceIndex, Voxel);
					}
				}
			}
		}
	}
}

void AVoxelTerrain::UpdateMesh()
{
	// 构建网格数据
	BuildMeshData();

	// 如果没有顶点，创建空网格
	if (Vertices.Num() == 0)
	{
		ProceduralMesh->ClearMeshSection(0);
		return;
	}

	// 更新程序化网格组件
	ProceduralMesh->CreateMeshSection(
		0,
		Vertices,
		Triangles,
		Normals,
		UVs,
		VertexColors,
		Tangents,
		true // 启用碰撞
	);

	// 设置材质
	if (Material)
	{
		ProceduralMesh->SetMaterial(0, Material);
	}
}


