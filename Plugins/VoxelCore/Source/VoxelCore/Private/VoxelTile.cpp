/********************************************************************
created:	2024/12/XX
filename: 	VoxelTile.cpp
author:		Auto Generated

purpose:	VoxelTile 实现
*********************************************************************/
#include "VoxelTile.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"

enum
{
	UCPIXEL_KIND_NONE0 = 0x00,

	UCPIXEL_KIND_HALF0 = 0x01,
	UCPIXEL_KIND_HALF1 = 0x02,
	UCPIXEL_KIND_HALF2 = 0x03,
	UCPIXEL_KIND_HALF3 = 0x04,

	UCPIXEL_KIND_FULL0 = 0x05
};

// 七个面的方向向量（从VoxelTerrain复制）
// 0=Left, 1=Front, 2=Right, 3=Back, 4=Top, 5=Bottom, 6=Slope(斜面)
const FIntVector FaceDirections[6] = {
	FIntVector( 0, -1,  0), // 0: Left (面向-Y方向)
	FIntVector( 1,  0,  0), // 1: Front (面向+X方向)
	FIntVector( 0,  1,  0), // 2: Right (面向+Y方向)
	FIntVector(-1,  0,  0), // 3: Back (面向-X方向)
	FIntVector( 0,  0,  1), // 4: Top (面向+Z方向)
	FIntVector( 0,  0, -1)  // 5: Bottom (面向-Z方向)
};

// 七个面的法向量
const FVector FaceNormals[6] = {
	FVector( 0, -1,  0), // 0: Left
	FVector( 1,  0,  0), // 1: Front
	FVector( 0,  1,  0), // 2: Right
	FVector(-1,  0,  0), // 3: Back
	FVector( 0,  0,  1), // 4: Top
	FVector( 0,  0, -1)  // 5: Bottom
};

// 基础立方体的8个顶点位置（单位立方体）
// Unreal坐标系：X=前(+X前,-X后), Y=右(+Y右,-Y左), Z=上(+Z上,-Z下)
// 顶点索引：
//   4---5
//  /|  /|
// 7---6 |
// | 0-|-1
// |/  |/
// 3---2
static const FVector BaseCubeVertices[8] = {
	FVector(0.0f, 0.0f, 0.0f), // 0: 前左下 (X=0前, Y=0左, Z=0下)
	FVector(1.0f, 0.0f, 0.0f), // 1: 后左下 (X=1后, Y=0左, Z=0下)
	FVector(1.0f, 1.0f, 0.0f), // 2: 后右下 (X=1后, Y=1右, Z=0下)
	FVector(0.0f, 1.0f, 0.0f), // 3: 前右下 (X=0前, Y=1右, Z=0下)
	FVector(0.0f, 0.0f, 1.0f), // 4: 前左上 (X=0前, Y=0左, Z=1上)
	FVector(1.0f, 0.0f, 1.0f), // 5: 后左上 (X=1后, Y=0左, Z=1上)
	FVector(1.0f, 1.0f, 1.0f), // 6: 后右上 (X=1后, Y=1右, Z=1上)
	FVector(0.0f, 1.0f, 1.0f)  // 7: 前右上 (X=0前, Y=1右, Z=1上)
};

struct FaceIndices {
	int32 Indices[4]; // 最多4个顶点
	int32 Count;      // 实际顶点数（3或4）
};

static const FaceIndices BoxFaces[6] = {
	// 0: Left (Y=0，面向-Y方向)
	{ {+0, +1, +5, +4}, 4 },
	// 1: Front (X=1，面向+X方向)
	{ {+1, +2, +6, +5}, 4 },
	// 2: Right (Y=1，面向+Y方向)
	{ {+2, +3, +7, +6}, 4 },
	// 3: Back (X=0，面向-X方向)
	{ {+3, +0, +4, +7}, 4 },
	// 4: Top (Z=1，面向+Z方向)
	{ {+4, +5, +6, +7}, 4 },
	// 5: Bottom (Z=0，面向-Z方向)
	{ {+0, +3, +2, +1}, 4 }
};

static const FaceIndices SlopeFaces[7] = {
	// 0: Left (Y=0) - 不存在（被斜面覆盖）
	{ {-1, -1, -1, -1}, 0 },
	// 1: Front (X=1) - 3个顶点（三角形）
	{ {+0, +7, +3, -1}, 3 },
	// 2: Right (Y=1) - 4个顶点
	{ {+2, +3, +7, +6}, 4 },
	// 3: Back (X=0) - 3个顶点（三角形）
	{ {+2, +6, +1, -1}, 3 },
	// 4: Top (Z=1) - 不存在（被斜面替代）
	{ {-1, -1, -1, -1}, 0 },
	// 5: Bottom (Z=0) - 4个顶点（正方形）
	{ {+0, +3, +2, +1}, 4 },
	// 6: Slope (斜面) - 4个顶点（倾斜四边形）
	{ {+0, +1, +6, +7}, 4 }
};

static const FaceIndices TriSlopeFaces[7] = {
	// 0: Left (Y=0) - 不存在（被切掉）
	{ {-1, -1, -1, -1}, 0 },
	// 1: Front (X=1) - 3个顶点（三角形）
	{ {-1, -1, -1, -1}, 0 },
	// 2: Right (Y=1) - 3个顶点（三角形）
	{ {+3, +6, +2, -1}, 3 },
	// 3: Back (X=0) - 3个顶点（三角形）
	{ {+2, +6, +1, -1}, 3 },
	// 4: Top (Z=1) - 不存在（被三个侧面替代）
	{ {-1, -1, -1, -1}, 0 },
	// 5: Bottom (Z=0) - 3个顶点（三角形，切掉0角）
	{ {+3, +2, +1, -1}, 3 },
	// 6: Slope (斜面) - 不存在
	{ {+3, +1, +6, -1}, 3 }
};

static const FaceIndices TriSlopeComplementFaces[7] = {
	// 0: Left (Y=0) - 4个顶点（但顶点4不存在，所以是3个顶点的三角形）
	{ {+0, +1, +5, -1}, 3 },
	// 1: Front (X=1) - 4个顶点
	{ {+1, +2, +6, +5}, 4 },
	// 2: Right (Y=1) - 4个顶点
	{ {+2, +3, +7, +6}, 4 },
	// 3: Back (X=0) - 3个顶点（三角形，切角）
	{ {+0, +7, +3, -1}, 3 },
	// 4: Top (Z=1) - 不存在（被斜面替代）
	{ {+7, +5, +6, -1}, 3 },
	// 5: Bottom (Z=0) - 4个顶点（正方形）
	{ {+0, +1, +2, +3}, 4 },
	// 6: Slope (斜面) 
	{ {+0, +5, +7, -1}, 3 }
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

	return X >= 0 && X < VOXEL_TILE_SIZE_X &&
		   Y >= 0 && Y < VOXEL_TILE_SIZE_Y &&
		   Z >= 0 && Z < VOXEL_TILE_SIZE_Z;
}

bool AVoxelTile::IsVoxelEmpty(int32 X, int32 Y, int32 Z) const
{
	// 如果坐标超出边界，视为空（用于边界面的渲染）
	if (!IsValidVoxelCoord(X, Y, Z))
		return true;

	// 检查体素的Layer是否为Null
	UCVoxelData Voxel = GetVoxel(X, Y, Z);
	return Voxel.LayerID == UCVoxelData_Layer_Null;
}

int32 AVoxelTile::VoxelCoordToIndex(int32 X, int32 Y, int32 Z) const
{
	return Z * VOXEL_TILE_SIZE_Y * VOXEL_TILE_SIZE_X + Y * VOXEL_TILE_SIZE_X + X;
}

void AVoxelTile::SetVoxelWithBlockType(int32 X, int32 Y, int32 Z, uint8 TextureID, uint8 Layer, uint8 BlockType, uint8 Roll, uint8 Pitch, bool bUpdateMesh)
{
	if (!bIsActive)
		return;
	if (!IsValidVoxelCoord(X, Y, Z))
		return;

	int32 Index = VoxelCoordToIndex(X, Y, Z);

	UCVoxelData VoxelData;
	VoxelData.TextureID = TextureID;
	VoxelData.LayerID = Layer;
	VoxelData.Type = BlockType & 0x03;  // 确保只有2位
	UCVoxelData_SetYawAndRoll(VoxelData, Roll & 0x03, Pitch & 0x03); // Roll参数实际上是Yaw，Pitch参数实际上是Roll
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

void AVoxelTile::AddFace(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, bool bFlat)
{
	if (FaceIndex < 0 || FaceIndex >= 7)
		return;

	// 计算体素在世界空间中的位置（Unreal中Z向上）
	FVector BasePos = FVector(X * VoxelSize - (VOXEL_TILE_SIZE_X / 2) * VoxelSize, Y * VoxelSize - (VOXEL_TILE_SIZE_Y / 2) * VoxelSize, Z * VoxelSize - (VOXEL_TILE_SIZE_Z / 2) * VoxelSize);

	// 获取当前面的法向量
	FVector Normal = FaceNormals[FaceIndex];

	// 获取当前面的顶点索引定义（Box使用统一的7个面结构）
	const FaceIndices& FaceDef = BoxFaces[FaceIndex];
	int32 NumVerts = FaceDef.Count;
	
	// 如果面不存在（Count为0），直接返回
	if (NumVerts == 0)
		return;

	// 计算当前面的基础索引（用于三角形索引）
	int32 BaseIndex = Vertices.Num();

	if (bFlat)
	{
		// 简单模式：4顶点 + 2三角形（原来的方式）
		for (int32 i = 0; i < NumVerts; ++i)
		{
			int32 VertexIndex = FaceDef.Indices[i];
			FVector LocalVertex = BaseCubeVertices[VertexIndex];
			FVector VertexOffset = LocalVertex * VoxelSize;
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
			if (Voxel.TextureID > 0)
			{
				uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
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
		FVector Center = FVector::ZeroVector;
		for (int32 i = 0; i < NumVerts; ++i)
		{
			int32 VertexIndex = FaceDef.Indices[i];
			Center += BaseCubeVertices[VertexIndex];
		}
		Center = Center / NumVerts * VoxelSize;
		FVector CenterPos = BasePos + Center;
		
		// 获取方向向量用于检查相邻体素
		FIntVector Direction = FaceDirections[FaceIndex];
		
		// 添加4个角顶点 + 1个中心顶点
		for (int32 i = 0; i < 5; ++i)
		{
			FVector VertexPos;
			FVector2D UV;
			
			if (i < NumVerts)
			{
				// 角顶点
				int32 VertexIndex = FaceDef.Indices[i];
				FVector LocalVertex = BaseCubeVertices[VertexIndex];
				FVector VertexOffset = LocalVertex * VoxelSize;
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
				
				if (Voxel.TextureID > 0)
				{
					uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
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
				if (Voxel.TextureID > 0)
				{
					uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
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
		uint8 CurrentLayer = CurrentVoxel.LayerID;
	
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
			if (PlaneAdjVoxel.LayerID != CurrentLayer)
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
	for (int32 Z = 0; Z < VOXEL_TILE_SIZE_Z; ++Z)
	{
		for (int32 X = 0; X < VOXEL_TILE_SIZE_X; ++X)
		{
			for (int32 Y = 0; Y < VOXEL_TILE_SIZE_Y; ++Y)
			{
				UCVoxelData Voxel = GetVoxel(X, Y, Z);
				if (Voxel.LayerID == UCVoxelData_Layer_Null)
					continue;

				// 获取体素类型和旋转
				uint8 BlockType = Voxel.Type & 0x03;
				uint8 Roll = UCVoxelData_GetRoll(Voxel);
				uint8 Yaw = UCVoxelData_GetYaw(Voxel);
				
				// 根据图素类型直接调用对应的Add函数
				switch (BlockType)
				{
					case UCVoxelBlockType_Cube:
						AddBoxFace(X, Y, Z, Voxel);
						break;
					case UCVoxelBlockType_SquareSlope:
						AddSquareSlopeFace(X, Y, Z, Voxel);
						break;
					case UCVoxelBlockType_TriangularSlope:
						AddTriangularSlopeFace(X, Y, Z, Voxel);
						break;
					case UCVoxelBlockType_TriangularComplement:
						AddTriangularComplementFace(X, Y, Z, Voxel);
						break;
				}
			}
		}
	}
}

void AVoxelTile::AddBoxFace(int32 X, int32 Y, int32 Z, const UCVoxelData& Voxel)
{
	for (int32 FaceIndex = 0; FaceIndex < 6; ++FaceIndex)
	{
		FIntVector Direction = FaceDirections[FaceIndex];
		int32 AdjX = X + Direction.X;
		int32 AdjY = Y + Direction.Y;
		int32 AdjZ = Z + Direction.Z;

		// 检查是否应该渲染这个面
		if (!IsVoxelEmpty(AdjX, AdjY, AdjZ))
		{
			// 相邻体素存在，不渲染（被遮挡）
			continue;
		}

		// 检查面是否存在（Count是否为0）
		const FaceIndices& FaceDef = BoxFaces[FaceIndex];
		if (FaceDef.Count == 0)
			continue;

		// 检查面是否可以合并（周围都平）
		bool bFlat = IsFaceFlat(X, Y, Z, FaceIndex);
		
		// 渲染该面
		AddFace(X, Y, Z, FaceIndex, Voxel, bFlat);
	}
}

void AVoxelTile::AddSquareSlopeFace(int32 X, int32 Y, int32 Z, const UCVoxelData& Voxel)
{
	// 遍历所有7个面
	for (int32 FaceIndex = 0; FaceIndex < 7; ++FaceIndex)
	{
// 		FIntVector Direction = FaceDirections[FaceIndex];
// 		int32 AdjX = X + Direction.X;
// 		int32 AdjY = Y + Direction.Y;
// 		int32 AdjZ = Z + Direction.Z;

		// 检查面是否存在（Count是否为0）
		const FaceIndices& FaceDef = SlopeFaces[FaceIndex];
		if (FaceDef.Count == 0)
			continue;

		// 渲染该面
		AddSquareSlopeFaceSingle(X, Y, Z, FaceIndex, Voxel);
	}
}

void AVoxelTile::AddSquareSlopeFaceSingle(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel)
{
	if (FaceIndex < 0 || FaceIndex >= 7)
		return;
		
	// 计算体素在世界空间中的位置
	FVector BasePos = FVector(X * VoxelSize - (VOXEL_TILE_SIZE_X / 2) * VoxelSize, Y * VoxelSize - (VOXEL_TILE_SIZE_Y / 2) * VoxelSize, Z * VoxelSize - (VOXEL_TILE_SIZE_Z / 2) * VoxelSize);
	
	int32 BaseIndex = Vertices.Num();
	
	// 获取面的顶点索引定义（使用统一的7个面结构）
	const FaceIndices& FaceDef = SlopeFaces[FaceIndex];
	int32 NumVerts = FaceDef.Count;
	
	// 如果面不存在（Count为0），直接返回
	if (NumVerts == 0)
		return;
	
	// 计算顶点位置
	FVector Verts[4];
	for (int32 i = 0; i < NumVerts; ++i)
	{
		int32 VertexIndex = FaceDef.Indices[i];
		FVector LocalVertex = BaseCubeVertices[VertexIndex];
		// 对于楔形体，顶点6需要特殊处理（从(1,1,1)保持不变，因为是从(0,0,0)到(1,1,1)的楔形）
		Verts[i] = BasePos + LocalVertex * VoxelSize;
	}
	
	// 计算法向量
	FVector Normal;
	if (NumVerts == 3)
	{
		FVector Edge1 = Verts[1] - Verts[0];
		FVector Edge2 = Verts[2] - Verts[0];
		Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
	}
	else
	{
		FVector Edge1 = Verts[1] - Verts[0];
		FVector Edge2 = Verts[3] - Verts[0];
		Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
	}
	
	// 添加顶点
	for (int32 i = 0; i < NumVerts; ++i)
	{
		Vertices.Add(Verts[i]);
		Normals.Add(Normal);
		
		// UV坐标
		FVector LocalVertex = BaseCubeVertices[FaceDef.Indices[i]];
		FVector2D UV = FVector2D(LocalVertex.X, LocalVertex.Y);
		UVs.Add(UV);
		
		// 顶点颜色
		FColor Color = FColor::White;
		if (Voxel.TextureID > 0)
		{
			uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
			Color = FColor(Gray, Gray, Gray, 255);
		}
		VertexColors.Add(Color);
		
		// 切线
		Tangents.Add(FProcMeshTangent(0, 0, 1));
	}
	
	// 添加三角形
	if (NumVerts == 3)
	{
		Triangles.Add(BaseIndex + 0);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 1);
	}
	else
	{
		// 四边形：两个三角形
		Triangles.Add(BaseIndex + 0);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 1);
		
		Triangles.Add(BaseIndex + 0);
		Triangles.Add(BaseIndex + 3);
		Triangles.Add(BaseIndex + 2);
	}
}

void AVoxelTile::AddTriangularSlopeFace(int32 X, int32 Y, int32 Z, const UCVoxelData& Voxel)
{
	// 遍历所有7个面
	for (int32 FaceIndex = 0; FaceIndex < 7; ++FaceIndex)
	{
// 		FIntVector Direction = FaceDirections[FaceIndex];
// 		int32 AdjX = X + Direction.X;
// 		int32 AdjY = Y + Direction.Y;
// 		int32 AdjZ = Z + Direction.Z;

		// 检查面是否存在（Count是否为0）
		const FaceIndices& FaceDef = TriSlopeFaces[FaceIndex];
		if (FaceDef.Count == 0)
			continue;

		// 渲染该面
		AddTriangularSlopeFaceSingle(X, Y, Z, FaceIndex, Voxel);
	}
}

void AVoxelTile::AddTriangularSlopeFaceSingle(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel)
{
	if (FaceIndex < 0 || FaceIndex >= 7)
		return;
	
	// 计算体素在世界空间中的位置
	FVector BasePos = FVector(X * VoxelSize - (VOXEL_TILE_SIZE_X / 2) * VoxelSize, Y * VoxelSize - (VOXEL_TILE_SIZE_Y / 2) * VoxelSize, Z * VoxelSize - (VOXEL_TILE_SIZE_Z / 2) * VoxelSize);
	
	int32 BaseIndex = Vertices.Num();
	
	// 获取面的顶点索引定义（使用统一的7个面结构）
	const FaceIndices& FaceDef = TriSlopeFaces[FaceIndex];
	int32 NumVerts = FaceDef.Count;
	
	// 如果面不存在（Count为0），直接返回
	if (NumVerts == 0)
		return;
	
	// 计算顶点位置
	FVector Verts[3];
	for (int32 i = 0; i < NumVerts; ++i)
	{
		int32 VertexIndex = FaceDef.Indices[i];
		FVector LocalVertex = BaseCubeVertices[VertexIndex];
		Verts[i] = BasePos + LocalVertex * VoxelSize;
	}
	
	// 计算三角形法向量
	FVector Edge1 = Verts[1] - Verts[0];
	FVector Edge2 = Verts[2] - Verts[0];
	FVector TriNormal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
	
	// 添加3个顶点
	for (int32 i = 0; i < NumVerts; ++i)
	{
		Vertices.Add(Verts[i]);
		Normals.Add(TriNormal);
		
		// UV坐标
		FVector LocalVertex = BaseCubeVertices[FaceDef.Indices[i]];
		FVector2D UV;
		if (FaceIndex < 4) // 侧面
		{
			if (FaceIndex == 0 || FaceIndex == 2) // Left/Right面
			{
				UV = FVector2D(LocalVertex.X, LocalVertex.Z);
			}
			else // Front/Back面
			{
				UV = FVector2D(LocalVertex.Y, LocalVertex.Z);
			}
		}
		else // Top/Bottom面
		{
			UV = FVector2D(LocalVertex.X, LocalVertex.Y);
		}
		UVs.Add(UV);
		
		// 顶点颜色
		FColor Color = FColor::White;
		if (Voxel.TextureID > 0)
		{
			uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
			Color = FColor(Gray, Gray, Gray, 255);
		}
		VertexColors.Add(Color);
		
		// 切线
		Tangents.Add(FProcMeshTangent(0, 0, 1));
	}
	
	// 添加一个三角形
	Triangles.Add(BaseIndex + 0);
	Triangles.Add(BaseIndex + 2);
	Triangles.Add(BaseIndex + 1);
}

void AVoxelTile::AddTriangularComplementFace(int32 X, int32 Y, int32 Z, const UCVoxelData& Voxel)
{
	// 遍历所有7个面
	for (int32 FaceIndex = 0; FaceIndex < 7; ++FaceIndex)
	{
// 		FIntVector Direction = FaceDirections[FaceIndex];
// 		int32 AdjX = X + Direction.X;
// 		int32 AdjY = Y + Direction.Y;
// 		int32 AdjZ = Z + Direction.Z;

		// 检查面是否存在（Count是否为0）
		const FaceIndices& FaceDef = TriSlopeComplementFaces[FaceIndex];
		if (FaceDef.Count == 0)
			continue;

		// 渲染该面
		AddTriangularComplementFaceSingle(X, Y, Z, FaceIndex, Voxel);
	}
}

void AVoxelTile::AddTriangularComplementFaceSingle(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel)
{
	if (FaceIndex < 0 || FaceIndex >= 7)
		return;
	
	// 计算体素在世界空间中的位置
	FVector BasePos = FVector(X * VoxelSize - (VOXEL_TILE_SIZE_X / 2) * VoxelSize, Y * VoxelSize - (VOXEL_TILE_SIZE_Y / 2) * VoxelSize, Z * VoxelSize - (VOXEL_TILE_SIZE_Z / 2) * VoxelSize);
	
	int32 BaseIndex = Vertices.Num();
	
	// 获取面的顶点索引定义（使用统一的7个面结构）
	const FaceIndices& FaceDef = TriSlopeComplementFaces[FaceIndex];
	int32 NumVerts = FaceDef.Count;
	
	// 如果面不存在（Count为0），直接返回
	if (NumVerts == 0)
		return;
	
	// 计算顶点位置
	FVector Verts[4];
	for (int32 i = 0; i < NumVerts; ++i)
	{
		int32 VertexIndex = FaceDef.Indices[i];
		FVector LocalVertex = BaseCubeVertices[VertexIndex];
		Verts[i] = BasePos + LocalVertex * VoxelSize;
	}
	
	// 计算法向量
	FVector Normal;
	if (NumVerts == 3)
	{
		FVector Edge1 = Verts[1] - Verts[0];
		FVector Edge2 = Verts[2] - Verts[0];
		Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
	}
	else
	{
		FVector Edge1 = Verts[1] - Verts[0];
		FVector Edge2 = Verts[3] - Verts[0];
		Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
	}
	
	// 添加顶点
	for (int32 i = 0; i < NumVerts; ++i)
	{
		Vertices.Add(Verts[i]);
		Normals.Add(Normal);
		
		// UV坐标
		FVector LocalVertex = BaseCubeVertices[FaceDef.Indices[i]];
		FVector2D UV = FVector2D(LocalVertex.X, LocalVertex.Y);
		UVs.Add(UV);
		
		// 顶点颜色
		FColor Color = FColor::White;
		if (Voxel.TextureID > 0)
		{
			uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
			Color = FColor(Gray, Gray, Gray, 255);
		}
		VertexColors.Add(Color);
		
		// 切线
		Tangents.Add(FProcMeshTangent(0, 0, 1));
	}
	
	// 添加三角形
	if (NumVerts == 3)
	{
		Triangles.Add(BaseIndex + 0);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 1);
	}
	else
	{
		// 四边形：两个三角形
		Triangles.Add(BaseIndex + 0);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 1);
		
		Triangles.Add(BaseIndex + 0);
		Triangles.Add(BaseIndex + 3);
		Triangles.Add(BaseIndex + 2);
	}
}

bool AVoxelTile::DoFacesOverlap(int32 X, int32 Y, int32 Z, int32 FaceIndex, int32 AdjX, int32 AdjY, int32 AdjZ) const
{
	// 获取当前体素和相邻体素的数据
	UCVoxelData CurrentVoxel = GetVoxel(X, Y, Z);
	UCVoxelData AdjVoxel = GetVoxel(AdjX, AdjY, AdjZ);
	
	// 如果相邻体素为空，面不重叠
	if (AdjVoxel.LayerID == UCVoxelData_Layer_Null)
		return false;
	
	// 如果Layer不同，面不重叠
	if (CurrentVoxel.LayerID != AdjVoxel.LayerID)
		return false;
	
	// 获取体素类型和旋转
	uint8 CurrentBlockType = CurrentVoxel.Type & 0x03;
	uint8 CurrentRoll = UCVoxelData_GetRoll(CurrentVoxel);
	uint8 CurrentYaw = UCVoxelData_GetYaw(CurrentVoxel);
	uint8 AdjBlockType = AdjVoxel.Type & 0x03;
	uint8 AdjRoll = UCVoxelData_GetRoll(AdjVoxel);
	uint8 AdjYaw = UCVoxelData_GetYaw(AdjVoxel);
	
	// 对面索引（当前面的法线方向对应的相邻体的面）
	int32 OppositeFaceIndex = -1;
	if (FaceIndex == 6)
	{
		// 斜面：暂时使用Top的对立面
		OppositeFaceIndex = 5; // Slope -> Bottom
	}
	else
	{
		switch (FaceIndex)
		{
			case 0: OppositeFaceIndex = 2; break; // Left -> Right
			case 1: OppositeFaceIndex = 3; break; // Front -> Back
			case 2: OppositeFaceIndex = 0; break; // Right -> Left
			case 3: OppositeFaceIndex = 1; break; // Back -> Front
			case 4: OppositeFaceIndex = 5; break; // Top -> Bottom
			case 5: OppositeFaceIndex = 4; break; // Bottom -> Top
		}
	}
	
	// 如果是普通方块，对面也是普通方块，则完全重叠
	if (CurrentBlockType == UCVoxelBlockType_Cube && AdjBlockType == UCVoxelBlockType_Cube)
	{
		return true; // 两个都是普通方块，完全重叠
	}
	
	// 对于斜面和三角斜面，需要具体判断面的形状
	if (FaceIndex == 4) // 顶部面
	{
		if (CurrentBlockType == UCVoxelBlockType_SquareSlope)
		{
			// 当前是方斜面的顶部（斜面）
			if (AdjBlockType == UCVoxelBlockType_Cube)
			{
				// 相邻是普通方块（正方形面），完全重叠
				return true;
			}
			else if (AdjBlockType == UCVoxelBlockType_SquareSlope)
			{
				// 相邻也是方斜面，检查旋转是否匹配
				// 如果旋转相同，面完全重叠
				if (CurrentYaw == AdjYaw && CurrentRoll == AdjRoll)
					return true;
				// 否则需要更复杂的几何计算来判断是否重叠
				// 这里简化：如果旋转相对（相差180度模4），可能不重叠
				// 实际上需要根据具体的斜面形状来判断
				// 暂时假设相同旋转才重叠
			}
			else if (AdjBlockType == UCVoxelBlockType_TriangularSlope)
			{
				// 相邻是三角斜面，面的形状不同，可能部分重叠
				// 这里简化处理：认为不重叠（需要渲染）
				return false;
			}
		}
		else if (CurrentBlockType == UCVoxelBlockType_TriangularSlope)
		{
			// 当前是三角斜面的顶部（三角形）
			if (AdjBlockType == UCVoxelBlockType_Cube)
			{
				// 相邻是普通方块（正方形面），可能部分重叠
				// 三角斜面的三角形是正方形的一部分，所以可能有重叠区域
				// 这里简化：认为部分重叠，需要更精确的几何计算
				// 暂时返回false（不重叠），让渲染系统处理
				return false;
			}
			else if (AdjBlockType == UCVoxelBlockType_SquareSlope)
			{
				// 相邻是方斜面，形状不同
				return false;
			}
			else if (AdjBlockType == UCVoxelBlockType_TriangularSlope)
			{
				// 相邻也是三角斜面，如果旋转相同且切掉的角互补，可能完全重叠
				// 这里简化：只有旋转相同时才认为可能重叠
				if (CurrentYaw == AdjYaw && CurrentRoll == AdjRoll)
				{
					// 需要检查切掉的角是否在对面
					// 暂时返回false，让渲染系统处理
					return false;
				}
				return false;
			}
		}
	}
	else if (FaceIndex == 5) // 底部面
	{
		// 底部面对于斜面和三角斜面通常是完整的（正方形）
		if (AdjBlockType == UCVoxelBlockType_Cube)
		{
			return true; // 都是正方形，完全重叠
		}
		// 对于斜面，底部面是完整的正方形
		return true; // 简化处理
	}
	else // 侧面（0-3）
	{
		// 对于方斜面，侧面通常是完整的（正方形或矩形）
		// 对于三角斜面，侧面可能被切掉一部分
		
		if (CurrentBlockType == UCVoxelBlockType_SquareSlope)
		{
			// 方斜面的侧面通常是完整的
			if (AdjBlockType == UCVoxelBlockType_Cube)
				return true; // 完全重叠
			else if (AdjBlockType == UCVoxelBlockType_SquareSlope)
				return true; // 假设完全重叠
			else
				return false; // 三角斜面可能有不同的形状
		}
		else if (CurrentBlockType == UCVoxelBlockType_TriangularSlope)
		{
			// 三角斜面的侧面可能部分被切掉
			// 需要根据旋转判断哪个侧面被切掉
			// 这里简化处理：根据旋转和面索引判断
			
			// 三角斜面切掉的角对应的侧面会被切掉一部分
			// 需要具体计算，暂时返回false（不重叠）
			if (AdjBlockType == UCVoxelBlockType_Cube)
			{
				// 相邻是普通方块，可能有部分重叠
				// 需要更精确的几何计算
				return false;
			}
			else
			{
				return false; // 简化处理
			}
		}
	}
	
	// 默认情况：不重叠
	return false;
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

	const int32 HalfTileSizeX = VOXEL_TILE_SIZE_X / 2; // 16
	const int32 HalfTileSizeY = VOXEL_TILE_SIZE_Y / 2; // 16
	const int32 HalfTileSizeZ = VOXEL_TILE_SIZE_Z / 2; // 32

	// 获取Tile的世界位置（Tile中心）
	FVector TileWorldPos = GetActorLocation();
	
	// Tile的边界（世界坐标）：从-16*100到+16*100 (X和Y)，从0到64*100 (Z)
	FVector TileMin = TileWorldPos - FVector(HalfTileSizeX * VoxelSize, HalfTileSizeY * VoxelSize, 0.0f);
	FVector TileMax = TileWorldPos + FVector(HalfTileSizeX * VoxelSize, HalfTileSizeY * VoxelSize, VOXEL_TILE_SIZE_Z * VoxelSize);
	
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

	const int32 HalfTileSizeX = VOXEL_TILE_SIZE_X / 2; // 16
	const int32 HalfTileSizeY = VOXEL_TILE_SIZE_Y / 2; // 16
	const int32 HalfTileSizeZ = VOXEL_TILE_SIZE_Z / 2; // 32

	// 获取Tile的世界位置（Tile中心）
	FVector TileWorldPos = GetActorLocation();
	
	// 先做AABB检测
	float TMin = 0.0f;
	float TMax = FLT_MAX;
	
	FVector TileMin = TileWorldPos - FVector(HalfTileSizeX * VoxelSize, HalfTileSizeY * VoxelSize, 0.0f);
	FVector TileMax = TileWorldPos + FVector(HalfTileSizeX * VoxelSize, HalfTileSizeY * VoxelSize, VOXEL_TILE_SIZE_Z * VoxelSize);
	
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
	const int32 MaxSteps = VOXEL_TILE_SIZE_X * VOXEL_TILE_SIZE_Y * VOXEL_TILE_SIZE_Z;
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
		if (Voxel.LayerID != UCVoxelData_Layer_Null)
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

