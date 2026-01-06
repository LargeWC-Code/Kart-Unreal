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
	UCVoxelBlockType_Cube = 0,           // 方块
	UCVoxelBlockType_SquareSlope = 1,    // 斜面
	UCVoxelBlockType_TriangularSlope = 2 // 三角斜面
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
			ucBYTE			Type;			// 2位：砖块类型（0=方块, 1=斜面, 2=三角斜面, 3=保留）
			ucBYTE			RotationXYZ;	// 6位：旋转（X、Y、Z各2位，每个轴0-3对应0°, 90°, 180°, 270°）
		};

		ucDWORD				Data;
	};

	UCVoxelData();
	UCVoxelData(ucCONST UCVoxelData&);
	~UCVoxelData();

	UCVoxelData& operator =(ucCONST UCVoxelData&);
};

// 辅助宏和函数（用于位操作）
// 旋转位域布局：RotationXYZ (6位)
// Bits 0-1: RotationX (0-3)
// Bits 2-3: RotationY (0-3)
// Bits 4-5: RotationZ (0-3)

// 获取旋转X (0-3)
inline ucBYTE UCVoxelData_GetRotationX(ucCONST UCVoxelData& Voxel)
{
	return (ucBYTE)(Voxel.RotationXYZ & 0x03);
}

// 获取旋转Y (0-3)
inline ucBYTE UCVoxelData_GetRotationY(ucCONST UCVoxelData& Voxel)
{
	return (ucBYTE)((Voxel.RotationXYZ >> 2) & 0x03);
}

// 获取旋转Z (0-3)
inline ucBYTE UCVoxelData_GetRotationZ(ucCONST UCVoxelData& Voxel)
{
	return (ucBYTE)((Voxel.RotationXYZ >> 4) & 0x03);
}

// 设置旋转（X、Y、Z各0-3）
inline ucVOID UCVoxelData_SetRotation(UCVoxelData& Voxel, ucBYTE RotationX, ucBYTE RotationY, ucBYTE RotationZ)
{
	Voxel.RotationXYZ = (RotationX & 0x03) | ((RotationY & 0x03) << 2) | ((RotationZ & 0x03) << 4);
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
