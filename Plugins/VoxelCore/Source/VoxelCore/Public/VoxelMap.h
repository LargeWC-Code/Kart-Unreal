#pragma once

#include "CoreMinimal.h"  // For VOXELCORE_API (defined by build system)
#include "ucgamebase.h"  // For UCString, UCEArray, etc. from MagicXCore

struct VOXELCORE_API UCVoxelPrefabData
{
	UCString		Name;
	ucINT			Type;
};

SCRIPT_DECLARE(VOXELCORE_API, UCE_UCVoxelPrefabData, UCVoxelPrefabData, ucTRUE);

enum UCVoxelData_Layer
{
	UCVoxelData_Layer_Null = -1,
	UCVoxelData_Layer_Ground = 0,
	UCVoxelData_Layer_Water
};

// Tile 数据：存储一个 Tile 的坐标和体素数据
struct VOXELCORE_API UCVoxelData
{
	ucINT			Type;
	ucINT			Layer;

	UCVoxelData();
	UCVoxelData(ucCONST UCVoxelData&);
	~UCVoxelData();

	UCVoxelData& operator =(ucCONST UCVoxelData&);
};

SCRIPT_DECLARE(VOXELCORE_API, UCE_UCVoxelData, UCVoxelData, ucTRUE);

// Tile 数据：存储一个 Tile 的坐标和体素数据
struct VOXELCORE_API UCVoxelTileData
{
	ucINT			TileX;			// Tile 坐标 X
	ucINT			TileY;			// Tile 坐标 Y
	union
	{
		UCEArray<UCVoxelData>			AryVoxels;		// 体素数组：32*32*64 = 65536 个字节
		_UCEArray						_AryVoxels;		// 体素数组：32*32*64 = 65536 个字节
	};

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
