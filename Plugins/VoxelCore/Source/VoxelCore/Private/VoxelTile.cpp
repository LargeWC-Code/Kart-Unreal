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
static const FVector2D BaseCubeUVs[8][6] = {
	// 顶点0: (0,0,0) 前左下
	{ FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f) }, // Left, Front, Right, Back, Top, Bottom
	// 顶点1: (1,0,0) 后左下
	{ FVector2D(1.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(1.0f, 0.0f) },
	// 顶点2: (1,1,0) 后右下
	{ FVector2D(1.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(1.0f, 1.0f), FVector2D(1.0f, 1.0f) },
	// 顶点3: (0,1,0) 前右下
	{ FVector2D(0.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(0.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f) },
	// 顶点4: (0,0,1) 前左上
	{ FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), FVector2D(0.0f, 0.0f), FVector2D(0.0f, 0.0f) },
	// 顶点5: (1,0,1) 后左上
	{ FVector2D(1.0f, 1.0f), FVector2D(0.0f, 1.0f), FVector2D(1.0f, 1.0f), FVector2D(0.0f, 1.0f), FVector2D(1.0f, 0.0f), FVector2D(1.0f, 0.0f) },
	// 顶点6: (1,1,1) 后右上
	{ FVector2D(1.0f, 1.0f), FVector2D(1.0f, 1.0f), FVector2D(1.0f, 1.0f), FVector2D(1.0f, 1.0f), FVector2D(1.0f, 1.0f), FVector2D(1.0f, 1.0f) },
	// 顶点7: (0,1,1) 前右上
	{ FVector2D(0.0f, 1.0f), FVector2D(1.0f, 1.0f), FVector2D(0.0f, 1.0f), FVector2D(1.0f, 1.0f), FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f) }
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
	// 楔形体：12个方向（Yaw: 0-3, Pitch: 0-2），方向索引 = Yaw * 3 + Pitch
	FaceIndices SlopeFaces[12][7];

	// 楔形体：12个方向对应的面索引映射（将原来的0-5面索引映射到旋转后的绝对面索引0-5）
	// SlopeFaceDirections[DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
	int32 SlopeFaceDirections[12][6];

	// 楔形体：12个方向对应的反向面索引映射（将绝对面索引0-5映射回自己的相对面索引0-5）
	// SlopeFaceDirectionsReverse[DirectionIndex][AbsoluteFaceIndex] = RelativeFaceIndex
	int32 SlopeFaceDirectionsReverse[12][6];

	// 三角锥：8个方向（Yaw: 0-3, Pitch: 0-1），方向索引 = Yaw * 2 + Pitch
	FaceIndices TriSlopeFaces[8][7];

	// 三角锥：8个方向对应的面索引映射（将原来的0-5面索引映射到旋转后的绝对面索引0-5）
	// TriSlopeFaceDirections[DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
	int32 TriSlopeFaceDirections[8][6];

	// 三角锥：8个方向对应的反向面索引映射（将绝对面索引0-5映射回自己的相对面索引0-5）
	// TriSlopeFaceDirectionsReverse[DirectionIndex][AbsoluteFaceIndex] = RelativeFaceIndex
	int32 TriSlopeFaceDirectionsReverse[8][6];

	// 三角锥互补体：8个方向（Yaw: 0-3, Pitch: 0-1），方向索引 = Yaw * 2 + Pitch
	FaceIndices TriSlopeComplementFaces[8][7];

	// 三角锥互补体：8个方向对应的面索引映射（将原来的0-5面索引映射到旋转后的绝对面索引0-5）
	// TriSlopeComplementFaceDirections[DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
	int32 TriSlopeComplementFaceDirections[8][6];

	// 三角锥互补体：8个方向对应的反向面索引映射（将绝对面索引0-5映射回自己的相对面索引0-5）
	// TriSlopeComplementFaceDirectionsReverse[DirectionIndex][AbsoluteFaceIndex] = RelativeFaceIndex
	int32 TriSlopeComplementFaceDirectionsReverse[8][6];

	FVoxelBlockShapeGenerator()
	{
		// 使用默认方向的静态数组作为基础（Yaw=0, Pitch=0）
		// 生成所有旋转方向的顶点数据
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
					TransformFace(SlopeFaces[DirectionIndex][FaceIdx], ::SlopeFaces[FaceIdx], Yaw, Pitch);
				}

				// 生成对应方向的面的索引映射（6个面：Left, Front, Right, Back, Top, Bottom）
				// SlopeFaceDirections[DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
				for (int32 RelativeFaceIdx = 0; RelativeFaceIdx < 6; ++RelativeFaceIdx)
				{
					int32 AbsoluteFaceIdx = TransformFaceIndex(RelativeFaceIdx, Yaw, Pitch);
					SlopeFaceDirections[DirectionIndex][RelativeFaceIdx] = AbsoluteFaceIdx;
					// 生成反向映射：AbsoluteFaceIndex -> RelativeFaceIndex
					SlopeFaceDirectionsReverse[DirectionIndex][AbsoluteFaceIdx] = RelativeFaceIdx;
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
					TransformFace(TriSlopeFaces[DirectionIndex][FaceIdx], ::TriSlopeFaces[FaceIdx], Yaw, Pitch);
				}

				// 生成对应方向的面的索引映射（6个面：Left, Front, Right, Back, Top, Bottom）
				// TriSlopeFaceDirections[DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
				for (int32 RelativeFaceIdx = 0; RelativeFaceIdx < 6; ++RelativeFaceIdx)
				{
					int32 AbsoluteFaceIdx = TransformFaceIndex(RelativeFaceIdx, Yaw, Pitch);
					TriSlopeFaceDirections[DirectionIndex][RelativeFaceIdx] = AbsoluteFaceIdx;
					// 生成反向映射：AbsoluteFaceIndex -> RelativeFaceIndex
					TriSlopeFaceDirectionsReverse[DirectionIndex][AbsoluteFaceIdx] = RelativeFaceIdx;
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
					TransformFace(TriSlopeComplementFaces[DirectionIndex][FaceIdx], ::TriSlopeComplementFaces[FaceIdx], Yaw, Pitch);
				}

				// 生成对应方向的面的索引映射（6个面：Left, Front, Right, Back, Top, Bottom）
				// TriSlopeComplementFaceDirections[DirectionIndex][RelativeFaceIndex] = AbsoluteFaceIndex
				for (int32 RelativeFaceIdx = 0; RelativeFaceIdx < 6; ++RelativeFaceIdx)
				{
					int32 AbsoluteFaceIdx = TransformFaceIndex(RelativeFaceIdx, Yaw, Pitch);
					TriSlopeComplementFaceDirections[DirectionIndex][RelativeFaceIdx] = AbsoluteFaceIdx;
					// 生成反向映射：AbsoluteFaceIndex -> RelativeFaceIndex
					TriSlopeComplementFaceDirectionsReverse[DirectionIndex][AbsoluteFaceIdx] = RelativeFaceIdx;
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

FIntVector AVoxelTile::LocalToWorldPosition(const FIntVector& LocalGridPosition) const
{
	const int32 HalfTileSizeX = VOXEL_TILE_SIZE_X / 2; // 16
	const int32 HalfTileSizeY = VOXEL_TILE_SIZE_Y / 2; // 16
	const int32 HalfTileSizeZ = VOXEL_TILE_SIZE_Z / 2; // 32

	// 将存储坐标转换为相对于Tile中心的局部网格坐标
	int32 LocalOffsetX = LocalGridPosition.X - HalfTileSizeX;
	int32 LocalOffsetY = LocalGridPosition.Y - HalfTileSizeY;
	int32 LocalOffsetZ = LocalGridPosition.Z - HalfTileSizeZ;

	// 获取Tile的世界网格位置（Tile中心在世界网格中的坐标）
	// Tile的世界网格位置 = TileCoord * TileSize
	const int32 TileSizeX = VOXEL_TILE_SIZE_X;
	const int32 TileSizeY = VOXEL_TILE_SIZE_Y;
	FIntVector TileWorldGridPos(TileCoord.X * TileSizeX, TileCoord.Y * TileSizeY, 0);

	// 计算体素的世界网格坐标（Tile世界网格位置 + 局部偏移）
	FIntVector WorldGridPos = TileWorldGridPos + FIntVector(
		LocalOffsetX,
		LocalOffsetY,
		LocalOffsetZ
	);

	return WorldGridPos;
}

FIntTriangle AVoxelTile::SortTriangleVertices(const FIntVector& V0, const FIntVector& V1, const FIntVector& V2)
{
	// 创建一个数组来存储三个顶点
	FIntVector Vertices[3] = { V0, V1, V2 };

	// 按 Z、X、Y 顺序排序
	// 比较函数：先比较 Z，如果相等则比较 X，如果还相等则比较 Y
	auto CompareVertices = [](const FIntVector& A, const FIntVector& B) -> bool
		{
			if (A.Z != B.Z)
				return A.Z < B.Z;  // 先按 Z 排序
			if (A.X != B.X)
				return A.X < B.X;  // Z 相等时按 X 排序
			return A.Y < B.Y;      // Z 和 X 都相等时按 Y 排序
		};

	// 使用冒泡排序（只有3个元素，简单高效）
	for (int32 i = 0; i < 3; ++i)
	{
		for (int32 j = 0; j < 2 - i; ++j)
		{
			if (!CompareVertices(Vertices[j], Vertices[j + 1]))
			{
				// 交换
				FIntVector Temp = Vertices[j];
				Vertices[j] = Vertices[j + 1];
				Vertices[j + 1] = Temp;
			}
		}
	}

	// 返回排序后的三角形
	return FIntTriangle(Vertices[0], Vertices[1], Vertices[2]);
}

int32 AVoxelTile::GetNeighborFaceInfo(int32 FaceIndex)
{
	// 对应面的ID：对面关系
	// 0(Left) -> 2(Right), 1(Front) -> 3(Back), 2(Right) -> 0(Left), 
	// 3(Back) -> 1(Front), 4(Top) -> 5(Bottom), 5(Bottom) -> 4(Top)
	int32 OppositeFaceIndex = FaceIndex < 4 ? (FaceIndex + 2) % 4 : !(FaceIndex - 4) + 4;
	return OppositeFaceIndex;
}

FIntTriangle AVoxelTile::GetFaceTriangleVertices(int32 X, int32 Y, int32 Z, int32 FaceIndex) const
{
	// 验证面索引
	if (FaceIndex < 0 || FaceIndex >= 6)
	{
		return FIntTriangle(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector::ZeroValue);
	}

	// 获取面的顶点索引定义（使用Box的标准面定义）
	const FaceIndices& FaceDef = BoxFaces[FaceIndex];

	if (FaceDef.Count < 3)
	{
		// 如果面的顶点数不足3个，返回无效三角形
		return FIntTriangle(FIntVector::ZeroValue, FIntVector::ZeroValue, FIntVector::ZeroValue);
	}

	// 将局部顶点索引转换为体素网格坐标
	// BaseCubeVertices定义的是单位立方体(0-1)，需要转换为体素网格坐标
	// 顶点索引到网格坐标的转换：
	// 顶点0(0,0,0) -> (X, Y, Z)
	// 顶点1(1,0,0) -> (X+1, Y, Z)
	// 顶点2(1,1,0) -> (X+1, Y+1, Z)
	// 顶点3(0,1,0) -> (X, Y+1, Z)
	// 顶点4(0,0,1) -> (X, Y, Z+1)
	// 顶点5(1,0,1) -> (X+1, Y, Z+1)
	// 顶点6(1,1,1) -> (X+1, Y+1, Z+1)
	// 顶点7(0,1,1) -> (X, Y+1, Z+1)

	FIntVector V0, V1, V2;

	// 获取前三个顶点的索引
	int32 VertexIndex0 = FaceDef.Indices[0];
	int32 VertexIndex1 = FaceDef.Indices[1];
	int32 VertexIndex2 = FaceDef.Indices[2];

	// 将顶点索引转换为网格坐标
	auto ConvertVertexIndexToGridPos = [X, Y, Z](int32 VertexIndex) -> FIntVector
		{
			// BaseCubeVertices的顶点定义：
			// 0: (0,0,0), 1: (1,0,0), 2: (1,1,0), 3: (0,1,0)
			// 4: (0,0,1), 5: (1,0,1), 6: (1,1,1), 7: (0,1,1)
			// 转换为网格坐标：(X + vx, Y + vy, Z + vz)
			const FVector& LocalVertex = BaseCubeVertices[VertexIndex];
			return FIntVector(
				X + (int32)LocalVertex.X,
				Y + (int32)LocalVertex.Y,
				Z + (int32)LocalVertex.Z
			);
		};

	V0 = ConvertVertexIndexToGridPos(VertexIndex0);
	V1 = ConvertVertexIndexToGridPos(VertexIndex1);
	V2 = ConvertVertexIndexToGridPos(VertexIndex2);

	// 使用SortTriangleVertices对三个顶点进行排序
	return SortTriangleVertices(V0, V1, V2);
}

void AVoxelTile::GetFaceTriangles(int32 X, int32 Y, int32 Z, int32 FaceIndex, uint8 BlockType, int32 DirectionIndex, const UCVoxelData& Voxel, bool bFlat, TArray<FIntTriangle>& OutTriangles) const
{
	FVoxelBlockShapeGenerator* BlockShapeGenerator = GetBlockShapeGenerator();
	OutTriangles.Empty();

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

	switch (BlockType)
	{
	case UCVoxelBlockType_Cube:
	{
		const FaceIndices& FaceDef = BoxFaces[FaceIndex];
		if (FaceDef.Count == 0)
			return;

		if (bFlat)
		{
			// 简单模式：2个三角形
			if (FaceDef.Count >= 4)
			{
				FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
				FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
				FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
				FIntVector V3 = ConvertVertexIndexToGridPos(FaceDef.Indices[3]);

				OutTriangles.Add(SortTriangleVertices(V0, V2, V1));
				OutTriangles.Add(SortTriangleVertices(V0, V3, V2));
			}
		}
		else
		{
			// AO模式：4个三角形（中心点在面的中心）
			if (FaceDef.Count >= 4)
			{
				FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
				FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
				FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
				FIntVector V3 = ConvertVertexIndexToGridPos(FaceDef.Indices[3]);

				// 计算中心点（面的中心）
				FIntVector Center(
					(V0.X + V1.X + V2.X + V3.X) / 4,
					(V0.Y + V1.Y + V2.Y + V3.Y) / 4,
					(V0.Z + V1.Z + V2.Z + V3.Z) / 4
				);

				OutTriangles.Add(SortTriangleVertices(Center, V1, V0));
				OutTriangles.Add(SortTriangleVertices(Center, V2, V1));
				OutTriangles.Add(SortTriangleVertices(Center, V3, V2));
				OutTriangles.Add(SortTriangleVertices(Center, V0, V3));
			}
		}
		break;
	}
	case UCVoxelBlockType_SquareSlope:
	{
		int32 RelativeFaceIndex = FaceIndex > 5 ? FaceIndex : BlockShapeGenerator->SlopeFaceDirectionsReverse[DirectionIndex][FaceIndex];
		const FaceIndices& FaceDef = BlockShapeGenerator->SlopeFaces[DirectionIndex][RelativeFaceIndex];
		if (FaceDef.Count == 0)
			return;

		// 计算中心点
		FIntVector Center(0, 0, 0);
		int32 NumPointsForCenter = 0;
		for (int32 i = 0; i < 4; ++i)
		{
			int32 VertexIndex = FaceDef.Indices[i];
			if (VertexIndex == -1)
				break;
			FIntVector V = ConvertVertexIndexToGridPos(VertexIndex);
			Center += V;
			NumPointsForCenter++;
		}
		if (NumPointsForCenter > 0)
		{
			Center.X /= NumPointsForCenter;
			Center.Y /= NumPointsForCenter;
			Center.Z /= NumPointsForCenter;
		}

		if (FaceDef.Count == 3)
		{
			// 三角形拆解成2个三角形
			FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
			FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
			FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);

			OutTriangles.Add(SortTriangleVertices(V0, Center, V1));
			OutTriangles.Add(SortTriangleVertices(V0, V2, Center));
		}
		else if (FaceDef.Count == 4)
		{
			// 四边形拆解成4个三角形
			FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
			FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
			FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
			FIntVector V3 = ConvertVertexIndexToGridPos(FaceDef.Indices[3]);

			OutTriangles.Add(SortTriangleVertices(V0, Center, V1));
			OutTriangles.Add(SortTriangleVertices(V1, Center, V2));
			OutTriangles.Add(SortTriangleVertices(V2, Center, V3));
			OutTriangles.Add(SortTriangleVertices(V3, Center, V0));
		}
		break;
	}
	case UCVoxelBlockType_TriangularSlope:
	{
		int32 RelativeFaceIndex = FaceIndex > 5 ? FaceIndex : BlockShapeGenerator->TriSlopeFaceDirectionsReverse[DirectionIndex][FaceIndex];
		const FaceIndices& FaceDef = BlockShapeGenerator->TriSlopeFaces[DirectionIndex][RelativeFaceIndex];
		if (FaceDef.Count == 0)
			return;

		if (FaceIndex < 6)
		{
			// 计算中心点
			FIntVector Center(0, 0, 0);
			int32 NumPointsForCenter = 0;
			for (int32 i = 0; i < 4; ++i)
			{
				int32 VertexIndex = FaceDef.Indices[i];
				if (VertexIndex == -1)
					break;
				FIntVector V = ConvertVertexIndexToGridPos(VertexIndex);
				Center += V;
				NumPointsForCenter++;
			}
			if (NumPointsForCenter > 0)
			{
				Center.X /= NumPointsForCenter;
				Center.Y /= NumPointsForCenter;
				Center.Z /= NumPointsForCenter;
			}

			FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
			FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
			FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);

			OutTriangles.Add(SortTriangleVertices(V0, Center, V1));
			OutTriangles.Add(SortTriangleVertices(V0, V2, Center));
		}
		else
		{
			// 单个三角形
			FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
			FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
			FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
			OutTriangles.Add(SortTriangleVertices(V0, V2, V1));
		}
		break;
	}
	case UCVoxelBlockType_TriangularComplement:
	{
		int32 RelativeFaceIndex = FaceIndex > 5 ? FaceIndex : BlockShapeGenerator->TriSlopeComplementFaceDirectionsReverse[DirectionIndex][FaceIndex];
		const FaceIndices& FaceDef = BlockShapeGenerator->TriSlopeComplementFaces[DirectionIndex][RelativeFaceIndex];
		if (FaceDef.Count == 0)
			return;

		// 计算中心点
		FIntVector Center(0, 0, 0);
		int32 NumPointsForCenter = 0;
		for (int32 i = 0; i < 4; ++i)
		{
			int32 VertexIndex = FaceDef.Indices[i];
			if (VertexIndex == -1)
				break;
			FIntVector V = ConvertVertexIndexToGridPos(VertexIndex);
			Center += V;
			NumPointsForCenter++;
		}
		if (NumPointsForCenter > 0)
		{
			Center.X /= NumPointsForCenter;
			Center.Y /= NumPointsForCenter;
			Center.Z /= NumPointsForCenter;
		}

		if (FaceDef.Count == 3)
		{
			FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
			FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
			FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);

			OutTriangles.Add(SortTriangleVertices(V0, Center, V1));
			OutTriangles.Add(SortTriangleVertices(V0, V2, Center));
		}
		else if (FaceDef.Count == 4)
		{
			FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
			FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
			FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
			FIntVector V3 = ConvertVertexIndexToGridPos(FaceDef.Indices[3]);

			OutTriangles.Add(SortTriangleVertices(V0, Center, V1));
			OutTriangles.Add(SortTriangleVertices(V1, Center, V2));
			OutTriangles.Add(SortTriangleVertices(V2, Center, V3));
			OutTriangles.Add(SortTriangleVertices(V3, Center, V0));
		}
		break;
	}
	}
}

void AVoxelTile::GetNeighborFaceTriangles(int32 X, int32 Y, int32 Z, int32 FaceIndex, uint8 BlockType, int32 DirectionIndex, TArray<FIntTriangle>& OutTriangles) const
{
	// 获取相邻体素的位置和对应面
	// 获取对面索引
	int32 NeighborFaceIndex = GetNeighborFaceInfo(FaceIndex);
	if (NeighborFaceIndex == -1)
		return;

	const FIntVector& FaceDir = FaceDirections[FaceIndex];
	int32 NeighborX = X + FaceDir.X;
	int32 NeighborY = Y + FaceDir.Y;
	int32 NeighborZ = Z + FaceDir.Z;

	// 检查相邻体素是否在有效范围内
	if (!IsValidVoxelCoord(NeighborX, NeighborY, NeighborZ))
		return;

	// 获取相邻体素
	UCVoxelData NeighborVoxel = GetVoxel(NeighborX, NeighborY, NeighborZ);
	if (NeighborVoxel.LayerID == UCVoxelData_Layer_Null)
		return;

	// 获取相邻体素的类型和方向索引
	uint8 NeighborBlockType = NeighborVoxel.Type & 0x03;
	uint8 NeighborDirectionIndex = 0;
	if (NeighborBlockType == UCVoxelBlockType_SquareSlope)
	{
		NeighborDirectionIndex = UCVoxelData_GetSlopeDirectionIndex(NeighborVoxel);
		if (NeighborDirectionIndex >= 12)
			NeighborDirectionIndex = 0;
	}
	else if (NeighborBlockType == UCVoxelBlockType_TriangularSlope || NeighborBlockType == UCVoxelBlockType_TriangularComplement)
	{
		NeighborDirectionIndex = UCVoxelData_GetTriSlopeDirectionIndex(NeighborVoxel);
		if (NeighborDirectionIndex >= 8)
			NeighborDirectionIndex = 0;
	}

	// 获取相邻面的三角形
	bool bNeighborFlat = (NeighborBlockType == UCVoxelBlockType_Cube) ? IsFaceFlat(NeighborX, NeighborY, NeighborZ, NeighborFaceIndex) : false;
	GetFaceTriangles(NeighborX, NeighborY, NeighborZ, NeighborFaceIndex, NeighborBlockType, NeighborDirectionIndex, NeighborVoxel, bNeighborFlat, OutTriangles);
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
					// 根据图素类型处理不同的面
					switch (BlockType)
					{
					case UCVoxelBlockType_Cube:
					{
						// 添加方块的面
						// 检查面是否存在（Count是否为0）
						const FaceIndices& FaceDef = BoxFaces[FaceIndex];
						if (FaceDef.Count == 0)
							continue;

						// 获取面的三角形数组
						TArray<FIntTriangle> FaceTriangles;
						bool bFlat = IsFaceFlat(X, Y, Z, FaceIndex);
						GetFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, Voxel, bFlat, FaceTriangles);

						if (FaceTriangles.Num() == 0)
							continue;

						// 获取相邻面的三角形数组
						TArray<FIntTriangle> FaceNeighborTriangles;
						GetNeighborFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, FaceNeighborTriangles);

						// 从当前面的三角形中删除相邻面已有的三角形
						TArray<FIntTriangle> TrianglesToRender;
						for (const FIntTriangle& Triangle : FaceTriangles)
						{
							bool bFound = false;
							for (const FIntTriangle& NeighborTriangle : FaceNeighborTriangles)
							{
								if (Triangle == NeighborTriangle)
								{
									bFound = true;
									break;
								}
							}
							if (!bFound)
							{
								TrianglesToRender.Add(Triangle);
							}
						}

						// 如果没有需要渲染的三角形，跳过
						if (TrianglesToRender.Num() == 0)
							continue;

						// 渲染该面（只渲染剩余的三角形）
						AddFaceRender(X, Y, Z, FaceIndex, Voxel, bFlat, TrianglesToRender);
						break;
					}
					case UCVoxelBlockType_SquareSlope:
					{
						// 获取面的三角形数组
						TArray<FIntTriangle> FaceTriangles;
						GetFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, Voxel, false, FaceTriangles);

						if (FaceTriangles.Num() == 0)
							continue;

						// 获取相邻面的三角形数组
						TArray<FIntTriangle> FaceNeighborTriangles;
						GetNeighborFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, FaceNeighborTriangles);

						// 从当前面的三角形中删除相邻面已有的三角形
						TArray<FIntTriangle> TrianglesToRender;
						for (const FIntTriangle& Triangle : FaceTriangles)
						{
							bool bFound = false;
							for (const FIntTriangle& NeighborTriangle : FaceNeighborTriangles)
							{
								if (Triangle == NeighborTriangle)
								{
									bFound = true;
									break;
								}
							}
							if (!bFound)
							{
								TrianglesToRender.Add(Triangle);
							}
						}

						// 如果没有需要渲染的三角形，跳过
						if (TrianglesToRender.Num() == 0)
							continue;

						// 渲染该面（只渲染剩余的三角形）
						AddSquareSlopeFaceSingleRender(X, Y, Z, FaceIndex, Voxel, DirectionIndex, TrianglesToRender);
						break;
					}
					case UCVoxelBlockType_TriangularSlope:
					{
						// 获取面的三角形数组
						TArray<FIntTriangle> FaceTriangles;
						GetFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, Voxel, false, FaceTriangles);

						if (FaceTriangles.Num() == 0)
							continue;

						// 获取相邻面的三角形数组
						TArray<FIntTriangle> FaceNeighborTriangles;
						GetNeighborFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, FaceNeighborTriangles);

						// 从当前面的三角形中删除相邻面已有的三角形
						TArray<FIntTriangle> TrianglesToRender;
						for (const FIntTriangle& Triangle : FaceTriangles)
						{
							bool bFound = false;
							for (const FIntTriangle& NeighborTriangle : FaceNeighborTriangles)
							{
								if (Triangle == NeighborTriangle)
								{
									bFound = true;
									break;
								}
							}
							if (!bFound)
							{
								TrianglesToRender.Add(Triangle);
							}
						}

						// 如果没有需要渲染的三角形，跳过
						if (TrianglesToRender.Num() == 0)
							continue;

						// 渲染该面（只渲染剩余的三角形）
						AddTriangularSlopeFaceSingleRender(X, Y, Z, FaceIndex, Voxel, DirectionIndex, TrianglesToRender);
						break;
					}
					case UCVoxelBlockType_TriangularComplement:
					{
						// 获取面的三角形数组
						TArray<FIntTriangle> FaceTriangles;
						GetFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, Voxel, false, FaceTriangles);

						if (FaceTriangles.Num() == 0)
							continue;

						// 获取相邻面的三角形数组
						TArray<FIntTriangle> FaceNeighborTriangles;
						GetNeighborFaceTriangles(X, Y, Z, FaceIndex, BlockType, DirectionIndex, FaceNeighborTriangles);

						// 从当前面的三角形中删除相邻面已有的三角形
						TArray<FIntTriangle> TrianglesToRender;
						for (const FIntTriangle& Triangle : FaceTriangles)
						{
							bool bFound = false;
							for (const FIntTriangle& NeighborTriangle : FaceNeighborTriangles)
							{
								if (Triangle == NeighborTriangle)
								{
									bFound = true;
									break;
								}
							}
							if (!bFound)
							{
								TrianglesToRender.Add(Triangle);
							}
						}

						// 如果没有需要渲染的三角形，跳过
						if (TrianglesToRender.Num() == 0)
							continue;

						// 渲染该面（只渲染剩余的三角形）
						AddTriangularComplementFaceSingleRender(X, Y, Z, FaceIndex, Voxel, DirectionIndex, TrianglesToRender);
						break;
					}
					}
				}
			}
		}
	}
}

void AVoxelTile::AddFaceRender(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, bool bFlat, const TArray<FIntTriangle>& TrianglesToRender)
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

	// 如果指定了要渲染的三角形，先获取面的所有三角形，然后过滤
	TArray<FIntTriangle> AllFaceTriangles;
	if (TrianglesToRender.Num() > 0)
	{
		GetFaceTriangles(X, Y, Z, FaceIndex, UCVoxelBlockType_Cube, 0, Voxel, bFlat, AllFaceTriangles);
	}

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

			// UV坐标（从缓存读取）
			FVector2D UV = BaseCubeUVs[VertexIndex][FaceIndex];
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

		// 将局部顶点索引转换为网格坐标的辅助函数
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

		// 定义两个三角形的顶点坐标
		FIntTriangle Tri1(
			ConvertVertexIndexToGridPos(FaceDef.Indices[0]),
			ConvertVertexIndexToGridPos(FaceDef.Indices[2]),
			ConvertVertexIndexToGridPos(FaceDef.Indices[1])
		);
		Tri1 = SortTriangleVertices(Tri1.V0, Tri1.V1, Tri1.V2);

		FIntTriangle Tri2(
			ConvertVertexIndexToGridPos(FaceDef.Indices[0]),
			ConvertVertexIndexToGridPos(FaceDef.Indices[3]),
			ConvertVertexIndexToGridPos(FaceDef.Indices[2])
		);
		Tri2 = SortTriangleVertices(Tri2.V0, Tri2.V1, Tri2.V2);

		// 如果指定了要渲染的三角形，只渲染匹配的
		if (TrianglesToRender.Num() > 0)
		{
			bool bRenderTri1 = false;
			bool bRenderTri2 = false;

			for (const FIntTriangle& TriangleToRender : TrianglesToRender)
			{
				if (TriangleToRender == Tri1)
					bRenderTri1 = true;
				if (TriangleToRender == Tri2)
					bRenderTri2 = true;
			}

			// 第一个三角形
			if (bRenderTri1)
			{
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(BaseIndex + 2);
				Triangles.Add(BaseIndex + 1);
			}

			// 第二个三角形
			if (bRenderTri2)
			{
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(BaseIndex + 3);
				Triangles.Add(BaseIndex + 2);
			}
		}
		else
		{
			// 没有指定，渲染所有三角形
			// 第一个三角形
			Triangles.Add(BaseIndex + 0);
			Triangles.Add(BaseIndex + 2);
			Triangles.Add(BaseIndex + 1);

			// 第二个三角形
			Triangles.Add(BaseIndex + 0);
			Triangles.Add(BaseIndex + 3);
			Triangles.Add(BaseIndex + 2);
		}
	}
	else
	{
		// AO模式：5顶点（4个角+1个中心）+ 4个三角形

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

				// UV坐标（从缓存读取）
				UV = BaseCubeUVs[VertexIndex][FaceIndex];
			}
			else
			{
				// 中心顶点
				VertexPos = CenterPos;

				// UV坐标（中心点，所有面都是0.5, 0.5）
				UV = FVector2D(0.5f, 0.5f);
			}

			Vertices.Add(VertexPos);
			Normals.Add(Normal);
			UVs.Add(UV);

			// 顶点颜色（AO）
			FColor Color = FColor::White;
			// 			if (i < 4)
			// 			{
			// 				// 角顶点：检查是否有相邻体素，如果有则添加阴影
			// 				FIntVector CornerOffset;
			// 				if (i == 0) // 左下
			// 					CornerOffset = FIntVector(-1, -1, 0);
			// 				else if (i == 1) // 右下
			// 					CornerOffset = FIntVector(1, -1, 0);
			// 				else if (i == 2) // 右上
			// 					CornerOffset = FIntVector(1, 1, 0);
			// 				else // 左上
			// 					CornerOffset = FIntVector(-1, 1, 0);
			// 				
			// 				// 调整CornerOffset到正确的坐标系
			// 				if (FaceIndex < 4) // 侧面
			// 				{
			// 					if (FaceIndex == 0) // Left: CornerOffset应该是(-1,0,-1), (1,0,-1), (1,0,1), (-1,0,1)在XZ平面
			// 					{
			// 						if (i == 0) CornerOffset = FIntVector(-1, 0, -1);
			// 						else if (i == 1) CornerOffset = FIntVector(1, 0, -1);
			// 						else if (i == 2) CornerOffset = FIntVector(1, 0, 1);
			// 						else CornerOffset = FIntVector(-1, 0, 1);
			// 					}
			// 					else if (FaceIndex == 1) // Front
			// 					{
			// 						if (i == 0) CornerOffset = FIntVector(0, -1, -1);
			// 						else if (i == 1) CornerOffset = FIntVector(0, 1, -1);
			// 						else if (i == 2) CornerOffset = FIntVector(0, 1, 1);
			// 						else CornerOffset = FIntVector(0, -1, 1);
			// 					}
			// 					else if (FaceIndex == 2) // Right
			// 					{
			// 						if (i == 0) CornerOffset = FIntVector(1, 0, -1);
			// 						else if (i == 1) CornerOffset = FIntVector(-1, 0, -1);
			// 						else if (i == 2) CornerOffset = FIntVector(-1, 0, 1);
			// 						else CornerOffset = FIntVector(1, 0, 1);
			// 					}
			// 					else // Back
			// 					{
			// 						if (i == 0) CornerOffset = FIntVector(0, 1, -1);
			// 						else if (i == 1) CornerOffset = FIntVector(0, -1, -1);
			// 						else if (i == 2) CornerOffset = FIntVector(0, -1, 1);
			// 						else CornerOffset = FIntVector(0, 1, 1);
			// 					}
			// 				}
			// 				else if (FaceIndex == 4) // Top
			// 				{
			// 					if (i == 0) CornerOffset = FIntVector(-1, -1, 1);
			// 					else if (i == 1) CornerOffset = FIntVector(1, -1, 1);
			// 					else if (i == 2) CornerOffset = FIntVector(1, 1, 1);
			// 					else CornerOffset = FIntVector(-1, 1, 1);
			// 				}
			// 				else // Bottom
			// 				{
			// 					if (i == 0) CornerOffset = FIntVector(-1, 1, -1);
			// 					else if (i == 1) CornerOffset = FIntVector(1, 1, -1);
			// 					else if (i == 2) CornerOffset = FIntVector(1, -1, -1);
			// 					else CornerOffset = FIntVector(-1, -1, -1);
			// 				}
			// 				
			// 				// 检查对角相邻体素是否存在
			// 				int32 DiagX = X + Direction.X + CornerOffset.X;
			// 				int32 DiagY = Y + Direction.Y + CornerOffset.Y;
			// 				int32 DiagZ = Z + Direction.Z + CornerOffset.Z;
			// 				
			// 				// 如果对角相邻体素存在，添加阴影（降低亮度）
			// 				if (!IsVoxelEmpty(DiagX, DiagY, DiagZ))
			// 				{
			// 					// 角落有阴影：使用较暗的颜色（约0.6-0.7的亮度）
			// 					uint8 AOValue = 160; // 约0.63的亮度
			// 					Color = FColor(AOValue, AOValue, AOValue, 255);
			// 				}
			// 				else
			// 				{
			// 					Color = FColor::White;
			// 				}
			// 				
			// 				if (Voxel.TextureID > 0)
			// 				{
			// 					uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
			// 					Color = FColor(
			// 						FMath::Min((uint32)Color.R * Gray / 255, 255u),
			// 						FMath::Min((uint32)Color.G * Gray / 255, 255u),
			// 						FMath::Min((uint32)Color.B * Gray / 255, 255u),
			// 						255
			// 					);
			// 				}
			// 			}
			// 			else
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

void AVoxelTile::AddSquareSlopeFaceSingleRender(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, int32 DirectionIndex, const TArray<FIntTriangle>& TrianglesToRender)
{
	if (FaceIndex < 0 || FaceIndex >= 7)
		return;

	// 计算体素在世界空间中的位置
	FVector BasePos = FVector(X * VoxelSize - (VOXEL_TILE_SIZE_X / 2) * VoxelSize, Y * VoxelSize - (VOXEL_TILE_SIZE_Y / 2) * VoxelSize, Z * VoxelSize - (VOXEL_TILE_SIZE_Z / 2) * VoxelSize);

	int32 BaseIndex = Vertices.Num();

	FVoxelBlockShapeGenerator* BlockShapeGenerator = GetBlockShapeGenerator();
	int32 RelativeFaceIndex = FaceIndex > 5 ? FaceIndex : BlockShapeGenerator->SlopeFaceDirectionsReverse[DirectionIndex][FaceIndex];
	const FaceIndices& FaceDef = BlockShapeGenerator->SlopeFaces[DirectionIndex][RelativeFaceIndex];

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

	// 计算中心点（使用四个点）
	// 不管NumVerts是多少，都尝试使用FaceDef.Indices的前4个索引
	FVector Center(0, 0, 0);
	// 中心点的UV坐标（使用四个点的平均UV）
	FVector2D CenterUV(0, 0);
	for (int32 i = 0; i < 4; ++i)
	{
		int32 VertexIndex = FaceDef.Indices[i];
		// 使用BaseCubeVertices计算顶点位置
		FVector LocalVertex = BaseCubeVertices[VertexIndex];
		FVector VertexPos = BasePos + LocalVertex * VoxelSize;
		Center += VertexPos;
		CenterUV += BaseCubeUVs[VertexIndex][FaceIndex];
	}
	Center = Center / 4.0f;
	CenterUV = CenterUV / 4.0f;

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

	// 添加原始顶点
	for (int32 i = 0; i < NumVerts; ++i)
	{
		Vertices.Add(Verts[i]);
		Normals.Add(Normal);

		// UV坐标（从缓存读取）
		int32 VertexIndex = FaceDef.Indices[i];
		FVector2D UV = BaseCubeUVs[VertexIndex][FaceIndex];
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

	// 添加中心点
	int32 CenterIndex = BaseIndex + NumVerts;
	Vertices.Add(Center);
	Normals.Add(Normal);
	UVs.Add(CenterUV);

	FColor CenterColor = FColor::White;
	if (Voxel.TextureID > 0)
	{
		uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
		CenterColor = FColor(Gray, Gray, Gray, 255);
	}
	VertexColors.Add(CenterColor);
	Tangents.Add(FProcMeshTangent(0, 0, 1));

	// 将局部顶点索引转换为网格坐标的辅助函数
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

	// 计算中心点的网格坐标
	FIntVector CenterGrid(0, 0, 0);
	int32 NumPointsForCenter = 0;
	for (int32 i = 0; i < 4; ++i)
	{
		int32 VertexIndex = FaceDef.Indices[i];
		if (VertexIndex == -1)
			break;
		FIntVector V = ConvertVertexIndexToGridPos(VertexIndex);
		CenterGrid += V;
		NumPointsForCenter++;
	}
	if (NumPointsForCenter > 0)
	{
		CenterGrid.X /= NumPointsForCenter;
		CenterGrid.Y /= NumPointsForCenter;
		CenterGrid.Z /= NumPointsForCenter;
	}

	// 添加三角形（使用中心点拆分）
	if (NumVerts == 3)
	{
		// 三角形拆解成2个三角形
		FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
		FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
		FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);

		FIntTriangle Tri1 = SortTriangleVertices(V0, CenterGrid, V1);
		FIntTriangle Tri2 = SortTriangleVertices(V0, V2, CenterGrid);

		// 如果指定了要渲染的三角形，只渲染匹配的
		if (TrianglesToRender.Num() > 0)
		{
			bool bRenderTri1 = false;
			bool bRenderTri2 = false;

			for (const FIntTriangle& TriangleToRender : TrianglesToRender)
			{
				if (TriangleToRender == Tri1)
					bRenderTri1 = true;
				if (TriangleToRender == Tri2)
					bRenderTri2 = true;
			}

			if (bRenderTri1)
			{
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 1);
			}

			if (bRenderTri2)
			{
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(BaseIndex + 2);
				Triangles.Add(CenterIndex);
			}
		}
		else
		{
			// 没有指定，渲染所有三角形
			Triangles.Add(BaseIndex + 0);
			Triangles.Add(CenterIndex);
			Triangles.Add(BaseIndex + 1);

			Triangles.Add(BaseIndex + 0);
			Triangles.Add(BaseIndex + 2);
			Triangles.Add(CenterIndex);
		}
	}
	else
	{
		// 四边形拆解成4个三角形
		FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
		FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
		FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
		FIntVector V3 = ConvertVertexIndexToGridPos(FaceDef.Indices[3]);

		FIntTriangle Tri1 = SortTriangleVertices(V0, CenterGrid, V1);
		FIntTriangle Tri2 = SortTriangleVertices(V1, CenterGrid, V2);
		FIntTriangle Tri3 = SortTriangleVertices(V2, CenterGrid, V3);
		FIntTriangle Tri4 = SortTriangleVertices(V3, CenterGrid, V0);

		// 如果指定了要渲染的三角形，只渲染匹配的
		if (TrianglesToRender.Num() > 0)
		{
			bool bRenderTri1 = false, bRenderTri2 = false, bRenderTri3 = false, bRenderTri4 = false;

			for (const FIntTriangle& TriangleToRender : TrianglesToRender)
			{
				if (TriangleToRender == Tri1) bRenderTri1 = true;
				if (TriangleToRender == Tri2) bRenderTri2 = true;
				if (TriangleToRender == Tri3) bRenderTri3 = true;
				if (TriangleToRender == Tri4) bRenderTri4 = true;
			}

			if (bRenderTri1)
			{
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 1);
			}

			if (bRenderTri2)
			{
				Triangles.Add(BaseIndex + 1);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 2);
			}

			if (bRenderTri3)
			{
				Triangles.Add(BaseIndex + 2);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 3);
			}

			if (bRenderTri4)
			{
				Triangles.Add(BaseIndex + 3);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 0);
			}
		}
		else
		{
			// 没有指定，渲染所有三角形
			Triangles.Add(BaseIndex + 0);
			Triangles.Add(CenterIndex);
			Triangles.Add(BaseIndex + 1);

			Triangles.Add(BaseIndex + 1);
			Triangles.Add(CenterIndex);
			Triangles.Add(BaseIndex + 2);

			Triangles.Add(BaseIndex + 2);
			Triangles.Add(CenterIndex);
			Triangles.Add(BaseIndex + 3);

			Triangles.Add(BaseIndex + 3);
			Triangles.Add(CenterIndex);
			Triangles.Add(BaseIndex + 0);
		}
	}
}

void AVoxelTile::AddTriangularSlopeFaceSingleRender(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, int32 DirectionIndex, const TArray<FIntTriangle>& TrianglesToRender)
{
	if (FaceIndex < 0 || FaceIndex >= 7)
		return;

	// 计算体素在世界空间中的位置
	FVector BasePos = FVector(X * VoxelSize - (VOXEL_TILE_SIZE_X / 2) * VoxelSize, Y * VoxelSize - (VOXEL_TILE_SIZE_Y / 2) * VoxelSize, Z * VoxelSize - (VOXEL_TILE_SIZE_Z / 2) * VoxelSize);

	int32 BaseIndex = Vertices.Num();

	FVoxelBlockShapeGenerator* BlockShapeGenerator = GetBlockShapeGenerator();
	int32 RelativeFaceIndex = FaceIndex > 5 ? FaceIndex : BlockShapeGenerator->TriSlopeFaceDirectionsReverse[DirectionIndex][FaceIndex];
	// 确保RelativeFaceIndex在有效范围内
	if (RelativeFaceIndex < 0 || RelativeFaceIndex >= 7)
		return;

	const FaceIndices& FaceDef = BlockShapeGenerator->TriSlopeFaces[DirectionIndex][RelativeFaceIndex];

	int32 NumVerts = FaceDef.Count;
	// 如果面不存在（Count为0），直接返回
	if (NumVerts == 0)
		return;

	// 确保NumVerts不超过3（三角锥的面最多3个顶点）
	if (NumVerts > 3)
		NumVerts = 3;

	// 计算顶点位置
	FVector Verts[3];
	for (int32 i = 0; i < NumVerts; ++i)
	{
		int32 VertexIndex = FaceDef.Indices[i];
		if (VertexIndex < 0 || VertexIndex >= 8)
			return; // 无效的顶点索引
		FVector LocalVertex = BaseCubeVertices[VertexIndex];
		Verts[i] = BasePos + LocalVertex * VoxelSize;
	}

	// 计算中心点（使用四个点）
	// 不管NumVerts是多少，都尝试使用FaceDef.Indices的前4个索引
	FVector Center(0, 0, 0);
	FVector2D CenterUV(0, 0);

	if (FaceIndex < 6)
	{
		int32 NumPointsForCenter = 0;
		for (int32 i = 0; i < 4; ++i)
		{
			int32 VertexIndex = FaceDef.Indices[i];
			if (VertexIndex < 0 || VertexIndex >= 8)
				break; // 遇到无效索引，停止
			// 使用BaseCubeVertices计算顶点位置
			FVector LocalVertex = BaseCubeVertices[VertexIndex];
			FVector VertexPos = BasePos + LocalVertex * VoxelSize;
			Center += VertexPos;
			CenterUV += BaseCubeUVs[VertexIndex][FaceIndex];
			NumPointsForCenter++;
		}
		if (NumPointsForCenter > 0)
		{
			Center = Center / (float)NumPointsForCenter;
			CenterUV = CenterUV / (float)NumPointsForCenter;
		}
	}

	// 计算三角形法向量
	FVector Edge1 = Verts[1] - Verts[0];
	FVector Edge2 = Verts[2] - Verts[0];
	FVector TriNormal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

	// 添加3个原始顶点
	for (int32 i = 0; i < NumVerts; ++i)
	{
		Vertices.Add(Verts[i]);
		Normals.Add(TriNormal);

		// UV坐标（从缓存读取）
		int32 VertexIndex = FaceDef.Indices[i];
		if (VertexIndex < 0 || VertexIndex >= 8 || FaceIndex < 0 || FaceIndex > 6)
			return; // 无效的索引
		FVector2D UV = BaseCubeUVs[VertexIndex][FaceIndex];
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

	// 添加中心点
	int32 CenterIndex = BaseIndex + NumVerts;
	if (FaceIndex < 6)
	{
		Vertices.Add(Center);
		Normals.Add(TriNormal);
		UVs.Add(CenterUV);

		FColor CenterColor = FColor::White;
		if (Voxel.TextureID > 0)
		{
			uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
			CenterColor = FColor(Gray, Gray, Gray, 255);
		}
		VertexColors.Add(CenterColor);
		Tangents.Add(FProcMeshTangent(0, 0, 1));

		// 将局部顶点索引转换为网格坐标的辅助函数
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

		// 计算中心点的网格坐标
		FIntVector CenterGrid(0, 0, 0);
		int32 NumPointsForCenterGrid = 0;
		for (int32 i = 0; i < 4; ++i)
		{
			int32 VertexIndex = FaceDef.Indices[i];
			if (VertexIndex < 0 || VertexIndex >= 8)
				break; // 遇到无效索引，停止
			FIntVector V = ConvertVertexIndexToGridPos(VertexIndex);
			CenterGrid += V;
			NumPointsForCenterGrid++;
		}
		if (NumPointsForCenterGrid > 0)
		{
			CenterGrid.X /= NumPointsForCenterGrid;
			CenterGrid.Y /= NumPointsForCenterGrid;
			CenterGrid.Z /= NumPointsForCenterGrid;
		}

		// 确保索引有效
		if (FaceDef.Indices[0] < 0 || FaceDef.Indices[0] >= 8 ||
			FaceDef.Indices[1] < 0 || FaceDef.Indices[1] >= 8 ||
			FaceDef.Indices[2] < 0 || FaceDef.Indices[2] >= 8)
			return; // 无效的顶点索引

		FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
		FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
		FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);

		FIntTriangle Tri1 = SortTriangleVertices(V0, CenterGrid, V1);
		FIntTriangle Tri2 = SortTriangleVertices(V0, V2, CenterGrid);

		// 如果指定了要渲染的三角形，只渲染匹配的
		if (TrianglesToRender.Num() > 0)
		{
			bool bRenderTri1 = false;
			bool bRenderTri2 = false;

			for (const FIntTriangle& TriangleToRender : TrianglesToRender)
			{
				if (TriangleToRender == Tri1)
					bRenderTri1 = true;
				if (TriangleToRender == Tri2)
					bRenderTri2 = true;
			}

			if (bRenderTri1)
			{
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 1);
			}

			if (bRenderTri2)
			{
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(BaseIndex + 2);
				Triangles.Add(CenterIndex);
			}
		}
		else
		{
			// 没有指定，渲染所有三角形
			Triangles.Add(BaseIndex + 0);
			Triangles.Add(BaseIndex + 2);
			Triangles.Add(BaseIndex + 1);
		}
	}
	else
	{
		// FaceIndex == 6，单个三角形
		// 将局部顶点索引转换为网格坐标的辅助函数
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

		FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
		FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
		FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
		FIntTriangle Tri = SortTriangleVertices(V0, V2, V1);

		// 如果指定了要渲染的三角形，只渲染匹配的
		if (TrianglesToRender.Num() > 0)
		{
			bool bRender = false;
			for (const FIntTriangle& TriangleToRender : TrianglesToRender)
			{
				if (TriangleToRender == Tri)
				{
					bRender = true;
					break;
				}
			}

			if (bRender)
			{
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(BaseIndex + 2);
				Triangles.Add(BaseIndex + 1);
			}
		}
		else
		{
			// 没有指定，渲染所有三角形
			Triangles.Add(BaseIndex + 0);
			Triangles.Add(BaseIndex + 2);
			Triangles.Add(BaseIndex + 1);
		}
	}
}

void AVoxelTile::AddTriangularComplementFaceSingleRender(int32 X, int32 Y, int32 Z, int32 FaceIndex, const UCVoxelData& Voxel, int32 DirectionIndex, const TArray<FIntTriangle>& TrianglesToRender)
{
	if (FaceIndex < 0 || FaceIndex >= 7)
		return;

	// 计算体素在世界空间中的位置
	FVector BasePos = FVector(X * VoxelSize - (VOXEL_TILE_SIZE_X / 2) * VoxelSize, Y * VoxelSize - (VOXEL_TILE_SIZE_Y / 2) * VoxelSize, Z * VoxelSize - (VOXEL_TILE_SIZE_Z / 2) * VoxelSize);

	int32 BaseIndex = Vertices.Num();

	FVoxelBlockShapeGenerator* BlockShapeGenerator = GetBlockShapeGenerator();
	int32 RelativeFaceIndex = FaceIndex > 5 ? FaceIndex : BlockShapeGenerator->TriSlopeComplementFaceDirectionsReverse[DirectionIndex][FaceIndex];
	const FaceIndices& FaceDef = BlockShapeGenerator->TriSlopeComplementFaces[DirectionIndex][RelativeFaceIndex];

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

	// 计算中心点（使用四个点）
	// 不管NumVerts是多少，都尝试使用FaceDef.Indices的前4个索引
	FVector Center(0, 0, 0);
	int32 NumPointsForCenter = 4;
	for (int32 i = 0; i < 4; ++i)
	{
		int32 VertexIndex = FaceDef.Indices[i];
		if (VertexIndex == -1)
		{
			NumPointsForCenter--;
			continue; // 跳过-1索引
		}
		// 使用BaseCubeVertices计算顶点位置
		FVector LocalVertex = BaseCubeVertices[VertexIndex];
		FVector VertexPos = BasePos + LocalVertex * VoxelSize;
		Center += VertexPos;
	}
	if (NumPointsForCenter > 0)
	{
		Center = Center / (float)NumPointsForCenter;
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

	// 添加原始顶点
	for (int32 i = 0; i < NumVerts; ++i)
	{
		Vertices.Add(Verts[i]);
		Normals.Add(Normal);

		// UV坐标（从缓存读取）
		int32 VertexIndex = FaceDef.Indices[i];
		FVector2D UV = BaseCubeUVs[VertexIndex][FaceIndex];
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

	// 添加中心点
	int32 CenterIndex = BaseIndex + NumVerts;
	if (FaceIndex < 6)
	{
		Vertices.Add(Center);
		Normals.Add(Normal);
		// 中心点的UV坐标（使用四个点的平均UV）
		FVector2D CenterUV(0, 0);
		int32 NumPointsForCenterUV = 4;
		for (int32 i = 0; i < 4; ++i)
		{
			int32 VertexIndex = FaceDef.Indices[i];
			if (VertexIndex == -1)
			{
				NumPointsForCenterUV--;
				continue; // 跳过-1索引
			}
			CenterUV += BaseCubeUVs[VertexIndex][FaceIndex];
		}
		if (NumPointsForCenterUV > 0)
		{
			CenterUV = CenterUV / (float)NumPointsForCenterUV;
		}
		UVs.Add(CenterUV);

		FColor CenterColor = FColor::White;
		if (Voxel.TextureID > 0)
		{
			uint8 Gray = FMath::Clamp(Voxel.TextureID * 255 / 255, (uint8)64, (uint8)255);
			CenterColor = FColor(Gray, Gray, Gray, 255);
		}
		VertexColors.Add(CenterColor);
		Tangents.Add(FProcMeshTangent(0, 0, 1));

		// 将局部顶点索引转换为网格坐标的辅助函数
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

		// 计算中心点的网格坐标
		FIntVector CenterGrid(0, 0, 0);
		int32 NumPointsForCenterGrid = 0;
		for (int32 i = 0; i < 4; ++i)
		{
			int32 VertexIndex = FaceDef.Indices[i];
			if (VertexIndex == -1)
				break;
			FIntVector V = ConvertVertexIndexToGridPos(VertexIndex);
			CenterGrid += V;
			NumPointsForCenterGrid++;
		}
		if (NumPointsForCenterGrid > 0)
		{
			CenterGrid.X /= NumPointsForCenterGrid;
			CenterGrid.Y /= NumPointsForCenterGrid;
			CenterGrid.Z /= NumPointsForCenterGrid;
		}

		// 添加三角形（使用中心点拆分）
		if (NumVerts == 3)
		{
			FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
			FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
			FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
			if (FaceIndex < 6)
			{
				FIntTriangle Tri1 = SortTriangleVertices(V0, CenterGrid, V1);
				FIntTriangle Tri2 = SortTriangleVertices(V0, V2, CenterGrid);

				// 如果指定了要渲染的三角形，只渲染匹配的
				if (TrianglesToRender.Num() > 0)
				{
					bool bRenderTri1 = false;
					bool bRenderTri2 = false;

					for (const FIntTriangle& TriangleToRender : TrianglesToRender)
					{
						if (TriangleToRender == Tri1)
							bRenderTri1 = true;
						if (TriangleToRender == Tri2)
							bRenderTri2 = true;
					}

					if (bRenderTri1)
					{
						Triangles.Add(BaseIndex + 0);
						Triangles.Add(CenterIndex);
						Triangles.Add(BaseIndex + 1);
					}

					if (bRenderTri2)
					{
						Triangles.Add(BaseIndex + 0);
						Triangles.Add(BaseIndex + 2);
						Triangles.Add(CenterIndex);
					}
				}
				else
				{
					// 没有指定，渲染所有三角形
					Triangles.Add(BaseIndex + 0);
					Triangles.Add(CenterIndex);
					Triangles.Add(BaseIndex + 1);

					Triangles.Add(BaseIndex + 0);
					Triangles.Add(BaseIndex + 2);
					Triangles.Add(CenterIndex);
				}
			}
		}
		else
		{
			// 四边形拆解成4个三角形
			FIntVector V0 = ConvertVertexIndexToGridPos(FaceDef.Indices[0]);
			FIntVector V1 = ConvertVertexIndexToGridPos(FaceDef.Indices[1]);
			FIntVector V2 = ConvertVertexIndexToGridPos(FaceDef.Indices[2]);
			FIntVector V3 = ConvertVertexIndexToGridPos(FaceDef.Indices[3]);

			FIntTriangle Tri1 = SortTriangleVertices(V0, CenterGrid, V1);
			FIntTriangle Tri2 = SortTriangleVertices(V1, CenterGrid, V2);
			FIntTriangle Tri3 = SortTriangleVertices(V2, CenterGrid, V3);
			FIntTriangle Tri4 = SortTriangleVertices(V3, CenterGrid, V0);

			// 如果指定了要渲染的三角形，只渲染匹配的
			if (TrianglesToRender.Num() > 0)
			{
				bool bRenderTri1 = false, bRenderTri2 = false, bRenderTri3 = false, bRenderTri4 = false;

				for (const FIntTriangle& TriangleToRender : TrianglesToRender)
				{
					if (TriangleToRender == Tri1) bRenderTri1 = true;
					if (TriangleToRender == Tri2) bRenderTri2 = true;
					if (TriangleToRender == Tri3) bRenderTri3 = true;
					if (TriangleToRender == Tri4) bRenderTri4 = true;
				}

				if (bRenderTri1)
				{
					Triangles.Add(BaseIndex + 0);
					Triangles.Add(CenterIndex);
					Triangles.Add(BaseIndex + 1);
				}

				if (bRenderTri2)
				{
					Triangles.Add(BaseIndex + 1);
					Triangles.Add(CenterIndex);
					Triangles.Add(BaseIndex + 2);
				}

				if (bRenderTri3)
				{
					Triangles.Add(BaseIndex + 2);
					Triangles.Add(CenterIndex);
					Triangles.Add(BaseIndex + 3);
				}

				if (bRenderTri4)
				{
					Triangles.Add(BaseIndex + 3);
					Triangles.Add(CenterIndex);
					Triangles.Add(BaseIndex + 0);
				}
			}
			else
			{
				// 没有指定，渲染所有三角形
				Triangles.Add(BaseIndex + 0);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 1);

				Triangles.Add(BaseIndex + 1);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 2);

				Triangles.Add(BaseIndex + 2);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 3);

				Triangles.Add(BaseIndex + 3);
				Triangles.Add(CenterIndex);
				Triangles.Add(BaseIndex + 0);
			}
		}
	}
	else
	{
		// 没有指定，渲染所有三角形
		Triangles.Add(BaseIndex + 0);
		Triangles.Add(BaseIndex + 2);
		Triangles.Add(BaseIndex + 1);
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

