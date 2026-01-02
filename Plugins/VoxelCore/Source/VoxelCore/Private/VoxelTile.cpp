/********************************************************************
created:	2024/12/XX
filename: 	VoxelTile.cpp
author:		Auto Generated

purpose:	VoxelTile 实现
*********************************************************************/
#include "VoxelTile.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"

// 六个面的方向向量（从VoxelTerrain复制）
const FIntVector AVoxelTile::FaceDirections[6] = {
	FIntVector( 0, -1,  0), // Left (面向-Y方向)
	FIntVector( 1,  0,  0), // Front (面向+X方向)
	FIntVector( 0,  1,  0), // Right (面向+Y方向)
	FIntVector(-1,  0,  0), // Back (面向-X方向)
	FIntVector( 0,  0,  1), // Top (面向+Z方向)
	FIntVector( 0,  0, -1)  // Bottom (面向-Z方向)
};

// 六个面的法向量
const FVector AVoxelTile::FaceNormals[6] = {
	FVector( 0, -1,  0), // Left
	FVector( 1,  0,  0), // Front
	FVector( 0,  1,  0), // Right
	FVector(-1,  0,  0), // Back
	FVector( 0,  0,  1), // Top
	FVector( 0,  0, -1)  // Bottom
};

// 六个面的顶点位置（相对于体素原点，单位立方体）
const FVector AVoxelTile::FaceVertices[6][4] = {
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

AVoxelTile::AVoxelTile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsActive(false)
	, TileCoord(0, 0)
	, VoxelSize(100.0f)
	, Material(nullptr)
	, TileData(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建程序化网格组件
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;
	
	// 设置碰撞
	ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AVoxelTile::BeginPlay()
{
	Super::BeginPlay();
}

void AVoxelTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AVoxelTile::IsValidVoxelCoord(int32 X, int32 Y, int32 Z) const
{
	if (TileData == nullptr)
		return false;

	int32 index = VoxelCoordToIndex(X, Y, Z);
	if (index < 0 || index >= TileData->AryVoxels.GetSize())
		return false;

	return X >= 0 && X < TileSizeX &&
		   Y >= 0 && Y < TileSizeY &&
		   Z >= 0 && Z < TileSizeZ;
}

bool AVoxelTile::IsVoxelEmpty(int32 X, int32 Y, int32 Z) const
{
	// 如果坐标超出边界，视为空（用于边界面的渲染）
	if (!IsValidVoxelCoord(X, Y, Z))
		return true;

	// 检查体素的Layer是否为Null
	UCVoxelData Voxel = GetVoxel(X, Y, Z);
	return Voxel.Layer == UCVoxelData_Layer_Null;
}

int32 AVoxelTile::VoxelCoordToIndex(int32 X, int32 Y, int32 Z) const
{
	return Z * TileSizeY * TileSizeX + Y * TileSizeX + X;
}

void AVoxelTile::SetVoxel(int32 X, int32 Y, int32 Z, uint8 Type, uint8 Layer, bool bUpdateMesh)
{
	if (!bIsActive)
		return;
	if (!IsValidVoxelCoord(X, Y, Z))
		return;

	int32 Index = VoxelCoordToIndex(X, Y, Z);

	UCVoxelData VoxelData;
	VoxelData.Type = Type;
	VoxelData.Layer = Layer;
	TileData->AryVoxels[Index] = VoxelData.Data;

	if (bUpdateMesh)
		UpdateMesh(true);
}

UCVoxelData AVoxelTile::GetVoxel(int32 X, int32 Y, int32 Z) const
{
	if (!bIsActive)
		return UCVoxelData();
	if (!IsValidVoxelCoord(X, Y, Z))
		return UCVoxelData();

	int32 Index = VoxelCoordToIndex(X, Y, Z);

	UCVoxelData VoxelData;
	VoxelData.Data = TileData->AryVoxels[Index];
	return VoxelData;
}

void AVoxelTile::SetActive(bool bActive)
{
	bIsActive = bActive;
	
	// 更新网格
	UpdateMesh(bIsActive);
}

void AVoxelTile::AddFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel)
{
	if (FaceIndex < 0 || FaceIndex >= 6)
		return;

	// 计算体素在世界空间中的位置（Unreal中Z向上）
	FVector BasePos = FVector(X * VoxelSize - 16 * VoxelSize, Y * VoxelSize - 16 * VoxelSize, Z * VoxelSize - 32 * VoxelSize);

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
		
		// UV坐标
		FVector2D UV;
		if (FaceIndex < 4) // 侧面
		{
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

		// 顶点颜色
		FColor Color = FColor::White;
		if (Voxel.Type > 0)
		{
			uint8 Gray = FMath::Clamp(Voxel.Type * 255 / 255, (uint8)64, (uint8)255);
			Color = FColor(Gray, Gray, Gray, 255);
		}
		VertexColors.Add(Color);

		// 切线
		Tangents.Add(FProcMeshTangent(0, 0, 1));
	}

	// 添加两个三角形（逆时针顺序，确保外表面正确渲染）
	// 第一个三角形
	Triangles.Add(BaseIndex + 0);
	Triangles.Add(BaseIndex + 2);
	Triangles.Add(BaseIndex + 1);

	// 第二个三角形
	Triangles.Add(BaseIndex + 0);
	Triangles.Add(BaseIndex + 3);
	Triangles.Add(BaseIndex + 2);
}

void AVoxelTile::ClearMeshData()
{
	// 清空之前的网格数据
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UVs.Empty();
	VertexColors.Empty();
	Tangents.Empty();
}

void AVoxelTile::BuildMeshData()
{
	// 遍历所有体素，为每个可见面生成几何体
	for (int32 X = 0; X < TileSizeX; ++X)
	{
		for (int32 Y = 0; Y < TileSizeY; ++Y)
		{
			for (int32 Z = 0; Z < TileSizeZ; ++Z)
			{
				UCVoxelData Voxel = GetVoxel(X, Y, Z);
				if (Voxel.Layer == UCVoxelData_Layer_Null)
					continue;

				// 检查六个面，渲染外表面（如果相邻体素为空或者是边界）
				for (int32 FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
				{
					FIntVector Direction = FaceDirections[FaceIndex];
					int32 AdjX = X + Direction.X;
					int32 AdjY = Y + Direction.Y;
					int32 AdjZ = Z + Direction.Z;

					// 如果相邻体素为空或者是边界，则添加这个面（外表面）
					if (IsVoxelEmpty(AdjX, AdjY, AdjZ))
					{
						AddFace(X, Y, Z, FaceIndex, Voxel);
					}
				}
			}
		}
	}
}

void AVoxelTile::UpdateMesh(bool Active)
{
	ClearMeshData();
	if (!Active)
	{
		ProceduralMesh->ClearMeshSection(0);
		return;
	}

	// 构建网格数据
	BuildMeshData();

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

bool AVoxelTile::IntersectAABB(const FVector& RayOrigin, const FVector& RayDirection, float& OutTMin) const
{
	if (!bIsActive)
	{
		return false;
	}

	const int32 HalfTileSizeX = TileSizeX / 2; // 16
	const int32 HalfTileSizeY = TileSizeY / 2; // 16
	const int32 HalfTileSizeZ = TileSizeZ / 2; // 32

	// 获取Tile的世界位置（Tile中心）
	FVector TileWorldPos = GetActorLocation();
	
	// Tile的边界（世界坐标）：从-16*100到+16*100 (X和Y)，从0到64*100 (Z)
	FVector TileMin = TileWorldPos - FVector(HalfTileSizeX * VoxelSize, HalfTileSizeY * VoxelSize, 0.0f);
	FVector TileMax = TileWorldPos + FVector(HalfTileSizeX * VoxelSize, HalfTileSizeY * VoxelSize, TileSizeZ * VoxelSize);
	
	// AABB检测：判断射线是否与Tile相交
	float TMin = 0.0f;
	float TMax = FLT_MAX;
	
	for (int32 i = 0; i < 3; ++i)
	{
		if (FMath::Abs(RayDirection[i]) < 0.0001f)
		{
			// 射线与轴平行
			if (RayOrigin[i] < TileMin[i] || RayOrigin[i] > TileMax[i])
				return false;
		}
		else
		{
			float Ood = 1.0f / RayDirection[i];
			float T1 = (TileMin[i] - RayOrigin[i]) * Ood;
			float T2 = (TileMax[i] - RayOrigin[i]) * Ood;
			
			if (T1 > T2)
			{
				float Temp = T1;
				T1 = T2;
				T2 = Temp;
			}
			
			if (T1 > TMin) TMin = T1;
			if (T2 < TMax) TMax = T2;
			
			if (TMin > TMax)
				return false;
		}
	}
	
	if (TMax < 0.0f)
		return false; // 射线在Tile后面
	
	OutTMin = FMath::Max(0.0f, TMin);
	return true;
}

bool AVoxelTile::Intersect(const FVector& RayOrigin, const FVector& RayDirection, float& OutHitDistance, FIntVector& OutLocalVoxelPos) const
{
	if (!bIsActive)
	{
		return false;
	}

	const int32 HalfTileSizeX = TileSizeX / 2; // 16
	const int32 HalfTileSizeY = TileSizeY / 2; // 16
	const int32 HalfTileSizeZ = TileSizeZ / 2; // 32

	// 获取Tile的世界位置（Tile中心）
	FVector TileWorldPos = GetActorLocation();
	
	// 先做AABB检测
	float TMin = 0.0f;
	float TMax = FLT_MAX;
	
	FVector TileMin = TileWorldPos - FVector(HalfTileSizeX * VoxelSize, HalfTileSizeY * VoxelSize, 0.0f);
	FVector TileMax = TileWorldPos + FVector(HalfTileSizeX * VoxelSize, HalfTileSizeY * VoxelSize, TileSizeZ * VoxelSize);
	
	for (int32 i = 0; i < 3; ++i)
	{
		if (FMath::Abs(RayDirection[i]) < 0.0001f)
		{
			if (RayOrigin[i] < TileMin[i] || RayOrigin[i] > TileMax[i])
				return false;
		}
		else
		{
			float Ood = 1.0f / RayDirection[i];
			float T1 = (TileMin[i] - RayOrigin[i]) * Ood;
			float T2 = (TileMax[i] - RayOrigin[i]) * Ood;
			
			if (T1 > T2)
			{
				float Temp = T1;
				T1 = T2;
				T2 = Temp;
			}
			
			if (T1 > TMin) TMin = T1;
			if (T2 < TMax) TMax = T2;
			
			if (TMin > TMax)
				return false;
		}
	}
	
	if (TMax < 0.0f)
		return false; // 射线在Tile后面
	
	float HitDist = FMath::Max(0.0f, TMin);
	FVector HitEntry = RayOrigin + RayDirection * HitDist;
	
	// 将射线转换到Tile的局部坐标系（Tile中心为原点）
	FVector LocalOrigin = HitEntry - TileWorldPos;
	FVector LocalDir = RayDirection;
	
	// 转换为体素坐标（局部坐标系，从-16到+15）
	int32 StartVoxelX = FMath::FloorToInt(LocalOrigin.X / VoxelSize);
	int32 StartVoxelY = FMath::FloorToInt(LocalOrigin.Y / VoxelSize);
	int32 StartVoxelZ = FMath::FloorToInt(LocalOrigin.Z / VoxelSize);
	
	// 限制在Tile范围内
	StartVoxelX = FMath::Clamp(StartVoxelX, -HalfTileSizeX, HalfTileSizeX - 1);
	StartVoxelY = FMath::Clamp(StartVoxelY, -HalfTileSizeY, HalfTileSizeY - 1);
	StartVoxelZ = FMath::Clamp(StartVoxelZ, -HalfTileSizeZ, HalfTileSizeZ - 1);
	
	// DDA算法：沿着射线方向遍历体素
	int32 VoxelX = StartVoxelX;
	int32 VoxelY = StartVoxelY;
	int32 VoxelZ = StartVoxelZ;
	
	// 计算步长
	float StepX = (LocalDir.X > 0.0f) ? 1.0f : -1.0f;
	float StepY = (LocalDir.Y > 0.0f) ? 1.0f : -1.0f;
	float StepZ = (LocalDir.Z > 0.0f) ? 1.0f : -1.0f;
	
	// 计算到下一个体素边界的距离
	float NextX = (VoxelX + (StepX > 0.0f ? 1.0f : 0.0f)) * VoxelSize;
	float NextY = (VoxelY + (StepY > 0.0f ? 1.0f : 0.0f)) * VoxelSize;
	float NextZ = (VoxelZ + (StepZ > 0.0f ? 1.0f : 0.0f)) * VoxelSize;
	
	float DeltaX = (FMath::Abs(LocalDir.X) > 0.0001f) ? VoxelSize / FMath::Abs(LocalDir.X) : FLT_MAX;
	float DeltaY = (FMath::Abs(LocalDir.Y) > 0.0001f) ? VoxelSize / FMath::Abs(LocalDir.Y) : FLT_MAX;
	float DeltaZ = (FMath::Abs(LocalDir.Z) > 0.0001f) ? VoxelSize / FMath::Abs(LocalDir.Z) : FLT_MAX;
	
	float MaxX = (LocalDir.X != 0.0f) ? (NextX - LocalOrigin.X) / LocalDir.X : FLT_MAX;
	float MaxY = (LocalDir.Y != 0.0f) ? (NextY - LocalOrigin.Y) / LocalDir.Y : FLT_MAX;
	float MaxZ = (LocalDir.Z != 0.0f) ? (NextZ - LocalOrigin.Z) / LocalDir.Z : FLT_MAX;
	
	// 遍历体素，找到第一个非空体素
	const int32 MaxSteps = TileSizeX * TileSizeY * TileSizeZ;
	for (int32 Step = 0; Step < MaxSteps; ++Step)
	{
		// 检查当前体素坐标是否在Tile范围内
		if (VoxelX < -HalfTileSizeX || VoxelX >= HalfTileSizeX ||
			VoxelY < -HalfTileSizeY || VoxelY >= HalfTileSizeY ||
			VoxelZ < -HalfTileSizeZ || VoxelZ >= HalfTileSizeZ)
		{
			break; // 超出Tile范围
		}
		
		// 转换为存储坐标（0到31 for X/Y, 0到63 for Z）
		int32 StorageX = VoxelX + HalfTileSizeX;
		int32 StorageY = VoxelY + HalfTileSizeY;
		int32 StorageZ = VoxelZ + HalfTileSizeZ;
		
		// 检查体素是否非空
		UCVoxelData Voxel = GetVoxel(StorageX, StorageY, StorageZ);
		if (Voxel.Layer != UCVoxelData_Layer_Null)
		{
			// 找到非空体素，使用AABB检测计算精确的hit距离
			FVector VoxelLocalMin = FVector(VoxelX * VoxelSize, VoxelY * VoxelSize, VoxelZ * VoxelSize);
			FVector VoxelWorldMin = TileWorldPos + VoxelLocalMin;
			FVector VoxelWorldMax = VoxelWorldMin + FVector(VoxelSize, VoxelSize, VoxelSize);
			
			// 计算射线与体素AABB的交点
			float VoxelTMin = 0.0f;
			float VoxelTMax = FLT_MAX;
			bool bHitVoxel = true;
			
			for (int32 i = 0; i < 3; ++i)
			{
				if (FMath::Abs(RayDirection[i]) < 0.0001f)
				{
					if (RayOrigin[i] < VoxelWorldMin[i] || RayOrigin[i] > VoxelWorldMax[i])
					{
						bHitVoxel = false;
						break;
					}
				}
				else
				{
					float Ood = 1.0f / RayDirection[i];
					float T1 = (VoxelWorldMin[i] - RayOrigin[i]) * Ood;
					float T2 = (VoxelWorldMax[i] - RayOrigin[i]) * Ood;
					
					if (T1 > T2)
					{
						float Temp = T1;
						T1 = T2;
						T2 = Temp;
					}
					
					if (T1 > VoxelTMin) VoxelTMin = T1;
					if (T2 < VoxelTMax) VoxelTMax = T2;
					
					if (VoxelTMin > VoxelTMax)
					{
						bHitVoxel = false;
						break;
					}
				}
			}
			
			if (bHitVoxel && VoxelTMin >= 0.0f)
			{
				OutHitDistance = VoxelTMin;
				OutLocalVoxelPos = FIntVector(StorageX, StorageY, StorageZ);
				return true;
			}
		}
		
		// 移动到下一个体素
		if (MaxX < MaxY && MaxX < MaxZ)
		{
			MaxX += DeltaX;
			VoxelX += (int32)StepX;
		}
		else if (MaxY < MaxZ)
		{
			MaxY += DeltaY;
			VoxelY += (int32)StepY;
		}
		else
		{
			MaxZ += DeltaZ;
			VoxelZ += (int32)StepZ;
		}
	}
	
	return false;
}

