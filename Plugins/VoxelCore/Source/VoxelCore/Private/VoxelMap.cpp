#include "VoxelMap.h"

SCRIPT_IMPLEMENT_BEGIN(UCE_UCVoxelPrefabData, UCVoxelPrefabData)
	SCRIPT_CONSTRUCT_0()
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCVoxelPrefabData&)
	SCRIPT_PROPERTY(UCE_UCString, Name)
	SCRIPT_PROPERTY(UCE_INT, Type)
	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCVoxelPrefabData)

SCRIPT_IMPLEMENT_BEGIN(UCE_UCVoxelData, UCVoxelData)
	SCRIPT_CONSTRUCT_0()
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCVoxelData&)
	SCRIPT_PROPERTY(UCE_INT, Type)
	SCRIPT_PROPERTY(UCE_INT, Layer)
	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCVoxelData)

SCRIPT_IMPLEMENT_BEGIN(UCE_UCVoxelTileData, UCVoxelTileData)
	SCRIPT_CONSTRUCT_0()
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCVoxelTileData&)
	SCRIPT_PROPERTY(UCE_INT, TileX)
	SCRIPT_PROPERTY(UCE_INT, TileY)
	SCRIPT_PROPERTY(UCE_UCIntArray, AryVoxels)
	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCVoxelTileData)

SCRIPT_IMPLEMENT_BEGIN(UCE_UCVoxelMapData, UCVoxelMapData)
	SCRIPT_CONSTRUCT_0()
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCVoxelMapData&)
	SCRIPT_PROPERTY(UCE_UCSize, Size)
	SCRIPT_PROPERTY(UCE_UCEArray, AryTiles)
	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCVoxelMapData)

SCRIPT_IMPLEMENT_BEGIN(UCE_UCVoxelMapNodeData, UCVoxelMapNodeData)
	SCRIPT_CONSTRUCT_0()
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCVoxelMapNodeData&)
	SCRIPT_PROPERTY(UCE_INT, Type)
	SCRIPT_PROPERTY(UCE_UCString, Name)
	SCRIPT_PROPERTY(UCE_UCEArray, AryNodes)
	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCVoxelMapNodeData)

#define new UCNEW

UCVoxelData::UCVoxelData() : Data(0)
{

}

UCVoxelData::UCVoxelData(ucCONST UCVoxelData& in) : Data(in.Data)
{
}

UCVoxelData& UCVoxelData::operator =(ucCONST UCVoxelData& in)
{
	Data = in.Data;
	return *this;
}

UCVoxelData::~UCVoxelData()
{

}

UCVoxelTileData::UCVoxelTileData() 
	: TileX(0)
	, TileY(0)
{
}

UCVoxelTileData::UCVoxelTileData(ucCONST UCVoxelTileData& in)
	: TileX(in.TileX)
	, TileY(in.TileY)
	, AryVoxels(in.AryVoxels)
{
}

UCVoxelTileData& UCVoxelTileData::operator =(ucCONST UCVoxelTileData& in)
{
	TileX = in.TileX;
	TileY = in.TileY;
	AryVoxels = in.AryVoxels;
	return *this;
}

UCVoxelTileData::~UCVoxelTileData()
{
}

UCVoxelMapData::UCVoxelMapData()
	: Size(UCSize(0, 0))
	, _AryTiles(UCEVariableInfoCC(&UCE_UCVoxelTileData::I))
{
}

UCVoxelMapData::UCVoxelMapData(ucCONST UCVoxelMapData& in)
	: Size(in.Size)
	, _AryTiles(in._AryTiles)
{
}

UCVoxelMapData& UCVoxelMapData::operator =(ucCONST UCVoxelMapData& in)
{
	Size = in.Size;
	_AryTiles = in._AryTiles;
	return *this;
}

UCVoxelMapData::~UCVoxelMapData()
{
	_AryTiles.~_UCEArray();
}

UCVoxelMapNodeData::UCVoxelMapNodeData() : _AryNodes(UCEVariableInfoCC(&UCE_UCVoxelMapNodeData::I))
{

}

UCVoxelMapNodeData::UCVoxelMapNodeData(ucCONST UCVoxelMapNodeData& in) : Type(in.Type), Name(in.Name), _AryNodes(in._AryNodes)
{
}

UCVoxelMapNodeData& UCVoxelMapNodeData::operator =(ucCONST UCVoxelMapNodeData& in)
{
	Type = in.Type;
	Name = in.Name;
	_AryNodes = in._AryNodes;
	return *this;
}

UCVoxelMapNodeData::~UCVoxelMapNodeData()
{
	_AryNodes.~_UCEArray();
}

UCVoxelMapManager::UCVoxelMapManager()
{
	Root.Type = UCVoxelMapNodeData_Folder;
	Root.Name = _ucT("Root");

	Curr = ucNULL;
}

UCVoxelMapManager::~UCVoxelMapManager()
{
	SF_DELETE(Curr);
}

ucBOOL	UCVoxelMapManager::LoadFromFile(ucCONST UCString& Filename)
{
	// TODO: 实现文件加载逻辑
	UCFile File;
	if (File.Open(Filename, UCFile::modeRead) != ucTRUE)
		return ucFALSE;

	UCEJsonFormatter Formatter;
	return Formatter.Load(&File, &Root, &UCE_UCVoxelMapNodeData::I);
}

ucVOID	UCVoxelMapManager::SaveToFile(ucCONST UCString& Filename)
{
	// TODO: 实现文件保存逻辑
	UCFile File;
	if (File.Open(Filename, UCFile::modeCreate | UCFile::modeWrite) != ucTRUE)
		return;

	UCEJsonFormatter Formatter;
	Formatter.Save(&File, &Root, &UCE_UCVoxelMapNodeData::I);
}

ucVOID	UCVoxelMapManager::NewCurrentMap(UCSize Size)
{
	SF_DELETE(Curr);
	Curr = new UCVoxelMapData;

	Curr->Size = Size;

	// 清空现有的 Tile 数据
	Curr->_AryTiles.RemoveAll();

	// Tile 尺寸（固定为 32*32*64）
	const ucINT TileSizeX = VOXEL_TILE_SIZE_X;
	const ucINT TileSizeY = VOXEL_TILE_SIZE_Y;
	const ucINT TileSizeZ = VOXEL_TILE_SIZE_Z;
	const ucINT TotalVoxels = TileSizeX * TileSizeY * TileSizeZ; // 65536

	// 计算 Tile 坐标范围（中心为原点）
	// X: 从 -(Size.cx - 1) / 2 到 (Size.cx - 1) / 2
	// Y: 从 -(Size.cy - 1) / 2 到 (Size.cy - 1) / 2
	ucINT StartX = -(Size.cx - 1) / 2;
	ucINT EndX = (Size.cx - 1) / 2;
	ucINT StartY = -(Size.cy - 1) / 2;
	ucINT EndY = (Size.cy - 1) / 2;

	// 为每个 Tile 坐标创建 TileData
	for (ucINT TileY = StartY; TileY <= EndY; ++TileY)
	{
		for (ucINT TileX = StartX; TileX <= EndX; ++TileX)
		{
			UCVoxelTileData TileData;
			TileData.TileX = TileX;
			TileData.TileY = TileY;

			// 清空并初始化 AryVoxels
			TileData.AryVoxels.SetSize(TotalVoxels);

			// 填充 VOXEL_TILE_SIZE_X*VOXEL_TILE_SIZE_Y*VOXEL_TILE_SIZE_Z = 65536 个 0（空体素）
			// UCVoxelData 的默认值是 Type=0, Layer=UCVoxelData_Layer_Null
			UCVoxelData EmptyVoxel;
			EmptyVoxel.Type = 0;
			EmptyVoxel.Layer = UCVoxelData_Layer_Null;

			for (ucINT i = 0; i < TotalVoxels; ++i)
				TileData.AryVoxels[i] = EmptyVoxel.Data;

			// 添加到地图数据中
			Curr->_AryTiles.Add(*(_UCEArray::TValue*)&TileData);
		}
	}
}

ucBOOL	UCVoxelMapManager::LoadMap(ucCONST UCString& Filename)
{
	SF_DELETE(Curr);
	Curr = new UCVoxelMapData;
	// TODO: 实现文件加载逻辑
	UCFile File;
	if (File.Open(Filename, UCFile::modeRead) != ucTRUE)
		return ucFALSE;

	UCEBinaryFormatter Formatter;
	return Formatter.Load(&File, Curr, &UCE_UCVoxelMapData::I);
}

ucVOID	UCVoxelMapManager::SaveMap(ucCONST UCString& Filename)
{
	if (Curr == ucNULL)
		return;

	// TODO: 实现文件保存逻辑
	UCFile File;
	if (File.Open(Filename, UCFile::modeCreate | UCFile::modeWrite) != ucTRUE)
		return;

	UCEBinaryFormatter Formatter;
	Formatter.Save(&File, Curr, &UCE_UCVoxelMapData::I);
}
