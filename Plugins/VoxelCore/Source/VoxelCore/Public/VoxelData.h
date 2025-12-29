#pragma once

#include "CoreMinimal.h"  // For VOXELCORE_API (defined by build system)
#include "ucgamebase.h"  // For UCString, UCEArray, etc. from MagicXCore

struct VOXELCORE_API UCVoxelPrefabData
{
	UCString		Name;
	ucINT			Type;
};

struct VOXELCORE_API UCVoxelMapData
{
	UCSize			Size;

	UCVoxelMapData();
	~UCVoxelMapData();
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
