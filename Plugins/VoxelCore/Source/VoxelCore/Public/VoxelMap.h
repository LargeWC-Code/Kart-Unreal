#pragma once

#include "CoreMinimal.h"  // For VOXELCORE_API (defined by build system)
#include "ucgamebase.h"  // For UCString, UCEArray, etc. from MagicXCore

// Tile尺寸宏定义（用于所有Tile相关计算）
#define VOXEL_TILE_SIZE_X 32
#define VOXEL_TILE_SIZE_Y 32
#define VOXEL_TILE_SIZE_Z 64

struct VOXELCORE_API UCVoxelPrefabData
{
	UCString		Name;
	ucINT			Type;
};

SCRIPT_DECLARE(VOXELCORE_API, UCE_UCVoxelPrefabData, UCVoxelPrefabData, ucTRUE);

enum UCVoxelData_Layer
{
	UCVoxelData_Layer_Null = 0,
	UCVoxelData_Layer_Ground,
	UCVoxelData_Layer_Water
};

// 砖块类型
enum UCVoxelBlockType
{
	UCVoxelBlockType_Cube = 0,              // 方块
	UCVoxelBlockType_SquareSlope = 1,       // 斜楔
	UCVoxelBlockType_TriangularSlope = 2,   // 三角锥
	UCVoxelBlockType_TriangularComplement = 3 // 三角锥互补体
};

// 面形状类型
enum UCVoxelFaceShape
{
	UCVoxelFaceShape_Square = 0,        // 正方面（正方形）
	UCVoxelFaceShape_Triangle = 1,      // 三角面（三角形）
	UCVoxelFaceShape_SlopedRectangle = 2, // 斜矩形面（矩形但有一个维度是斜的）
	UCVoxelFaceShape_SlopedTriangle = 3,  // 斜三角面（三角形但有一个维度是斜的）
	UCVoxelFaceShape_None = 4            // 不渲染（面被遮挡或不存在）
};

// Tile 数据：存储一个 Tile 的坐标和体素数据
struct VOXELCORE_API UCVoxelData
{
	union
	{
		struct
		{
			ucBYTE			TextureID;		// 8位：纹理ID（原来的Type）
			ucBYTE			LayerID;		// 8位：层
			ucBYTE			Type;			// 2位：砖块类型（0=方块, 1=斜面, 2=三角斜面, 3=三角斜面互补体）
			ucBYTE			YawRoll;
		};

		ucDWORD				Data;
	};

	UCVoxelData();
	UCVoxelData(ucCONST UCVoxelData&);
	~UCVoxelData();

	UCVoxelData& operator =(ucCONST UCVoxelData&);
};

// 辅助函数（用于位操作）
// YawRoll组合值：
// 对于Slope: YawRoll = Yaw * 3 + Roll（Yaw: 0-3四个方向，Roll: 0-2三个状态，总共12个方向）
// 对于TriSlope: YawRoll = Yaw * 2 + Roll（Yaw: 0-3四个方向，Roll: 0-1两个状态，总共8个方向）
// Yaw: 绕上下轴旋转（左右旋转），4个方向（0-3对应0°, 90°, 180°, 270°）
// Roll: 绕前后轴旋转（相对自身转），Slope有3个状态（0=朝上, 1=朝中间, 2=朝下），TriSlope有2个状态（0=朝上, 1=朝下）

// 获取Yaw (0-3) - 左右旋转，四个方向
inline ucBYTE UCVoxelData_GetYaw(ucCONST UCVoxelData& Voxel)
{
	// 对于Slope: Yaw = YawRoll / 3
	// 对于TriSlope: Yaw = YawRoll / 2
	// 由于YawRoll最大是11（Slope）或7（TriSlope），我们可以通过除以3或2来获取Yaw
	// 但为了通用，我们使用位操作：高2位存储Yaw，低2位存储Roll
	// 但这样YawRoll最大只能到15，对于Slope需要12个值，对于TriSlope需要8个值
	// 所以我们需要用不同的编码方式
	// 实际上，我们可以用4位来存储：高2位是Yaw，低2位是Roll（但Roll只有0-2或0-1）
	// 为了简化，我们使用：YawRoll = Yaw * 3 + Roll（Slope）或 YawRoll = Yaw * 2 + Roll（TriSlope）
	// 但这样需要知道BlockType才能正确解码
	// 暂时使用：高2位是Yaw，低2位是Roll（Roll的值会被限制在0-2或0-1）
	return (ucBYTE)((Voxel.YawRoll >> 2) & 0x03); // 高2位是Yaw
}

// 获取Roll (0-3，但实际Slope只用到0-2，TriSlope只用到0-1) - 相对自身旋转
inline ucBYTE UCVoxelData_GetRoll(ucCONST UCVoxelData& Voxel)
{
	return (ucBYTE)(Voxel.YawRoll & 0x03); // 低2位是Roll
}

// 设置Yaw和Roll
inline ucVOID UCVoxelData_SetYawAndRoll(UCVoxelData& Voxel, ucBYTE Yaw, ucBYTE Roll)
{
	Voxel.YawRoll = ((Yaw & 0x03) << 2) | (Roll & 0x03);
}

// 获取Slope的方向索引（0-11）：YawRoll = Yaw * 3 + Roll
inline ucBYTE UCVoxelData_GetSlopeDirectionIndex(ucCONST UCVoxelData& Voxel)
{
	uint8 Yaw = UCVoxelData_GetYaw(Voxel);
	uint8 Roll = UCVoxelData_GetRoll(Voxel);
	return (ucBYTE)(Yaw * 3 + Roll); // Yaw: 0-3, Roll: 0-2
}

// 获取TriSlope的方向索引（0-7）：YawRoll = Yaw * 2 + Roll
inline ucBYTE UCVoxelData_GetTriSlopeDirectionIndex(ucCONST UCVoxelData& Voxel)
{
	uint8 Yaw = UCVoxelData_GetYaw(Voxel);
	uint8 Roll = UCVoxelData_GetRoll(Voxel);
	return (ucBYTE)(Yaw * 2 + Roll); // Yaw: 0-3, Roll: 0-1
}

SCRIPT_DECLARE(VOXELCORE_API, UCE_UCVoxelData, UCVoxelData, ucTRUE);

// Tile 数据：存储一个 Tile 的坐标和体素数据
struct VOXELCORE_API UCVoxelTileData
{
	ucINT			TileX;			// Tile 坐标 X
	ucINT			TileY;			// Tile 坐标 Y

	UCIntArray		AryVoxels;

	UCVoxelTileData();
	UCVoxelTileData(ucCONST UCVoxelTileData&);
	~UCVoxelTileData();
	
	UCVoxelTileData& operator =(ucCONST UCVoxelTileData&);
};

SCRIPT_DECLARE(VOXELCORE_API, UCE_UCVoxelTileData, UCVoxelTileData, ucTRUE);

// 地图数据
struct VOXELCORE_API UCVoxelMapData
{
	UCSize			Size;

	union
	{
		UCEArray<UCVoxelTileData>		AryTiles;
		_UCEArray						_AryTiles;		// Tile 数据数组
	};

	UCVoxelMapData();
	UCVoxelMapData(ucCONST UCVoxelMapData&);
	~UCVoxelMapData();

	UCVoxelMapData& operator =(ucCONST UCVoxelMapData&);
};

SCRIPT_DECLARE(VOXELCORE_API, UCE_UCVoxelMapData, UCVoxelMapData, ucTRUE);

enum UCVoxelMapNodeData_Type
{
	UCVoxelMapNodeData_Folder = 0,
	UCVoxelMapNodeData_Map
};

struct VOXELCORE_API UCVoxelMapNodeData
{
	ucINT			Type;
	UCString		Name;
	union
	{
		UCEArray<UCVoxelMapNodeData>	AryNodes;
		_UCEArray						_AryNodes;
	};
	UCVoxelMapNodeData();
	UCVoxelMapNodeData(ucCONST UCVoxelMapNodeData&);
	~UCVoxelMapNodeData();

	UCVoxelMapNodeData& operator =(ucCONST UCVoxelMapNodeData&);
};

SCRIPT_DECLARE(VOXELCORE_API, UCE_UCVoxelMapNodeData, UCVoxelMapNodeData, ucTRUE);

class VOXELCORE_API UCVoxelMapManager
{
public:
	UCVoxelMapNodeData					Root;

	UCVoxelMapData*						Curr;
public:
	UCVoxelMapManager();
	~UCVoxelMapManager();
	ucBOOL	LoadFromFile(ucCONST UCString& Filename);
	ucVOID	SaveToFile(ucCONST UCString& Filename);

	ucVOID	NewCurrentMap(UCSize Size);

	ucBOOL	LoadMap(ucCONST UCString& Filename);
	ucVOID	SaveMap(ucCONST UCString& Filename);
};
