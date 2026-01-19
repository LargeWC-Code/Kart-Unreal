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

// 六个面的方向向量（从VoxelTerrain复制）
// 0=Left, 1=Front, 2=Right, 3=Back, 4=Top, 5=Bottom
const FIntVector FaceDirections[6] = {
	FIntVector(0, -1,  0), // 0: Left (面向-Y方向)
	FIntVector(-1,  0,  0), // 1: Front (面向-X方向)
	FIntVector(0,  1,  0), // 2: Right (面向+Y方向)
	FIntVector(1,  0,  0), // 3: Back (面向+X方向)
	FIntVector(0,  0,  1), // 4: Top (面向+Z方向)
	FIntVector(0,  0, -1)  // 5: Bottom (面向-Z方向)
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

static const FaceIndices BoxFaces[6] = {
	// 0: Left (Y=0，面向-Y方向) - 斜对角：0和5
	{ {+0, +1, +4, +5}, 4 },
	// 1: Front (X=1，面向+X方向) - 斜对角：0和7
	{ {+3, +0, +7, +4}, 4 },
	// 2: Right (Y=1，面向+Y方向) - 斜对角：2和7
	{ {+2, +3, +6, +7}, 4 },
	// 3: Back (X=0，面向-X方向) - 斜对角：1和6
	{ {+1, +2, +5, +6}, 4 },
	// 4: Top (Z=1，面向+Z方向) - 斜对角：4和6
	{ {+4, +5, +7, +6}, 4 },
	// 5: Bottom (Z=0，面向-Z方向) - 斜对角：0和2
	{ {+0, +3, +1, +2}, 4 }
};

static const FaceIndices SlopeFaces[6] = {
	// 0: Left (Y=0) - 不存在（被斜面覆盖）
	{ {-1, -1, -1, -1}, 0 },
	// 1: Front (X=1) - 3个顶点（三角形）
	{ {+3, +0, +7, +4}, 3 },
	// 2: Right (Y=1) - 4个顶点
	{ {+2, +3, +6, +7}, 4 },
	// 3: Back (X=0) - 3个顶点（三角形）
	{ {+2, +6, +1, +5}, 3 },
	// 4: Top (Z=1) - 斜面（4个顶点，倾斜四边形）
	{ {+0, +1, +7, +6}, 4 },
	// 5: Bottom (Z=0，面向-Z方向) - 斜对角：0和2
	{ {+0, +3, +1, +2}, 4 }
};

static const FaceIndices TriSlopeFaces[6] = {
	// 0: Left (Y=0) - 不存在（被切掉）
	{ {-1, -1, -1, -1}, 0 },
	// 1: Front (X=1) - 3个顶点（三角形）
	{ {-1, -1, -1, -1}, 0 },
	// 2: Right (Y=1) - 3个顶点（三角形）
	{ {+2, +3, +6, +7}, 3 },
	// 3: Back (X=0) - 3个顶点（三角形）
	{ {+2, +6, +1, +5}, 3 },
	// 4: Top (Z=1) - 斜面（3个顶点，三角形）
	{ {+3, +1, +6, -1}, 3 },
	// 5: Bottom (Z=0) - 3个顶点（三角形，切掉0角）
	{ {+2, +1, +3, +0}, 3 }
};

static const FaceIndices TriSlopeComplementFaces[6] = {
	// 0: Left (Y=0) - 4个顶点（但顶点4不存在，所以是3个顶点的三角形）
	{ {+1, +5, +0, +4}, 3 },
	// 1: Front (X=1) - 4个顶点
	{ {+3, +0, +7, +4}, 3 },
	// 2: Right (Y=1) - 4个顶点
	{ {+2, +3, +6, +7}, 4 },
	// 3: Back (X=0) - 3个顶点（三角形，切角）
	{ {+1, +2, +5, +6}, 4 },
	// 4: Top (Z=1) - 斜面（顶面和斜面合并，3个顶点）
	{ {+6, +7, +5, +0}, 4 },
	// 5: Bottom (Z=0) - 4个顶点（正方形）
	{ {+0, +3, +1, +2}, 4 }
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
	FaceIndices AllBlockFaces[4][12][6];
	
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
			{0, 4, 2, 5, 3, 1}, // Pitch=1: 绕Y轴旋转90度 (Left->Back, Front->Bottom, Right->Front, Back->Top, Top->Right, Bottom->Left)
			{0, 3, 2, 1, 5, 4}  // Pitch=2: 绕Y轴旋转180度 (Left->Right, Front->Back, Right->Left, Back->Front, Top->Bottom, Bottom->Top)
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
		for (int32 FaceIdx = 0; FaceIdx < 6; ++FaceIdx)
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
				for (int32 FaceIdx = 0; FaceIdx < 6; ++FaceIdx)
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
				Pitch = Pitch * 2;

				// 使用默认方向的 TriSlopeFaces 作为基础
				// 对于三角锥，Pitch 只有 0 和 1，Pitch=1 表示绕Y轴旋转90度
				for (int32 FaceIdx = 0; FaceIdx < 6; ++FaceIdx)
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
				Pitch = Pitch * 2;

				// 使用默认方向的 TriSlopeComplementFaces 作为基础
				// 对于三角锥互补体，Pitch 只有 0 和 1，Pitch=1 表示绕Y轴旋转90度
				for (int32 FaceIdx = 0; FaceIdx < 6; ++FaceIdx)
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

void AVoxelTile::SetVoxelWithBlockType(int32 X, int32 Y, int32 Z, uint8 TextureID, uint8 Layer, uint8 BlockType, uint8 Yaw, uint8 Pitch, bool bUpdateMesh)
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
	UCVoxelData_SetYawAndPitch(VoxelData, Yaw & 0x03, Pitch & 0x03);
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

void AVoxelTile::GetFaceTriangles(int32 X, int32 Y, int32 Z, int32 FaceIndex, uint8 BlockType, int32 DirectionIndex, const UCVoxelData& Voxel, bool bFlat, TArray<FIntVertex>& OutVertices, TArray<FIntVector>& OutFaceIndexes) const
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

	int32 RelativeFaceIndex = BlockShapeGenerator->AllBlockFaceDirectionsReverse[BlockType][DirectionIndex][FaceIndex];
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

	FIntVector V0 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[0])) * 2;
	FIntVector V1 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[1])) * 2;
	FIntVector V2 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[2])) * 2;
	FIntVector V3 = LocalToWorldPosition(ConvertVertexIndexToGridPos(FaceDef.Indices[3])) * 2;

	FIntVector Dis0 = V1 - V0;
	FIntVector Dis1 = V2 - V0;

	int32 OffX = -1;
	int32 OffY = -1;
	if (Dis0.Z == 0 && Dis1.Z == 0)
	{
		OffX = 0;
		OffY = 1;
	}
	else if (Dis0.Y == 0 && Dis1.Y == 0)
	{
		OffX = 0;
		OffY = 2;
	}
	else if (Dis0.X == 0 && Dis1.X == 0)
	{
		OffX = 1;
		OffY = 2;
	}
	else
	{
		FIntVector Dis2 = V3 - V0;
		OffX = 0;
		OffY = 1;

		Dis0 = FIntVector(Dis2.X, 0, 0);
		Dis1 = FIntVector(0, Dis2.Y, 0);
	}
	FIntVector2 UV0(0, 0);
	FIntVector2 UV1(Dis0[OffX], Dis0[OffY]);
	FIntVector2 UV2(Dis1[OffX], Dis1[OffY]);

	// 异形体的Top面（索引4，斜面）：通过顶点计算法向量
	FVector Edge0 = FVector(V1.X - V0.X, V1.Y - V0.Y, V1.Z - V0.Z);
	FVector Edge1 = FVector(V2.X - V0.X, V2.Y - V0.Y, V2.Z - V0.Z);
	FVector Normal0 = FVector::CrossProduct(Edge0, Edge1).GetSafeNormal();
	// 如果归一化失败，使用默认值
	if (Normal0.IsNearlyZero())
		Normal0 = FVector(0, 0, 1); // 默认向上

	FVector Normal1 = Normal0;
	if (FaceDef.Count == 4)
	{
		FVector Edge2 = FVector(V1.X - V3.X, V1.Y - V3.Y, V1.Z - V3.Z);
		FVector Edge3 = FVector(V2.X - V3.X, V2.Y - V3.Y, V2.Z - V3.Z);
		Normal1 = FVector::CrossProduct(Edge3, Edge2).GetSafeNormal();
		// 如果归一化失败，使用默认值
		if (Normal1.IsNearlyZero())
			Normal1 = FVector(0, 0, 1); // 默认向上
	}

	if (bFlat)
	{
		// 简单模式：2个三角形
		if (FaceDef.Count >= 4)
		{
			FIntVector Dis2 = V3 - V0;
			FIntVector2 UV3(Dis2[OffX], Dis2[OffY]);

			OutVertices.Add(FIntVertex(V0, Normal0, UV0, TriangleColor));
			OutVertices.Add(FIntVertex(V1, Normal0, UV1, TriangleColor));
			OutVertices.Add(FIntVertex(V2, Normal0, UV2, TriangleColor));
			OutVertices.Add(FIntVertex(V3, Normal0, UV3, TriangleColor));

			OutFaceIndexes.Add(FIntVector(0, 2, 1));
			OutFaceIndexes.Add(FIntVector(3, 1, 2));
		}
	}
	else
	{
		if (FaceDef.Count == 3)
		{
			FIntVector Dis2 = V3 - V0;
			FIntVector2 UV3(Dis2[OffX], Dis2[OffY]);

			FIntVector Center = (V1 + V2) / 2;
			FIntVector2 CenterUV = (UV1 + UV2) / 2;

			OutVertices.Add(FIntVertex(V0, Normal0, UV0, TriangleColor));
			OutVertices.Add(FIntVertex(V1, Normal0, UV1, TriangleColor));
			OutVertices.Add(FIntVertex(V2, Normal0, UV2, TriangleColor));
			OutVertices.Add(FIntVertex(Center, Normal0, CenterUV, TriangleColor));

			OutFaceIndexes.Add(FIntVector(0, 3, 1));
			OutFaceIndexes.Add(FIntVector(0, 2, 3));
		}
		else if (FaceDef.Count == 4)
		{
			FIntVector Dis2 = V3 - V0;
			FIntVector2 UV3(Dis2[OffX], Dis2[OffY]);

			FIntVector Center = (V1 + V2) / 2;
			FIntVector2 CenterUV = (UV1 + UV2) / 2;

			if (Normal0 != Normal1)
			{
				OutVertices.Add(FIntVertex(V0, Normal0, UV0, TriangleColor));
				OutVertices.Add(FIntVertex(V1, Normal0, UV1, TriangleColor));
				OutVertices.Add(FIntVertex(V2, Normal0, UV2, TriangleColor));
				OutVertices.Add(FIntVertex(Center, Normal0, CenterUV, TriangleColor));

				OutFaceIndexes.Add(FIntVector(0, 3, 1));
				OutFaceIndexes.Add(FIntVector(0, 2, 3));

				OutVertices.Add(FIntVertex(V3, Normal1, UV3, TriangleColor));
				OutVertices.Add(FIntVertex(V2, Normal1, UV2, TriangleColor));
				OutVertices.Add(FIntVertex(V1, Normal1, UV1, TriangleColor));
				OutVertices.Add(FIntVertex(Center, Normal1, CenterUV, TriangleColor));

				OutFaceIndexes.Add(FIntVector(4, 7, 5));
				OutFaceIndexes.Add(FIntVector(4, 6, 7));
			}
			else
			{
				OutVertices.Add(FIntVertex(V0, Normal0, UV0, TriangleColor));
				OutVertices.Add(FIntVertex(V1, Normal0, UV1, TriangleColor));
				OutVertices.Add(FIntVertex(V2, Normal0, UV2, TriangleColor));
				OutVertices.Add(FIntVertex(V3, Normal0, UV3, TriangleColor));
				OutVertices.Add(FIntVertex(Center, Normal0, CenterUV, TriangleColor));

				OutFaceIndexes.Add(FIntVector(0, 4, 1));
				OutFaceIndexes.Add(FIntVector(0, 2, 4));
				OutFaceIndexes.Add(FIntVector(3, 4, 2));
				OutFaceIndexes.Add(FIntVector(3, 1, 4));
			}
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
	// 清空之前的网格数据（按TextureID分组）
	MeshSections.Empty();
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
	const int32 UVSizeX = VOXEL_TILE_SIZE_X + 1;  // 33
	const int32 UVSizeY = VOXEL_TILE_SIZE_Y + 1;  // 33
	const int32 UVSizeZ = VOXEL_TILE_SIZE_Z + 1;  // 65
	FIntVector HalfSize(VOXEL_TILE_SIZE_X / 2, VOXEL_TILE_SIZE_Y / 2, VOXEL_TILE_SIZE_Z / 2);

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
				
				// 遍历所有面（所有块类型都是6个面）
				int32 MaxFaceIndex = 6;
				for (int32 FaceIndex = 0; FaceIndex < MaxFaceIndex; ++FaceIndex)
				{
					// 获取面的三角形数组
					TArray<FIntVertex> FaceVertices;
					TArray<FIntVector> FacesIndexes;
					bool bFlat = IsFaceFlat(X, Y, Z, FaceIndex);
					GetFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, Voxel, bFlat, FaceVertices, FacesIndexes);

					if (FacesIndexes.Num() == 0)
						continue;

					int32 NeighborFaceIndex = GetNeighborFaceInfo(FaceIndex);
					if (NeighborFaceIndex == -1)
						continue;

					const FIntVector& FaceDir = FaceDirections[FaceIndex];
					FIntVector Neighbor(X + FaceDir.X, Y + FaceDir.Y, Z + FaceDir.Z);
					TArray<FIntVertex> NeighborFaceVertices;
					TArray<FIntVector> NeighborFacesIndexes;
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

							GetFaceTriangles(Neighbor.X, Neighbor.Y, Neighbor.Z, NeighborFaceIndex, NeighborBlockType, NeighborDirectionIndex, NeighborVoxel, bNeighborFlat, NeighborFaceVertices, NeighborFacesIndexes);
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

					UCSimpleMap<ucINT, ucINT> MapTextureIDs;
					// 计算该面的TextureID key（使用4个顶点的TextureID，排序后组合成int64）
					int64 TextureKey = 0;
					if (FaceVertices.Num() > 0 && TileData && TileData->AryTextureIDs.GetSize() > 0)
					{
						for (int32 VertexIdx = 0; VertexIdx < FaceVertices.Num(); ++VertexIdx)
						{
							FIntVector VertexWorldPos = FaceVertices[VertexIdx].V;
							FIntVector VertexPos = VertexWorldPos / 2 + HalfSize;
							int32 UVIndex = (VertexPos.Z + 1) * UVSizeY * UVSizeX + (VertexPos.Y + 1) * UVSizeX + (VertexPos.X + 1);
							
							if (UVIndex >= 0 && UVIndex < TileData->AryTextureIDs.GetSize())
							{
								// 读取顶点的TextureID（0表示无纹理，1表示第一个纹理）
								int32 TextureID = TileData->AryTextureIDs[UVIndex];
								MapTextureIDs.Add(TextureID, TextureID);
							}
						}

						for (int32 i = 0; i < MapTextureIDs.GetSize(); i++)
						{
							int64 TextureID = MapTextureIDs.GetValueAt(i);

							TextureKey |= (TextureID << (16 * i));
						}
					}
					
					// 获取或创建对应的MeshSection
					FMeshSectionData& SectionData = MeshSections.FindOrAdd(TextureKey);
					if (!SectionData.Dirty)
						continue;

					int32 RandBlockID = randint(0, 10) == 10 ? randint(16, 31) : 0;

					TArray<int32> AryCornerTextureID;
					for (ucINT i = 0; i < FaceVertices.Num(); i++)
					{
						// 获取第一个顶点的世界坐标（已乘以2）
						FIntVector VertexWorldPos = FaceVertices[i].V;

						FIntVector VertexPos = VertexWorldPos / 2 + HalfSize;
						int32 UVIndex = (VertexPos.Z + 1) * UVSizeY * UVSizeX + (VertexPos.Y + 1) * UVSizeX + (VertexPos.X + 1);

						SectionData.MapVertices.Add(UVIndex, UVIndex);
						if (UVIndex >= 0 && UVIndex < TileData->AryTextureIDs.GetSize())
						{
							int32 TextureID = TileData->AryTextureIDs[UVIndex];
							FWar3TextureInfo TextureInfo = GetTextureInfoByID(TextureID);

							AryCornerTextureID.Add(TextureID);
						}
					}

					ucINT TextureSize = MapTextureIDs.GetSize();
					
					int32 TotalBlockID[4] = { 0, 0, 0, 0 };
					if (AryCornerTextureID.Num() == 4 || AryCornerTextureID.Num() == 5)
					{
						for (ucINT i = 0; i < 4; i++)
						{
							int32 TextureID = AryCornerTextureID[i];
							ucINT UVPos = MapTextureIDs.FindKey(TextureID);

							ucINT UV_Y = i / 2;
							ucINT UV_X = i % 2;

							TotalBlockID[UVPos] |= 1 << (UV_Y * 2 + (1 - UV_X));
						}
					}

					for (ucINT i = 0; i < MapTextureIDs.GetSize(); i++)
					{
						FWar3TextureInfo TextureInfo = GetTextureInfoByID(MapTextureIDs.GetValueAt(i));

						// 如果是0，随机从16到31（仅对8列纹理）
						if (TotalBlockID[i] == 0x0F && TextureInfo.Columns == 8)
							TotalBlockID[i] = RandBlockID;
					}

					int32 BaseIndex = SectionData.Vertices.Num();
					for (ucINT i = 0; i < FaceVertices.Num(); i++)
					{
						// 获取第一个顶点的世界坐标（已乘以2）
						FIntVector VertexWorldPos = FaceVertices[i].V;

						FVector Location(VertexWorldPos);
						SectionData.Vertices.Add(Location / 2.0f * VoxelSize - TileWorldGridPos);
						SectionData.Normals.Add(FaceVertices[i].Normal);

						FVector2D BaseUV((float)FaceVertices[i].UV.X / 2.0f, (float)FaceVertices[i].UV.Y / 2.0f);

						// 处理4层纹理（UVs0[0] 到 UVs0[3]）
						FVector2D LayerUVs[4] = { FVector2D(0, 0), FVector2D(0, 0), FVector2D(0, 0), FVector2D(0, 0) };
						for (ucINT j = 0; j < MapTextureIDs.GetSize(); j++)
						{
							int32 TextureID = MapTextureIDs.GetValueAt(j);
							int32 TextureBlockID = TotalBlockID[j];

							FWar3TextureInfo TextureInfo = GetTextureInfoByID(TextureID);

							float MinU = 8.0f / (float)TextureInfo.TextureWidth;
							float MinV = 8.0f / (float)TextureInfo.TextureHeight;
							BaseUV.X = FMath::Clamp(FMath::Abs(BaseUV.X), MinU, 1.0f - MinU);
							BaseUV.Y = FMath::Clamp(FMath::Abs(BaseUV.Y), MinV, 1.0f - MinV);
							
							// 计算该层的UV
							int32 U = TextureBlockID / 4;
							int32 V = TextureBlockID % 4;
							FVector2D CalculatedUV;
							if (TextureInfo.Columns == 8)
								CalculatedUV = FVector2D((U + FMath::Abs(BaseUV.X)) * 0.125f, (V + FMath::Abs(BaseUV.Y)) * 0.25f);
							else
								CalculatedUV = FVector2D((U + FMath::Abs(BaseUV.X)) * 0.25f, (V + FMath::Abs(BaseUV.Y)) * 0.25f);

							LayerUVs[j] = CalculatedUV;
						}

						// 每个顶点都有所有层的UV数据（即使某些层是(0,0)）
						SectionData.UVs0.Add(LayerUVs[0]);  // UV0 - 第一层
						SectionData.UVs1.Add(LayerUVs[1]);  // UV1 - 第二层
						SectionData.UVs2.Add(LayerUVs[2]);  // UV2 - 第三层
						SectionData.UVs3.Add(LayerUVs[3]);  // UV3 - 第四层

						FColor VertexColor;
						VertexColor.R = TextureSize > 1 ? 255 : 0;  // 第2层纹理开关
						VertexColor.G = TextureSize > 2 ? 255 : 0;  // 第3层纹理开关
						VertexColor.B = TextureSize > 3 ? 255 : 0;  // 第4层纹理开关
						VertexColor.A = FaceVertices[i].Color.A;  // 保持原有的Alpha通道
						
						SectionData.VertexColors.Add(VertexColor);
						SectionData.Tangents.Add(FProcMeshTangent(0, 0, 1));
					}
					for (const FIntVector& Face : TrianglesToRender)
					{
						SectionData.Triangles.Add(BaseIndex + Face.X);
						SectionData.Triangles.Add(BaseIndex + Face.Y);
						SectionData.Triangles.Add(BaseIndex + Face.Z);
					}
				}
			}
		}
	}
}

void AVoxelTile::UpdateMesh(bool Active)
{
	// 如果已经有待处理的刷新请求，忽略本次请求
	if (bMeshUpdatePending)
		return;

	//ClearMeshData();
	if (!Active)
	{
		// 清除所有MeshSection
		for (int32 SectionIndex = 0; SectionIndex < ProceduralMesh->GetNumSections(); ++SectionIndex)
		{
			ProceduralMesh->ClearMeshSection(SectionIndex);
		}
		return;
	}

	// 标记有待处理的刷新
	bMeshUpdatePending = true;

	for (auto& Pair : MeshSections)
	{
		FMeshSectionData& SectionData = Pair.Value;
		if (!SectionData.Dirty)
			continue;
		SectionData.MapVertices.Empty();
	}

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
			0.025f, // 1秒延迟
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
	
	// 清除所有现有的MeshSection
// 	for (int32 SectionIndex = 0; SectionIndex < ProceduralMesh->GetNumSections(); ++SectionIndex)
// 	{
// 		for (auto& pair : MeshSections)
// 		{
// 			if (pair.Value.Dirty)
// 				ProceduralMesh->ClearMeshSection(SectionIndex);
// 		}
// 	}
	
	// 获取纹理列表
	TArray<UTexture2D*> TextureList;
	if (Terrain)
	{
		TextureList = Terrain->GetTextureList();
	}
	
	// 为每个TextureKey创建独立的MeshSection（DrawCall）
	for (auto& Pair : MeshSections)
	{
		int64 TextureKey = Pair.Key;  // 4个顶点TextureID排序后组合成的int64
		FMeshSectionData& SectionData = Pair.Value;
		
		// 跳过空的Section
		if (SectionData.Vertices.Num() == 0 || SectionData.Triangles.Num() == 0)
			continue;
 		if (!SectionData.Dirty)
 			continue;
		
		if (SectionData.Index == -1)
			SectionData.Index = ProceduralMesh->GetNumSections();
		
		ProceduralMesh->ClearMeshSection(SectionData.Index);
		// 创建MeshSection
		ProceduralMesh->CreateMeshSection(
			SectionData.Index,
			SectionData.Vertices,
			SectionData.Triangles,
			SectionData.Normals,
			SectionData.UVs0,   // UV0 - 第一层
			SectionData.UVs1,  // UV1 - 第二层
			SectionData.UVs2,  // UV2 - 第三层
			SectionData.UVs3,  // UV3 - 第四层
			SectionData.VertexColors,
			SectionData.Tangents,
			true // 启用碰撞
		);

		// 为每个Section设置材质
		if (Material)
		{
			// 从TextureKey中提取4个顶点的TextureID（已排序）
			uint16 TextureID0 = (uint16)(TextureKey & 0xFFFF);
			uint16 TextureID1 = (uint16)((TextureKey >> 16) & 0xFFFF);
			uint16 TextureID2 = (uint16)((TextureKey >> 32) & 0xFFFF);
			uint16 TextureID3 = (uint16)((TextureKey >> 48) & 0xFFFF);

			// 为这个Section创建MaterialInstanceDynamic
			UMaterialInstanceDynamic* SectionMaterial = UMaterialInstanceDynamic::Create(Material, this);

			// 设置4层纹理参数（根据TextureID从纹理列表中获取）
			// TextureID从0开始（0表示第一个纹理）
			for (int32 LayerIndex = 0; LayerIndex < 4; ++LayerIndex)
			{
				FName TextureParamName = FName(*FString::Printf(TEXT("Texture%d"), LayerIndex));
				UTexture2D* LayerTexture = nullptr;

				// 根据该层的TextureID获取纹理
				uint16 LayerTextureID = -1;
				if (LayerIndex == 0) LayerTextureID = TextureID0;
				else if (LayerIndex == 1) LayerTextureID = TextureID1;
				else if (LayerIndex == 2) LayerTextureID = TextureID2;
				else if (LayerIndex == 3) LayerTextureID = TextureID3;

				// TextureID从0开始
				if (LayerTextureID >= 0 && LayerTextureID < TextureList.Num())
					LayerTexture = TextureList[LayerTextureID];
				else if (TextureList.Num() > 0)
					LayerTexture = TextureList[0];

				if (LayerTexture)
				{
					SectionMaterial->SetTextureParameterValue(TextureParamName, LayerTexture);
				}
				else
				{
					// 如果没有纹理，设置为null
					SectionMaterial->SetTextureParameterValue(TextureParamName, nullptr);
				}
			}

			ProceduralMesh->SetMaterial(SectionData.Index, SectionMaterial);
		}
		else
		{
			ProceduralMesh->SetMaterial(SectionData.Index, nullptr);
		}
	}
	
	for (auto& Pair : MeshSections)
		Pair.Value.Dirty = false;
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

UTexture2D* AVoxelTile::GetTextureByID(int32 TextureID) const
{
	if (Terrain)
	{
		return Terrain->GetTextureByID(TextureID);
	}
	return nullptr;
}

FWar3TextureInfo AVoxelTile::GetTextureInfoByID(int32 TextureID) const
{
	if (Terrain)
	{
		return Terrain->GetTextureInfoByID(TextureID);
	}
	return FWar3TextureInfo();
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
