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
	SCRIPT_PROPERTY(UCE_DWORD, TextureID)
	SCRIPT_PROPERTY(UCE_DWORD, LayerID)
	SCRIPT_PROPERTY(UCE_DWORD, Type)
	SCRIPT_PROPERTY(UCE_BYTE, YawRoll)
	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCVoxelData)

SCRIPT_IMPLEMENT_BEGIN(UCE_UCVoxelTileData, UCVoxelTileData)
	SCRIPT_CONSTRUCT_0()
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCVoxelTileData&)
	SCRIPT_PROPERTY(UCE_INT, TileX)
	SCRIPT_PROPERTY(UCE_INT, TileY)
	SCRIPT_PROPERTY(UCE_UCIntArray, AryVoxels)
	SCRIPT_PROPERTY(UCE_UCPointArray, AryTextureID)
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

SCRIPT_IMPLEMENT_BEGIN(UCE_UCVoxelTextureConfig, UCVoxelTextureConfig)
	SCRIPT_CONSTRUCT_0()
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCVoxelTextureConfig&)
	SCRIPT_PROPERTY(UCE_UCStringArray, AryTexturePaths)
	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCVoxelTextureConfig)

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
	, AryTextureID(in.AryTextureID)
{
}

UCVoxelTileData& UCVoxelTileData::operator =(ucCONST UCVoxelTileData& in)
{
	TileX = in.TileX;
	TileY = in.TileY;
	AryVoxels = in.AryVoxels;
	AryTextureID = in.AryTextureID;
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

UCVoxelTextureConfig::UCVoxelTextureConfig()
{
}

UCVoxelTextureConfig::UCVoxelTextureConfig(ucCONST UCVoxelTextureConfig& in)
{
}

UCVoxelTextureConfig& UCVoxelTextureConfig::operator =(ucCONST UCVoxelTextureConfig& in)
{
	AryTexturePaths = in.AryTexturePaths;
	return *this;
}

UCVoxelTextureConfig::~UCVoxelTextureConfig()
{
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
			// UCVoxelData 的默认值
			UCVoxelData EmptyVoxel;
			EmptyVoxel.TextureID = 0;
			EmptyVoxel.LayerID = UCVoxelData_Layer_Null;
			EmptyVoxel.Type = UCVoxelBlockType_Cube;
			UCVoxelData_SetYawAndRoll(EmptyVoxel, 0, 0);

			for (ucINT i = 0; i < TotalVoxels; ++i)
				TileData.AryVoxels[i] = EmptyVoxel.Data;

			// 初始化 AryTextureID，尺寸比 AryVoxels 的 XYZ 各自大一个单位
			const ucINT UVSizeX = TileSizeX + 1;  // 33
			const ucINT UVSizeY = TileSizeY + 1;  // 33
			const ucINT UVSizeZ = TileSizeZ + 1;  // 65
			const ucINT TotalUVs = UVSizeX * UVSizeY * UVSizeZ;  // 33 * 33 * 65 = 70785
			TileData.AryTextureID.SetSize(TotalUVs);

			// 填充默认 UV 值（0, 0）
			UCPoint DefaultUV;
			DefaultUV.x = 0;
			DefaultUV.y = 0;
			for (ucINT i = 0; i < TotalUVs; ++i)
				TileData.AryTextureID[i] = DefaultUV;

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
	if (!Formatter.Load(&File, Curr, &UCE_UCVoxelMapData::I))
		return ucFALSE;

	// 检查并初始化空的 AryVoxels 和 AryTextureID 数组
	const ucINT TileSizeX = VOXEL_TILE_SIZE_X;
	const ucINT TileSizeY = VOXEL_TILE_SIZE_Y;
	const ucINT TileSizeZ = VOXEL_TILE_SIZE_Z;
	const ucINT TotalVoxels = TileSizeX * TileSizeY * TileSizeZ; // 65536
	const ucINT UVSizeX = TileSizeX + 1;  // 33
	const ucINT UVSizeY = TileSizeY + 1;  // 33
	const ucINT UVSizeZ = TileSizeZ + 1;  // 65
	const ucINT TotalUVs = UVSizeX * UVSizeY * UVSizeZ;  // 33 * 33 * 65 = 70785

	// 默认体素值
	UCVoxelData EmptyVoxel;
	EmptyVoxel.TextureID = 0;
	EmptyVoxel.LayerID = UCVoxelData_Layer_Null;
	EmptyVoxel.Type = UCVoxelBlockType_Cube;
	UCVoxelData_SetYawAndRoll(EmptyVoxel, 0, 0);

	// 默认 UV 值
	UCPoint DefaultUV;
	DefaultUV.x = 0;
	DefaultUV.y = 0;

	// 遍历所有 Tile，检查并初始化空数组
	ucINT TileCount = Curr->_AryTiles.GetSize();
	for (ucINT i = 0; i < TileCount; ++i)
	{
		UCVoxelTileData& TileData = (UCVoxelTileData&)Curr->_AryTiles.GetAt(i);

		// 如果 AryVoxels 为空，初始化
		if (TileData.AryVoxels.GetSize() == 0)
		{
			TileData.AryVoxels.SetSize(TotalVoxels);
			for (ucINT j = 0; j < TotalVoxels; ++j)
				TileData.AryVoxels[j] = EmptyVoxel.Data;
		}

		// 如果 AryTextureID 为空，初始化
		if (TileData.AryTextureID.GetSize() == 0)
		{
			TileData.AryTextureID.SetSize(TotalUVs);
			for (ucINT j = 0; j < TotalUVs; ++j)
				TileData.AryTextureID[j] = DefaultUV;
		}
	}

	return ucTRUE;
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

ucVOID	UCVoxelMapManager::LoadResources(ucCONST UCString& BasePath)
{
	// 加载纹理配置
	UCString TextureConfigFile = BasePath;
	TextureConfigFile += _ucT("/Resource/Textures.texjson");
	
	UCFile File;
	if (File.Open(TextureConfigFile, UCFile::modeRead) == ucTRUE)
	{
		UCEJsonFormatter Formatter;
		Formatter.Load(&File, &TextureConfig, &UCE_UCVoxelTextureConfig::I);
	}
	// 如果文件不存在，使用默认值（不做任何操作，保持默认构造值）
}

ucVOID	UCVoxelMapManager::SaveResources(ucCONST UCString& BasePath)
{
	// 保存纹理配置
	UCString TextureConfigFile = BasePath;
	TextureConfigFile += _ucT("/Resource/Textures.texjson");
	
	UCFile File;
	if (File.Open(TextureConfigFile, UCFile::modeCreate | UCFile::modeWrite) == ucTRUE)
	{
		UCEJsonFormatter Formatter;
		Formatter.Save(&File, &TextureConfig, &UCE_UCVoxelTextureConfig::I);
	}
}
