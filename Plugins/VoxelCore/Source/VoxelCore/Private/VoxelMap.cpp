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

UCVoxelData::UCVoxelData() : Type(0), Layer(UCVoxelData_Layer_Null)
{

}

UCVoxelData::UCVoxelData(ucCONST UCVoxelData& in) : Type(in.Type), Layer(in.Layer)
{
}

UCVoxelData& UCVoxelData::operator =(ucCONST UCVoxelData& in)
{
	Type = in.Type;
	Layer = in.Layer;
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
	: Size(UCSize(4, 4))
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
}

ucBOOL	UCVoxelMapManager::LoadMap(ucCONST UCString& Filename)
{
	SF_DELETE(Curr);
	Curr = new UCVoxelMapData;
	// TODO: 实现文件加载逻辑
	UCFile File;
	if (File.Open(Filename, UCFile::modeRead) != ucTRUE)
		return ucFALSE;

	UCEJsonFormatter Formatter;
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

	UCEJsonFormatter Formatter;
	Formatter.Save(&File, Curr, &UCE_UCVoxelMapData::I);
}
