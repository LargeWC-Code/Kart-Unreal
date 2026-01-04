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

void AVoxelTile::SetTileData(UCVoxelTileData* TileDataFromMap)
{
	TileData = TileDataFromMap;
}

void AVoxelTile::SetActive(bool bActive)
{
	bIsActive = bActive;
	
	// 更新网格
	UpdateMesh(bIsActive);
}

void AVoxelTile::AddFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, bool bFlat)
{
	if (FaceIndex < 0 || FaceIndex >= 6)
		return;

	// 计算体素在世界空间中的位置（Unreal中Z向上）
	FVector BasePos = FVector(X * VoxelSize - (VOXEL_TILE_SIZE_X / 2) * VoxelSize, Y * VoxelSize - (VOXEL_TILE_SIZE_Y / 2) * VoxelSize, Z * VoxelSize - (VOXEL_TILE_SIZE_Z / 2) * VoxelSize);

	// 获取当前面的法向量
	FVector Normal = FaceNormals[FaceIndex];

	// 获取当前面的四个顶点
	const FVector* FaceVerts = FaceVertices[FaceIndex];

	// 计算当前面的基础索引（用于三角形索引）
	int32 BaseIndex = Vertices.Num();

	if (bFlat)
	{
		// 简单模式：4顶点 + 2三角形（原来的方式）
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

			// 顶点颜色（简单模式：白色）
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
	else
	{
		// AO模式：5顶点（4个角+1个中心）+ 4个三角形
		// 获取面的切向量
		FVector Tangent1, Tangent2;
		if (FaceIndex < 4) // 侧面
		{
			Tangent1 = FVector(0, 0, 1); // Z方向
			if (FaceIndex == 0 || FaceIndex == 2) // Left/Right
				Tangent2 = FVector(1, 0, 0); // X方向
			else // Front/Back
				Tangent2 = FVector(0, 1, 0); // Y方向
		}
		else // Top/Bottom
		{
			Tangent1 = FVector(1, 0, 0); // X方向
			Tangent2 = FVector(0, 1, 0); // Y方向
		}
		
		// 计算中心点
		FVector Center = (FaceVerts[0] + FaceVerts[1] + FaceVerts[2] + FaceVerts[3]) * 0.25f * VoxelSize;
		FVector CenterPos = BasePos + Center;
		
		// 获取方向向量用于检查相邻体素
		FIntVector Direction = FaceDirections[FaceIndex];
		
		// 添加4个角顶点 + 1个中心顶点
		for (int32 i = 0; i < 5; ++i)
		{
			FVector VertexPos;
			FVector2D UV;
			
			if (i < 4)
			{
				// 角顶点
				FVector VertexOffset = FaceVerts[i] * VoxelSize;
				VertexPos = BasePos + VertexOffset;
				
				// UV坐标（角顶点在边界）
				if (FaceIndex < 4) // 侧面
				{
					if (FaceIndex == 0 || FaceIndex == 2) // Left/Right面
					{
						UV = FVector2D(VertexOffset.X / VoxelSize, VertexOffset.Z / VoxelSize);
					}
					else // Front/Back面
					{
						UV = FVector2D(VertexOffset.Y / VoxelSize, VertexOffset.Z / VoxelSize);
					}
				}
				else // Top/Bottom面
				{
					UV = FVector2D(VertexOffset.X / VoxelSize, VertexOffset.Y / VoxelSize);
				}
			}
			else
			{
				// 中心顶点
				VertexPos = CenterPos;
				
				// UV坐标（中心点）
				if (FaceIndex < 4) // 侧面
				{
					if (FaceIndex == 0 || FaceIndex == 2) // Left/Right面
					{
						UV = FVector2D(0.5f, 0.5f);
					}
					else // Front/Back面
					{
						UV = FVector2D(0.5f, 0.5f);
					}
				}
				else // Top/Bottom面
				{
					UV = FVector2D(0.5f, 0.5f);
				}
			}
			
			Vertices.Add(VertexPos);
			Normals.Add(Normal);
			UVs.Add(UV);
			
			// 顶点颜色（AO）
			FColor Color = FColor::White;
			if (i < 4)
			{
				// 角顶点：检查是否有相邻体素，如果有则添加阴影
				FIntVector CornerOffset;
				if (i == 0) // 左下
					CornerOffset = FIntVector(-1, -1, 0);
				else if (i == 1) // 右下
					CornerOffset = FIntVector(1, -1, 0);
				else if (i == 2) // 右上
					CornerOffset = FIntVector(1, 1, 0);
				else // 左上
					CornerOffset = FIntVector(-1, 1, 0);
				
				// 调整CornerOffset到正确的坐标系
				if (FaceIndex < 4) // 侧面
				{
					if (FaceIndex == 0) // Left: CornerOffset应该是(-1,0,-1), (1,0,-1), (1,0,1), (-1,0,1)在XZ平面
					{
						if (i == 0) CornerOffset = FIntVector(-1, 0, -1);
						else if (i == 1) CornerOffset = FIntVector(1, 0, -1);
						else if (i == 2) CornerOffset = FIntVector(1, 0, 1);
						else CornerOffset = FIntVector(-1, 0, 1);
					}
					else if (FaceIndex == 1) // Front
					{
						if (i == 0) CornerOffset = FIntVector(0, -1, -1);
						else if (i == 1) CornerOffset = FIntVector(0, 1, -1);
						else if (i == 2) CornerOffset = FIntVector(0, 1, 1);
						else CornerOffset = FIntVector(0, -1, 1);
					}
					else if (FaceIndex == 2) // Right
					{
						if (i == 0) CornerOffset = FIntVector(1, 0, -1);
						else if (i == 1) CornerOffset = FIntVector(-1, 0, -1);
						else if (i == 2) CornerOffset = FIntVector(-1, 0, 1);
						else CornerOffset = FIntVector(1, 0, 1);
					}
					else // Back
					{
						if (i == 0) CornerOffset = FIntVector(0, 1, -1);
						else if (i == 1) CornerOffset = FIntVector(0, -1, -1);
						else if (i == 2) CornerOffset = FIntVector(0, -1, 1);
						else CornerOffset = FIntVector(0, 1, 1);
					}
				}
				else if (FaceIndex == 4) // Top
				{
					if (i == 0) CornerOffset = FIntVector(-1, -1, 1);
					else if (i == 1) CornerOffset = FIntVector(1, -1, 1);
					else if (i == 2) CornerOffset = FIntVector(1, 1, 1);
					else CornerOffset = FIntVector(-1, 1, 1);
				}
				else // Bottom
				{
					if (i == 0) CornerOffset = FIntVector(-1, 1, -1);
					else if (i == 1) CornerOffset = FIntVector(1, 1, -1);
					else if (i == 2) CornerOffset = FIntVector(1, -1, -1);
					else CornerOffset = FIntVector(-1, -1, -1);
				}
				
				// 检查对角相邻体素是否存在
				int32 DiagX = X + Direction.X + CornerOffset.X;
				int32 DiagY = Y + Direction.Y + CornerOffset.Y;
				int32 DiagZ = Z + Direction.Z + CornerOffset.Z;
				
				// 如果对角相邻体素存在，添加阴影（降低亮度）
				if (!IsVoxelEmpty(DiagX, DiagY, DiagZ))
				{
					// 角落有阴影：使用较暗的颜色（约0.6-0.7的亮度）
					uint8 AOValue = 160; // 约0.63的亮度
					Color = FColor(AOValue, AOValue, AOValue, 255);
				}
				else
				{
					Color = FColor::White;
				}
				
				if (Voxel.Type > 0)
				{
					uint8 Gray = FMath::Clamp(Voxel.Type * 255 / 255, (uint8)64, (uint8)255);
					Color = FColor(
						FMath::Min((uint32)Color.R * Gray / 255, 255u),
						FMath::Min((uint32)Color.G * Gray / 255, 255u),
						FMath::Min((uint32)Color.B * Gray / 255, 255u),
						255
					);
				}
			}
			else
			{
				// 中心顶点：无阴影，保持白色
				if (Voxel.Type > 0)
				{
					uint8 Gray = FMath::Clamp(Voxel.Type * 255 / 255, (uint8)64, (uint8)255);
					Color = FColor(Gray, Gray, Gray, 255);
				}
			}
			
			VertexColors.Add(Color);
			
			// 切线
			Tangents.Add(FProcMeshTangent(0, 0, 1));
		}
		
		// 添加4个三角形（从中心到每个角，连接相邻的角，逆时针顺序确保外表面正确渲染）
		// 三角形0: 中心, 角0, 角1
		Triangles.Add(BaseIndex + 4); // 中心
		Triangles.Add(BaseIndex + 1); // 角0
		Triangles.Add(BaseIndex + 0); // 角1
		
		// 三角形1: 中心, 角1, 角2
		Triangles.Add(BaseIndex + 4); // 中心
		Triangles.Add(BaseIndex + 2); // 角1
		Triangles.Add(BaseIndex + 1); // 角2
		
		// 三角形2: 中心, 角2, 角3
		Triangles.Add(BaseIndex + 4); // 中心
		Triangles.Add(BaseIndex + 3); // 角2
		Triangles.Add(BaseIndex + 2); // 角3
		
		// 三角形3: 中心, 角3, 角0
		Triangles.Add(BaseIndex + 4); // 中心
		Triangles.Add(BaseIndex + 0); // 角3
		Triangles.Add(BaseIndex + 3); // 角0
	}
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

bool AVoxelTile::IsFaceFlat(int32 X, int32 Y, int32 Z, int32 FaceIndex) const
{
	// 检查相邻的8个体素是否都与当前体素在同一Layer
	// 只有完全在一个平面上的（相邻8个体素都相同Layer，且面的法线方向为空）才使用2个三角形
	// 其他情况都使用5个顶点（4个三角形）
	
	FIntVector Direction = FaceDirections[FaceIndex];
	
	// 获取当前体素的Layer
	UCVoxelData CurrentVoxel = GetVoxel(X, Y, Z);
	uint8 CurrentLayer = CurrentVoxel.Layer;
	
	// 检查面的法线方向的相邻体素是否为空（这是前提条件，只有外表面才会被渲染）
	int32 FaceAdjX = X + Direction.X;
	int32 FaceAdjY = Y + Direction.Y;
	int32 FaceAdjZ = Z + Direction.Z;
	if (!IsVoxelEmpty(FaceAdjX, FaceAdjY, FaceAdjZ))
	{
		return false; // 面的法线方向有体素，这个面不应该被渲染
	}
	
	// 检查面的平面上（法线方向上偏移一个单位）的相邻8个体素
	// 对于任意面，都要检查在法线方向上偏移后，该面的3x3区域的8个体素是否都存在且Layer相同
	
	// 确定两个切向量（与法向量垂直的两个方向）
	FIntVector Axis1, Axis2;
	if (FaceIndex < 4) // 侧面 (Left, Front, Right, Back)
	{
		// 侧面：一个轴是Z，另一个是X或Y
		Axis2 = FIntVector(0, 0, 1); // Z轴
		if (Direction.X != 0) // Front/Back (Direction.X = +/-1)
		{
			Axis1 = FIntVector(0, 1, 0); // Y轴
		}
		else // Left/Right (Direction.Y = +/-1)
		{
			Axis1 = FIntVector(1, 0, 0); // X轴
		}
	}
	else // Top/Bottom
	{
		// 顶面/底面：轴是X和Y
		Axis1 = FIntVector(1, 0, 0); // X轴
		Axis2 = FIntVector(0, 1, 0); // Y轴
	}
	
	// 1. 检查法线方向上偏移一个单位后的3x3区域是否都为空（如果有体素，说明法线方向有体素，不是平的）
	for (int32 Offset1 = -1; Offset1 <= 1; ++Offset1)
	{
		for (int32 Offset2 = -1; Offset2 <= 1; ++Offset2)
		{
			// 跳过中心体素本身
			if (Offset1 == 0 && Offset2 == 0)
				continue;

			// 计算在法线方向上偏移一个单位后，该面的3x3区域中的体素位置
			// 位置 = (X, Y, Z) + Direction + Offset1*Axis1 + Offset2*Axis2
			int32 NormalAdjX = X + Direction.X + Offset1 * Axis1.X + Offset2 * Axis2.X;
			int32 NormalAdjY = Y + Direction.Y + Offset1 * Axis1.Y + Offset2 * Axis2.Y;
			int32 NormalAdjZ = Z + Direction.Z + Offset1 * Axis1.Z + Offset2 * Axis2.Z;
			
			// 如果法线方向上有体素（不空），说明不是平的
			if (!IsVoxelEmpty(NormalAdjX, NormalAdjY, NormalAdjZ))
			{
				return false; // 法线方向有体素，不是平的
			}

			// 计算同一平面上的相邻体素位置（不偏移法线方向）
			// 位置 = (X, Y, Z) + Offset1*Axis1 + Offset2*Axis2
			int32 PlaneAdjX = X + Offset1 * Axis1.X + Offset2 * Axis2.X;
			int32 PlaneAdjY = Y + Offset1 * Axis1.Y + Offset2 * Axis2.Y;
			int32 PlaneAdjZ = Z + Offset1 * Axis1.Z + Offset2 * Axis2.Z;

			UCVoxelData PlaneAdjVoxel = GetVoxel(PlaneAdjX, PlaneAdjY, PlaneAdjZ);
			if (PlaneAdjVoxel.Layer != CurrentLayer)
			{
				return false; // Layer不同，不是平的
			}
		}
	}
	
	return true; // 相邻的8个体素都与当前体素在同一Layer，且面的法线方向为空
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
						// 检查面是否可以合并（周围都平）
						bool bFlat = IsFaceFlat(X, Y, Z, FaceIndex);
						AddFace(X, Y, Z, FaceIndex, Voxel, bFlat);
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

