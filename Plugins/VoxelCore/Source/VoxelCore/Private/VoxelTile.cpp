/********************************************************************
created:	2024/12/XX
filename: 	VoxelTile.cpp
author:		Auto Generated

purpose:	VoxelTile 实现
*********************************************************************/
#include "VoxelTile.h"
#include "VoxelTerrain.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	FIntVector(0, -1,  0), // 0: Left (面向-Y方向)
	FIntVector(-1,  0,  0), // 1: Front (面向-X方向)
	FIntVector(0,  1,  0), // 2: Right (面向+Y方向)
	FIntVector(1,  0,  0), // 3: Back (面向+X方向)
	FIntVector(0,  0,  1), // 4: Top (面向+Z方向)
	FIntVector(0,  0, -1)  // 5: Bottom (面向-Z方向)
};

// 七个面的法向量
const FVector FaceNormals[6] = {
	FVector(0, -1,  0), // 0: Left
	FVector(-1,  0,  0), // 1: Front
	FVector(0,  1,  0), // 2: Right
	FVector(1,  0,  0),  // 3: Back
	FVector(0,  0,  1), // 4: Top
	FVector(0,  0, -1)  // 5: Bottom
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
	FVector(0.0f, 1.0f, 1.0f) // 7: 前右上 (X=0前, Y=1右, Z=1上)
};

// 8个顶点在6个面上的UV坐标缓存 [顶点索引][面索引]
// 面索引：0=Left, 1=Front, 2=Right, 3=Back, 4=Top, 5=Bottom
// Left/Right面：使用X和Z -> (X, Z)
// Front/Back面：使用Y和Z -> (Y, Z)
// Top/Bottom面：使用X和Y -> (X, Y)
static const FIntVector2 BaseCubeUVs[8][6] = {
	// 顶点0: (0,0,0) 前左下
	{ FIntVector2(0, 0), FIntVector2(0, 0), FIntVector2(0, 0), FIntVector2(0, 0), FIntVector2(0, 0), FIntVector2(0, 0) }, // Left, Front, Right, Back, Top, Bottom
	// 顶点1: (1,0,0) 后左下
	{ FIntVector2(1, 0), FIntVector2(0, 0), FIntVector2(1, 0), FIntVector2(0, 0), FIntVector2(1, 0), FIntVector2(1, 0) },
	// 顶点2: (1,1,0) 后右下
	{ FIntVector2(1, 0), FIntVector2(1, 0), FIntVector2(1, 0), FIntVector2(1, 0), FIntVector2(1, 1), FIntVector2(1, 1) },
	// 顶点3: (0,1,0) 前右下
	{ FIntVector2(0, 0), FIntVector2(1, 0), FIntVector2(0, 0), FIntVector2(1, 0), FIntVector2(0, 1), FIntVector2(0, 1) },
	// 顶点4: (0,0,1) 前左上
	{ FIntVector2(0, 1), FIntVector2(0, 1), FIntVector2(0, 1), FIntVector2(0, 1), FIntVector2(0, 0), FIntVector2(0, 0) },
	// 顶点5: (1,0,1) 后左上
	{ FIntVector2(1, 1), FIntVector2(0, 1), FIntVector2(1, 1), FIntVector2(0, 1), FIntVector2(1, 0), FIntVector2(1, 0) },
	// 顶点6: (1,1,1) 后右上
	{ FIntVector2(1, 1), FIntVector2(1, 1), FIntVector2(1, 1), FIntVector2(1, 1), FIntVector2(1, 1), FIntVector2(1, 1) },
	// 顶点7: (0,1,1) 前右上
	{ FIntVector2(0, 1), FIntVector2(1, 1), FIntVector2(0, 1), FIntVector2(1, 1), FIntVector2(0, 1), FIntVector2(0, 1) }
};

struct FaceIndices {
	int32 Indices[4]; // 最多4个顶点
	int32 Count;      // 实际顶点数（3或4）
};

static const FaceIndices BoxFaces[7] = {
	// 0: Left (Y=0，面向-Y方向)
	{ {+0, +1, +5, +4}, 4 },
	// 1: Front (X=1，面向+X方向)
	{ {+3, +0, +4, +7}, 4 },
	// 2: Right (Y=1，面向+Y方向)
	{ {+2, +3, +7, +6}, 4 },
	// 3: Back (X=0，面向-X方向)
	{ {+1, +2, +6, +5}, 4 },
	// 4: Top (Z=1，面向+Z方向)
	{ {+4, +5, +6, +7}, 4 },
	// 5: Bottom (Z=0，面向-Z方向)
	{ {+0, +3, +2, +1}, 4 },
	{ {-1, -1, -1, -1}, 0 }
};

static const FaceIndices SlopeFaces[7] = {
	// 0: Left (Y=0) - 不存在（被斜面覆盖）
	{ {-1, -1, -1, -1}, 0 },
	// 1: Front (X=1) - 3个顶点（三角形）
	{ {+3, +0, +7, +4}, 3 },
	// 2: Right (Y=1) - 4个顶点
	{ {+2, +3, +7, +6}, 4 },
	// 3: Back (X=0) - 3个顶点（三角形）
	{ {+2, +6, +1, +5}, 3 },
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
	{ {+2, +3, +6, +7}, 3 },
	// 3: Back (X=0) - 3个顶点（三角形）
	{ {+2, +6, +1, +5}, 3 },
	// 4: Top (Z=1) - 不存在（被三个侧面替代）
	{ {-1, -1, -1, -1}, 0 },
	// 5: Bottom (Z=0) - 3个顶点（三角形，切掉0角）
	{ {+2, +1, +3, +0}, 3 },
	// 6: Slope (斜面) - 不存在
	{ {+3, +1, +6, -1}, 3 }
};

static const FaceIndices TriSlopeComplementFaces[7] = {
	// 0: Left (Y=0) - 4个顶点（但顶点4不存在，所以是3个顶点的三角形）
	{ {+1, +5, +0, +4}, 3 },
	// 1: Front (X=1) - 4个顶点
	{ {+3, +0, +7, +4}, 3 },
	// 2: Right (Y=1) - 4个顶点
	{ {+2, +3, +7, +6}, 4 },
	// 3: Back (X=0) - 3个顶点（三角形，切角）
	{ {+1, +2, +6, +5}, 4 },
	// 4: Top (Z=1) - ）3个顶点（三角形，切角）
	{ {+6, +7, +5, +4}, 3 },
	// 5: Bottom (Z=0) - 4个顶点（正方形）
	{ {+0, +3, +2, +1}, 4 },
	// 6: Slope (斜面) 
	{ {+0, +5, +7, -1}, 3 }
};

// 多面体形状生成器：基于默认方向的面数据，计算所有旋转方向的顶点数据
class FVoxelBlockShapeGenerator
{
public:
	// 所有块类型的面数据：AllBlockFaces[UCVoxelBlockType][DirectionIndex][FaceIndex]
	// 类型：使用UCVoxelBlockType枚举值
	//   - 0=UCVoxelBlockType_Cube（立方体）：只有1个方向（方向索引0），直接使用全局BoxFaces
	//   - 1=UCVoxelBlockType_SquareSlope（楔形体）：12个方向（Yaw: 0-3, Pitch: 0-2），方向索引 = Yaw * 3 + Pitch
	//   - 2=UCVoxelBlockType_TriangularSlope（三角锥）：8个方向（Yaw: 0-3, Pitch: 0-1），方向索引 = Yaw * 2 + Pitch
	//   - 3=UCVoxelBlockType_TriangularComplement（三角锥互补体）：8个方向（Yaw: 0-3, Pitch: 0-1），方向索引 = Yaw * 2 + Pitch
	// 方向ID：楔形体用0-11，三角锥和三角锥互补体用0-7，立方体用0（以最大12为准）
	// 面ID：0-6（7个面）
	FaceIndices AllBlockFaces[4][12][7];
	
	// 所有块类型的反向面索引映射：AllBlockFaceDirectionsReverse[UCVoxelBlockType][DirectionIndex][AbsoluteFaceIndex] = RelativeFaceIndex
	// 类型：使用UCVoxelBlockType枚举值（同上）
	// 方向ID：楔形体用0-11，三角锥和三角锥互补体用0-7，立方体用0（以最大12为准）
	// 绝对面索引：0-5（6个面）
	int32 AllBlockFaceDirectionsReverse[4][12][6];

	FVoxelBlockShapeGenerator()
	{
		// 使用默认方向的静态数组作为基础（Yaw=0, Pitch=0）
		// 生成所有旋转方向的顶点数据
		GenerateCubeOrientations();
		GenerateAllSlopeOrientations();
		GenerateAllTriSlopeOrientations();
		GenerateAllTriSlopeComplementOrientations();
	}

private:
	// 旋转顶点索引（绕Y轴旋转Pitch*90度）
	// 顶点定义：0(0,0,0)前左下, 1(1,0,0)后左下, 2(1,1,0)后右下, 3(0,1,0)前右下
	//          4(0,0,1)前左上, 5(1,0,1)后左上, 6(1,1,1)后右上, 7(0,1,1)前右上
	// 绕Y轴旋转90度（从+Y方向看，逆时针）：(x,y,z) -> (z, y, 1-x)
	// 绕Y轴旋转180度：(x,y,z) -> (1-x, y, 1-z)
	int32 RotateVertexIndexByPitch(int32 VertexIndex, int32 Pitch) const
	{
		if (VertexIndex < 0 || VertexIndex >= 8)
			return VertexIndex;

		// Pitch旋转映射表：绕Y轴旋转
		// Pitch=0: 不旋转
		// Pitch=1: 绕Y轴旋转90度 (0->4, 1->0, 2->3, 3->7, 4->5, 5->1, 6->2, 7->6)
		// Pitch=2: 绕Y轴旋转180度 (0->5, 1->4, 2->7, 3->6, 4->1, 5->0, 6->3, 7->2)
		static const int32 PitchMap[3][8] = {
			{0, 1, 2, 3, 4, 5, 6, 7}, // Pitch=0: 不旋转
			{4, 0, 3, 7, 5, 1, 2, 6}, // Pitch=1: 绕Y轴旋转90度
			{5, 4, 7, 6, 1, 0, 3, 2}  // Pitch=2: 绕Y轴旋转180度
		};

		return PitchMap[Pitch % 3][VertexIndex];
	}

	// 旋转顶点索引（绕Z轴旋转Yaw*90度）
	// 顶点定义：0(0,0,0)前左下, 1(1,0,0)后左下, 2(1,1,0)后右下, 3(0,1,0)前右下
	//          4(0,0,1)前左上, 5(1,0,1)后左上, 6(1,1,1)后右上, 7(0,1,1)前右上
	// 绕Z轴旋转90度（逆时针从上方看）：(x,y,z) -> (y, 1-x, z)
	int32 RotateVertexIndex(int32 VertexIndex, int32 Yaw) const
	{
		if (VertexIndex < 0 || VertexIndex >= 8)
			return VertexIndex;

		// Yaw旋转映射表：绕Z轴旋转（逆时针从上方看）
		static const int32 RotateMap[4][8] = {
			{0, 1, 2, 3, 4, 5, 6, 7}, // Yaw=0: 不旋转
			{3, 0, 1, 2, 7, 4, 5, 6}, // Yaw=1: 旋转90度  (0->3, 1->0, 2->1, 3->2, 4->7, 5->4, 6->5, 7->6)
			{2, 3, 0, 1, 6, 7, 4, 5}, // Yaw=2: 旋转180度 (0->2, 1->3, 2->0, 3->1, 4->6, 5->7, 6->4, 7->5)
			{1, 2, 3, 0, 5, 6, 7, 4}  // Yaw=3: 旋转270度 (0->1, 1->2, 2->3, 3->0, 4->5, 5->6, 6->7, 7->4)
		};

		return RotateMap[Yaw & 0x03][VertexIndex];
	}

	// 变换面的顶点索引（先应用Pitch旋转，再应用Yaw旋转）
	void TransformFace(FaceIndices& OutFace, const FaceIndices& InFace, int32 Yaw, int32 Pitch) const
	{
		OutFace.Count = InFace.Count;
		for (int32 i = 0; i < 4; ++i)
		{
			if (InFace.Indices[i] >= 0)
			{
				// 先应用Pitch旋转（绕Y轴），再应用Yaw旋转（绕Z轴）
				int32 PitchedIndex = RotateVertexIndexByPitch(InFace.Indices[i], Pitch);
				OutFace.Indices[i] = RotateVertexIndex(PitchedIndex, Yaw);
			}
			else
				OutFace.Indices[i] = -1;
		}
	}

	// 旋转面索引（绕Y轴旋转Pitch*90度）
	// 面索引：0=Left, 1=Front, 2=Right, 3=Back, 4=Top, 5=Bottom
	// Pitch=1: Left->Back, Front->Bottom, Right->Front, Back->Top, Top->Right, Bottom->Left
	// Pitch=2: Left->Right, Front->Back, Right->Left, Back->Front, Top->Bottom, Bottom->Top
	int32 RotateFaceIndexByPitch(int32 FaceIndex, int32 Pitch) const
	{
		if (FaceIndex < 0 || FaceIndex >= 6)
			return FaceIndex;

		if (Pitch == 0)
			return FaceIndex;

		// Pitch旋转映射表：绕Y轴旋转
		// 0=Left(0,-1,0), 1=Front(-1,0,0), 2=Right(0,1,0), 3=Back(1,0,0), 4=Top(0,0,1), 5=Bottom(0,0,-1)
		// 绕Y轴旋转90度：(x,y,z) -> (z, y, -x)
		// 绕Y轴旋转180度：(x,y,z) -> (-x, y, -z)
		static const int32 PitchFaceMap[3][6] = {
			{0, 1, 2, 3, 4, 5}, // Pitch=0: 不旋转
			{3, 5, 1, 4, 2, 0}, // Pitch=1: 绕Y轴旋转90度 (Left->Back, Front->Bottom, Right->Front, Back->Top, Top->Right, Bottom->Left)
			{2, 3, 0, 1, 5, 4}  // Pitch=2: 绕Y轴旋转180度 (Left->Right, Front->Back, Right->Left, Back->Front, Top->Bottom, Bottom->Top)
		};

		return PitchFaceMap[Pitch % 3][FaceIndex];
	}

	// 旋转面索引（绕Z轴旋转Yaw*90度）
	// Yaw=1: Left->Front, Front->Right, Right->Back, Back->Left, Top->Top, Bottom->Bottom
	// Yaw=2: Left->Right, Front->Back, Right->Left, Back->Front, Top->Top, Bottom->Bottom
	// Yaw=3: Left->Back, Front->Left, Right->Front, Back->Right, Top->Top, Bottom->Bottom
	int32 RotateFaceIndexByYaw(int32 FaceIndex, int32 Yaw) const
	{
		if (FaceIndex < 0 || FaceIndex >= 6)
			return FaceIndex;

		if (Yaw == 0)
			return FaceIndex;

		// Yaw旋转映射表：绕Z轴旋转（逆时针从上方看）
		// Top和Bottom不受Yaw旋转影响
		if (FaceIndex == 4 || FaceIndex == 5)
			return FaceIndex;

		static const int32 YawFaceMap[4][4] = {
			{0, 1, 2, 3}, // Yaw=0: 不旋转 (Left, Front, Right, Back)
			{1, 2, 3, 0}, // Yaw=1: 旋转90度 (Left->Front, Front->Right, Right->Back, Back->Left)
			{2, 3, 0, 1}, // Yaw=2: 旋转180度 (Left->Right, Front->Back, Right->Left, Back->Front)
			{3, 0, 1, 2}  // Yaw=3: 旋转270度 (Left->Back, Front->Left, Right->Front, Back->Right)
		};

		return YawFaceMap[Yaw & 0x03][FaceIndex];
	}

	// 变换面索引（先应用Pitch旋转，再应用Yaw旋转）
	// 返回原始面索引在旋转后对应的绝对面索引（0-5）
	int32 TransformFaceIndex(int32 OriginalFaceIndex, int32 Yaw, int32 Pitch) const
	{
		// 先应用Pitch旋转（绕Y轴），再应用Yaw旋转（绕Z轴）
		int32 PitchedFaceIndex = RotateFaceIndexByPitch(OriginalFaceIndex, Pitch);
		return RotateFaceIndexByYaw(PitchedFaceIndex, Yaw);
	}

	// 生成立方体方向（只有1个方向，方向索引0）
	void GenerateCubeOrientations()
	{
		int32 DirectionIndex = 0;
		// 立方体只有1个方向，直接复制全局BoxFaces
		for (int32 FaceIdx = 0; FaceIdx < 7; ++FaceIdx)
		{
			AllBlockFaces[UCVoxelBlockType_Cube][DirectionIndex][FaceIdx] = ::BoxFaces[FaceIdx];
		}
		// 立方体的反向映射：AbsoluteFaceIndex -> RelativeFaceIndex（立方体不需要旋转，所以是恒等映射）
		for (int32 AbsoluteFaceIdx = 0; AbsoluteFaceIdx < 6; ++AbsoluteFaceIdx)
		{
			AllBlockFaceDirectionsReverse[UCVoxelBlockType_Cube][DirectionIndex][AbsoluteFaceIdx] = AbsoluteFaceIdx;
		}
	}

	// 生成所有楔形体方向（12个方向：Yaw 0-3, Pitch 0-2）
	void GenerateAllSlopeOrientations()
	{
		for (int32 Yaw = 0; Yaw < 4; ++Yaw)
		{
			for (int32 Pitch = 0; Pitch < 3; ++Pitch)
			{
				int32 DirectionIndex = Yaw * 3 + Pitch;

				// 使用默认方向的 SlopeFaces 作为基础
				// Pitch=0: 当前默认方向
				// Pitch=1: 绕Y轴旋转90度后，再绕Yaw旋转
				// Pitch=2: 绕Y轴旋转180度后，再绕Yaw旋转
				for (int32 FaceIdx = 0; FaceIdx < 7; ++FaceIdx)
				{
					TransformFace(AllBlockFaces[UCVoxelBlockType_SquareSlope][DirectionIndex][FaceIdx], ::SlopeFaces[FaceIdx], Yaw, Pitch);
				}

				// 生成对应方向的面的索引映射（6个面：Left, Front, Right, Back, Top, Bottom）
				// AllBlockFaceDirectionsReverse[UCVoxelBlockType_SquareSlope][DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
				for (int32 RelativeFaceIdx = 0; RelativeFaceIdx < 6; ++RelativeFaceIdx)
				{
					int32 AbsoluteFaceIdx = TransformFaceIndex(RelativeFaceIdx, Yaw, Pitch);
					// 生成反向映射：AbsoluteFaceIndex -> RelativeFaceIndex
					AllBlockFaceDirectionsReverse[UCVoxelBlockType_SquareSlope][DirectionIndex][AbsoluteFaceIdx] = RelativeFaceIdx;
				}
			}
		}
	}

	// 生成所有三角锥方向（8个方向：Yaw 0-3, Pitch 0-1）
	void GenerateAllTriSlopeOrientations()
	{
		for (int32 Yaw = 0; Yaw < 4; ++Yaw)
		{
			for (int32 Pitch = 0; Pitch < 2; ++Pitch)
			{
				int32 DirectionIndex = Yaw * 2 + Pitch;

				// 使用默认方向的 TriSlopeFaces 作为基础
				// 对于三角锥，Pitch 只有 0 和 1，Pitch=1 表示绕Y轴旋转90度
				for (int32 FaceIdx = 0; FaceIdx < 7; ++FaceIdx)
				{
					TransformFace(AllBlockFaces[UCVoxelBlockType_TriangularSlope][DirectionIndex][FaceIdx], ::TriSlopeFaces[FaceIdx], Yaw, Pitch);
				}

				// 生成对应方向的面的索引映射（6个面：Left, Front, Right, Back, Top, Bottom）
				// AllBlockFaceDirectionsReverse[UCVoxelBlockType_TriangularSlope][DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
				for (int32 RelativeFaceIdx = 0; RelativeFaceIdx < 6; ++RelativeFaceIdx)
				{
					int32 AbsoluteFaceIdx = TransformFaceIndex(RelativeFaceIdx, Yaw, Pitch);
					// 生成反向映射：AbsoluteFaceIndex -> RelativeFaceIndex
					AllBlockFaceDirectionsReverse[UCVoxelBlockType_TriangularSlope][DirectionIndex][AbsoluteFaceIdx] = RelativeFaceIdx;
				}
			}
		}
	}

	// 生成所有三角锥互补体方向（8个方向：Yaw 0-3, Pitch 0-1）
	void GenerateAllTriSlopeComplementOrientations()
	{
		for (int32 Yaw = 0; Yaw < 4; ++Yaw)
		{
			for (int32 Pitch = 0; Pitch < 2; ++Pitch)
			{
				int32 DirectionIndex = Yaw * 2 + Pitch;

				// 使用默认方向的 TriSlopeComplementFaces 作为基础
				// 对于三角锥互补体，Pitch 只有 0 和 1，Pitch=1 表示绕Y轴旋转90度
				for (int32 FaceIdx = 0; FaceIdx < 7; ++FaceIdx)
				{
					TransformFace(AllBlockFaces[UCVoxelBlockType_TriangularComplement][DirectionIndex][FaceIdx], ::TriSlopeComplementFaces[FaceIdx], Yaw, Pitch);
				}

				// 生成对应方向的面的索引映射（6个面：Left, Front, Right, Back, Top, Bottom）
				// AllBlockFaceDirectionsReverse[UCVoxelBlockType_TriangularComplement][DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
				for (int32 RelativeFaceIdx = 0; RelativeFaceIdx < 6; ++RelativeFaceIdx)
				{
					int32 AbsoluteFaceIdx = TransformFaceIndex(RelativeFaceIdx, Yaw, Pitch);
					// 生成反向映射：AbsoluteFaceIndex -> RelativeFaceIndex
					AllBlockFaceDirectionsReverse[UCVoxelBlockType_TriangularComplement][DirectionIndex][AbsoluteFaceIdx] = RelativeFaceIdx;
				}
			}
		}
	}
};

// 全局单例：在首次使用时初始化
static FVoxelBlockShapeGenerator* GetBlockShapeGenerator()
{
	static FVoxelBlockShapeGenerator* Generator = nullptr;
	if (!Generator)
	{
		Generator = new FVoxelBlockShapeGenerator();
	}
	return Generator;
}

AVoxelTile::AVoxelTile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsActive(false)
	, TileCoord(0, 0)
	, VoxelSize(100.0f)
	, Material(nullptr)
	, TileData(nullptr)
	, MaterialInstanceDynamic(nullptr)
	, bMeshUpdatePending(false)
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

FIntTriangle AVoxelTile::SortTriangleVertices(const FIntVector& V0, const FIntVector& V1, const FIntVector& V2)
{
	int64 Value0 = (int64)V0.Z * 0x10000000000 + (int64)V0.X * 0x100000 + (int64)V0.Y;
	int64 Value1 = (int64)V1.Z * 0x10000000000 + (int64)V1.X * 0x100000 + (int64)V1.Y;
	int64 Value2 = (int64)V2.Z * 0x10000000000 + (int64)V2.X * 0x100000 + (int64)V2.Y;

	if (Value0 < Value1)
	{
		if (Value0 < Value2)
		{
			if (Value1 < Value2)
				return FIntTriangle(V0, V1, V2);
			else
				return FIntTriangle(V0, V2, V1);
		}
		else
			return FIntTriangle(V2, V0, V1);
	}
	else
	{
		if (Value1 < Value2)
		{
			if (Value0 < Value2)
				return FIntTriangle(V1, V0, V2);
			else
				return FIntTriangle(V1, V2, V0);
		}
		else
			return FIntTriangle(V2, V1, V0);
	}
}

int32 AVoxelTile::GetNeighborFaceInfo(int32 FaceIndex)
{
	// 对应面的ID：对面关系
	// 0(Left) -> 2(Right), 1(Front) -> 3(Back), 2(Right) -> 0(Left), 
	// 3(Back) -> 1(Front), 4(Top) -> 5(Bottom), 5(Bottom) -> 4(Top)
	int32 OppositeFaceIndex = FaceIndex < 4 ? (FaceIndex + 2) % 4 : !(FaceIndex - 4) + 4;
	return OppositeFaceIndex;
}

void AVoxelTile::GetFaceTriangles(int32 X, int32 Y, int32 Z, int32 FaceIndex, uint8 BlockType, int32 DirectionIndex, const UCVoxelData& Voxel, bool bFlat, TArray<FIntVertex>& OutVertices, FVector& Normal, TArray<FIntVector>& OutFaceIndexes) const
{
	FVoxelBlockShapeGenerator* BlockShapeGenerator = GetBlockShapeGenerator();
	OutVertices.Empty();

	// 将局部顶点索引转换为体素网格坐标的辅助函数
	auto ConvertVertexIndexToGridPos = [X, Y, Z](int32 VertexIndex) -> FIntVector
		{
			if (VertexIndex < 0 || VertexIndex >= 8)
				return FIntVector::ZeroValue;
			const FVector& LocalVertex = BaseCubeVertices[VertexIndex];
			return FIntVector(
				X + (int32)LocalVertex.X,
				Y + (int32)LocalVertex.Y,
				Z + (int32)LocalVertex.Z
			);
		};

	int32 RelativeFaceIndex = FaceIndex > 5 ? FaceIndex : BlockShapeGenerator->AllBlockFaceDirectionsReverse[BlockType][DirectionIndex][FaceIndex];
	const FaceIndices& FaceDef = BlockShapeGenerator->AllBlockFaces[BlockType][DirectionIndex][RelativeFaceIndex];
	if (FaceDef.Count == 0)
		return;

	// 计算 Color（基于 TextureID）
	FColor TriangleColor = FColor::White;
	if (Voxel.TextureID > 0)
	{
		uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
		TriangleColor = FColor(Gray, Gray, Gray, 255);
	}

	uint32 UVFaceIndex = FaceIndex < 6 ? RelativeFaceIndex : BlockShapeGenerator->AllBlockFaceDirectionsReverse[BlockType][DirectionIndex][4];
	const FaceIndices& UVFaceDef = FaceIndex < 6 
		? BlockShapeGenerator->AllBlockFaces[BlockType][DirectionIndex][UVFaceIndex]
		: BlockShapeGenerator->AllBlockFaces[UCVoxelBlockType_Cube][0][UVFaceIndex];

	if (BlockType != UCVoxelBlockType_Cube)
		uint32 a = 0;

	FIntVector V0 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[0])) * 2;
	FIntVector V1 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[1])) * 2;
	FIntVector V2 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[2])) * 2;

	FIntVector Dis0 = V1 - V0;
	FIntVector Dis1 = V2 - V0;

// 	FIntVector2 UV0 = BaseCubeUVs[UVFaceDef.Indices[0]][UVFaceIndex] * 2;
// 	FIntVector2 UV1 = BaseCubeUVs[UVFaceDef.Indices[1]][UVFaceIndex] * 2;
// 	FIntVector2 UV2 = BaseCubeUVs[UVFaceDef.Indices[2]][UVFaceIndex] * 2;
	int32 OffX = 0;
	int32 OffY = 1;
	if (Dis0.Y == 0 && Dis1.Y == 0)
	{
		OffX = 0;
		OffY = 2;
	}
	if (Dis0.X == 0 && Dis1.X == 0)
	{
		OffX = 1;
		OffY = 2;
	}
	FIntVector2 UV0(0, 0);
	FIntVector2 UV1(Dis0[OffX], Dis0[OffY]);
	FIntVector2 UV2(Dis1[OffX], Dis1[OffY]);

	// 计算法向量
	if (FaceIndex < 6)
	{
		// 标准面：使用预定义的法向量
		Normal = FaceNormals[FaceIndex];
	}
	else
	{
		// 斜面（FaceIndex == 6）：通过顶点计算法向量
		// 使用前三个顶点计算：Normal = (V1 - V0) × (V2 - V0)，然后归一化
		FVector Edge1 = FVector(V1.X - V0.X, V1.Y - V0.Y, V1.Z - V0.Z);
		FVector Edge2 = FVector(V2.X - V0.X, V2.Y - V0.Y, V2.Z - V0.Z);
		Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
		// 如果归一化失败，使用默认值
		if (Normal.IsNearlyZero())
		{
			Normal = FVector(0, 0, 1); // 默认向上
		}
	}

	if (bFlat)
	{
		// 简单模式：2个三角形
		if (FaceDef.Count >= 4)
		{
			FIntVector V3 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[3])) * 2;
			FIntVector Dis2 = V3 - V0;
			FIntVector2 UV3(Dis2[OffX], Dis2[OffY]);
// 			FIntVector2 UV3 = BaseCubeUVs[UVFaceDef.Indices[3]][UVFaceIndex] * 2;

			OutVertices.Add(FIntVertex(V0, UV0, TriangleColor));
			OutVertices.Add(FIntVertex(V1, UV1, TriangleColor));
			OutVertices.Add(FIntVertex(V2, UV2, TriangleColor));
			OutVertices.Add(FIntVertex(V3, UV3, TriangleColor));

			OutFaceIndexes.Add(FIntVector(0, 2, 1));
			OutFaceIndexes.Add(FIntVector(0, 3, 2));
		}
	}
	else
	{
		if (FaceDef.Count == 3)
		{
			if (FaceIndex < 6)
			{
				FIntVector V3 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[3])) * 2;
				FIntVector Dis2 = V3 - V0;
				FIntVector2 UV3(Dis2[OffX], Dis2[OffY]);
//				FIntVector2 UV3 = BaseCubeUVs[UVFaceDef.Indices[3]][UVFaceIndex] * 2;

				FIntVector Center = (V0 + V1 + V2 + V3) / 4;
				FIntVector2 CenterUV = (UV0 + UV1 + UV2 + UV3) / 4;

				OutVertices.Add(FIntVertex(V0, UV0, TriangleColor));
				OutVertices.Add(FIntVertex(V1, UV1, TriangleColor));
				OutVertices.Add(FIntVertex(V2, UV2, TriangleColor));
				OutVertices.Add(FIntVertex(Center, CenterUV, TriangleColor));

				OutFaceIndexes.Add(FIntVector(0, 3, 1));
				OutFaceIndexes.Add(FIntVector(2, 3, 0));
			}
			else
			{
				OutVertices.Add(FIntVertex(V0, UV0, TriangleColor));
				OutVertices.Add(FIntVertex(V1, UV1, TriangleColor));
				OutVertices.Add(FIntVertex(V2, UV2, TriangleColor));

				OutFaceIndexes.Add(FIntVector(0, 2, 1));
			}
		}
		else if (FaceDef.Count == 4)
		{
			// 四边形拆解成4个三角形（带中心点）
			FIntVector V3 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[3])) * 2;
			FIntVector Dis2 = V3 - V0;
			FIntVector2 UV3(Dis2[OffX], Dis2[OffY]);
// 			FIntVector2 UV3 = BaseCubeUVs[UVFaceDef.Indices[3]][UVFaceIndex] * 2;

			FIntVector Center = (V0 + V1 + V2 + V3) / 4;
			FIntVector2 CenterUV = (UV0 + UV1 + UV2 + UV3) / 4;

			OutVertices.Add(FIntVertex(V0, UV0, TriangleColor));
			OutVertices.Add(FIntVertex(V1, UV1, TriangleColor));
			OutVertices.Add(FIntVertex(V2, UV2, TriangleColor));
			OutVertices.Add(FIntVertex(V3, UV3, TriangleColor));
			OutVertices.Add(FIntVertex(Center, CenterUV, TriangleColor));

			OutFaceIndexes.Add(FIntVector(0, 4, 1));
			OutFaceIndexes.Add(FIntVector(1, 4, 2));
			OutFaceIndexes.Add(FIntVector(2, 4, 3));
			OutFaceIndexes.Add(FIntVector(3, 4, 0));
		}
	}
}

void AVoxelTile::SetActive(bool bActive)
{
	bIsActive = bActive;

	// 更新网格
	UpdateMesh(bIsActive);
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
	FVector TileWorldGridPos(TileCoord.Y * VOXEL_TILE_SIZE_X * VoxelSize, TileCoord.X * VOXEL_TILE_SIZE_Y * VoxelSize, 0);

	FVoxelBlockShapeGenerator* BlockShapeGenerator = GetBlockShapeGenerator();
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

				// 计算方向索引（非Box类型需要）
				uint8 DirectionIndex = 0;
				if (BlockType == UCVoxelBlockType_SquareSlope)
					DirectionIndex = UCVoxelData_GetSlopeDirectionIndex(Voxel);
				else if (BlockType == UCVoxelBlockType_TriangularSlope || BlockType == UCVoxelBlockType_TriangularComplement)
					DirectionIndex = UCVoxelData_GetTriSlopeDirectionIndex(Voxel);

				
				// 遍历所有面（Box有6个面，其他有7个面）
				int32 MaxFaceIndex = (BlockType == UCVoxelBlockType_Cube) ? 6 : 7;
				for (int32 FaceIndex = 0; FaceIndex < MaxFaceIndex; ++FaceIndex)
				{
					// 获取面的三角形数组
					TArray<FIntVertex> FaceVertices;
					TArray<FIntVector> FacesIndexes;
					FVector FaceNormal;
					bool bFlat = IsFaceFlat(X, Y, Z, FaceIndex);
					GetFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, Voxel, bFlat, FaceVertices, FaceNormal, FacesIndexes);

					if (FacesIndexes.Num() == 0)
						continue;

					int32 NeighborFaceIndex = GetNeighborFaceInfo(FaceIndex);
					if (NeighborFaceIndex == -1)
						continue;

					const FIntVector& FaceDir = FaceDirections[FaceIndex];
					FIntVector Neighbor(X + FaceDir.X, Y + FaceDir.Y, Z + FaceDir.Z);
					TArray<FIntVertex> NeighborFaceVertices;
					TArray<FIntVector> NeighborFacesIndexes;
					FVector NeighborFaceNormal;
					// 检查相邻体素是否在有效范围内
					if (IsValidVoxelCoord(Neighbor.X, Neighbor.Y, Neighbor.Z))
					{
						// 获取相邻体素
						UCVoxelData NeighborVoxel = GetVoxel(Neighbor.X, Neighbor.Y, Neighbor.Z);
						if (NeighborVoxel.LayerID != UCVoxelData_Layer_Null)
						{
							// 获取相邻体素的类型和方向索引
							uint8 NeighborBlockType = NeighborVoxel.Type & 0x03;
							uint8 NeighborDirectionIndex = 0;
							if (NeighborBlockType == UCVoxelBlockType_SquareSlope)
								NeighborDirectionIndex = UCVoxelData_GetSlopeDirectionIndex(NeighborVoxel);
							else if (NeighborBlockType == UCVoxelBlockType_TriangularSlope || NeighborBlockType == UCVoxelBlockType_TriangularComplement)
								NeighborDirectionIndex = UCVoxelData_GetTriSlopeDirectionIndex(NeighborVoxel);

							// 获取相邻面的三角形
							bool bNeighborFlat = (NeighborBlockType == UCVoxelBlockType_Cube) ? IsFaceFlat(Neighbor.X, Neighbor.Y, Neighbor.Z, NeighborFaceIndex) : false;

							GetFaceTriangles(Neighbor.X, Neighbor.Y, Neighbor.Z, NeighborFaceIndex, NeighborBlockType, NeighborDirectionIndex, NeighborVoxel, bNeighborFlat, NeighborFaceVertices, NeighborFaceNormal, NeighborFacesIndexes);
						}
					}

					// 从当前面的三角形中删除相邻面已有的三角形
					TArray<FIntVector> TrianglesToRender;
					for (const FIntVector& Face : FacesIndexes)
					{
						FIntTriangle Triangle = SortTriangleVertices(FaceVertices[Face.X].V, FaceVertices[Face.Y].V, FaceVertices[Face.Z].V);
						bool bFound = false;
						for (const FIntVector& NeighborFace : NeighborFacesIndexes)
						{
							FIntTriangle NeighborTriangle = SortTriangleVertices(NeighborFaceVertices[NeighborFace.X].V, NeighborFaceVertices[NeighborFace.Y].V, NeighborFaceVertices[NeighborFace.Z].V);
							if (Triangle == NeighborTriangle)
							{
								bFound = true;
								break;
							}
						}
						if (!bFound)
							TrianglesToRender.Add(Face);
					}

					// 如果没有需要渲染的三角形，跳过
					if (TrianglesToRender.Num() == 0)
						continue;

					int32 BaseIndex = Vertices.Num();

					// 从 AryTextureID 获取面的纹理ID（使用面的中心点或第一个顶点）
					int32 TextureID = 0;
					if (TileData && TileData->AryTextureID.GetSize() > 0 && FaceVertices.Num() > 0)
					{
						// 获取第一个顶点的世界坐标（已乘以2）
						FIntVector VertexWorldPos = FaceVertices[0].V;
						// 转换为局部坐标（相对于Tile，范围0-32, 0-32, 0-64）
						// V0, V1, V2 已经是世界坐标（乘以2），需要转换回局部坐标
						// LocalToWorldPosition 返回的是世界网格坐标，需要反向转换
						// 简化：使用体素中心点 (X, Y, Z) 对应的 UV 坐标
						// AryTextureID 的索引：UVIndex = (Z+1) * (TileSizeY+1) * (TileSizeX+1) + (Y+1) * (TileSizeX+1) + (X+1)
						const int32 UVSizeX = VOXEL_TILE_SIZE_X + 1;  // 33
						const int32 UVSizeY = VOXEL_TILE_SIZE_Y + 1;  // 33
						const int32 UVSizeZ = VOXEL_TILE_SIZE_Z + 1;  // 65
						
						// 使用体素坐标 (X, Y, Z) 获取对应的 TextureID
						// 注意：AryTextureID 的坐标范围是 0-33, 0-33, 0-65（比体素大1）
						// 对于体素 (X, Y, Z)，对应的 UV 坐标是 (X+1, Y+1, Z+1)
						int32 UVIndex = (Z + 1) * UVSizeY * UVSizeX + (Y + 1) * UVSizeX + (X + 1);
						if (UVIndex >= 0 && UVIndex < TileData->AryTextureID.GetSize())
						{
							UCPoint TextureIDPoint = TileData->AryTextureID[UVIndex];
							TextureID = TextureIDPoint.x; // 使用 x 作为 TextureID
						}
					}

					for (ucINT i = 0; i < FaceVertices.Num(); i++)
					{
						FVector Location(FaceVertices[i].V);
						Vertices.Add(Location / 2.0f * VoxelSize - TileWorldGridPos);
						FVector2D UV(FaceVertices[i].UV);
						UVs.Add(UV / 2.0f);
						
						// 如果从 AryTextureID 获取到了 TextureID，使用它来设置顶点颜色或材质
						// 这里先保持原有的颜色逻辑，后续可以通过材质参数传递 TextureID
						FColor VertexColor = FaceVertices[i].Color;
						if (TextureID > 0 && Terrain)
						{
							// 可以通过顶点颜色传递 TextureID 信息，或者使用材质参数
							// 这里先保持原有逻辑，后续可以扩展
						}
						VertexColors.Add(VertexColor);

						Normals.Add(FaceNormal);
						Tangents.Add(FProcMeshTangent(0, 0, 1));
					}
					for (const FIntVector& Face : TrianglesToRender)
					{
						Triangles.Add(BaseIndex + Face.X);
						Triangles.Add(BaseIndex + Face.Y);
						Triangles.Add(BaseIndex + Face.Z);
					}
				}
			}
		}
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
	// 如果已经有待处理的刷新请求，忽略本次请求
	if (bMeshUpdatePending)
		return;

	ClearMeshData();
	if (!Active)
	{
		ProceduralMesh->ClearMeshSection(0);
		return;
	}

	// 标记有待处理的刷新
	bMeshUpdatePending = true;
	// 构建网格数据
	BuildMeshData();
	
	// 清除现有的定时器（如果存在）
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(MeshUpdateTimerHandle);
		
		// 设置1秒延迟刷新
		World->GetTimerManager().SetTimer(
			MeshUpdateTimerHandle,
			this,
			&AVoxelTile::ExecuteMeshUpdate,
			0.05f, // 1秒延迟
			false // 不循环
		);
	}
	else
	{
		// 如果World不存在，直接执行（不应该发生，但作为fallback）
		ExecuteMeshUpdate();
	}
}

void AVoxelTile::ExecuteMeshUpdate()
{
	// 清除待处理标志，允许新的刷新请求
	bMeshUpdatePending = false;
	
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
		// 创建 Material Instance Dynamic（如果还没有创建）
		if (!MaterialInstanceDynamic)
		{
			MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(Material, this);
			UE_LOG(LogTemp, Log, TEXT("AVoxelTile::ExecuteMeshUpdate: Created Material Instance Dynamic"));
		}
		
		// 如果有纹理列表，设置默认纹理（第一个纹理）
		if (Terrain)
		{
			TArray<UTexture2D*> TextureList = Terrain->GetTextureList();
			UE_LOG(LogTemp, Log, TEXT("AVoxelTile::ExecuteMeshUpdate: Terrain has %d textures"), TextureList.Num());
			
			if (TextureList.Num() > 0)
			{
				UTexture2D* DefaultTexture = TextureList[0];
				if (DefaultTexture)
				{
					// 设置纹理参数（参数名需要在材质中定义，例如 "Texture" 或 "BaseTexture"）
					// 注意：这里的参数名需要与材质中的 Texture Parameter 名称匹配
					MaterialInstanceDynamic->SetTextureParameterValue(FName(TEXT("Texture")), DefaultTexture);
					UE_LOG(LogTemp, Log, TEXT("AVoxelTile::ExecuteMeshUpdate: Set texture parameter 'Texture' (Size: %dx%d)"), 
						DefaultTexture->GetSizeX(), DefaultTexture->GetSizeY());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("AVoxelTile::ExecuteMeshUpdate: DefaultTexture is null"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AVoxelTile::ExecuteMeshUpdate: TextureList is empty"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AVoxelTile::ExecuteMeshUpdate: Terrain is null"));
		}
		
		ProceduralMesh->SetMaterial(0, MaterialInstanceDynamic);
		UE_LOG(LogTemp, Log, TEXT("AVoxelTile::ExecuteMeshUpdate: Material applied to ProceduralMesh"));
	}
	else
	{
		ProceduralMesh->SetMaterial(0, nullptr);
	}
}

void AVoxelTile::SetTextureToMaterial(int32 TextureID, UTexture2D* Texture)
{
	if (!Material || !Texture)
		return;

	// 创建或获取 Material Instance Dynamic
	if (!MaterialInstanceDynamic)
	{
		MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(Material, this);
	}

	// 设置纹理参数（参数名需要在材质中定义）
	// 注意：这里的参数名需要与材质中的 Texture Parameter 名称匹配
	MaterialInstanceDynamic->SetTextureParameterValue(FName(TEXT("Texture")), Texture);
	
	// 如果材质已经应用到网格，更新它
	if (ProceduralMesh && ProceduralMesh->GetMaterial(0) != MaterialInstanceDynamic)
	{
		ProceduralMesh->SetMaterial(0, MaterialInstanceDynamic);
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

