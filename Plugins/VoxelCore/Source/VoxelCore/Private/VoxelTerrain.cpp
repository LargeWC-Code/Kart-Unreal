/********************************************************************
created:	2024/12/XX
filename: 	VoxelTerrain.cpp
author:		Auto Generated

purpose:	VoxelTerrain 实现 - 管理地块的类
*********************************************************************/
#include "VoxelTerrain.h"
#include "VoxelTile.h"
#include "VoxelMap.h"
#include "VoxelWorldBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Guid.h"
#include "UObject/NameTypes.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Plane.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformFile.h"

UVoxelTerrain::UVoxelTerrain()
	: VoxelSize(100.0f)
	, Material(nullptr)
{
	TextureList.Empty();
}

void UVoxelTerrain::SetTextureList(const TArray<FString>& TexturePaths)
{
	TextureList.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("UVoxelTerrain::SetTextureList: Loading %d textures"), TexturePaths.Num());
	
	for (const FString& TexturePath : TexturePaths)
	{
		// 构建完整路径（相对于项目目录）
		FString FullPath = FPaths::ProjectDir() / TEXT("ExternalData/VoxelWorld") / TexturePath;
		
		UE_LOG(LogTemp, Log, TEXT("UVoxelTerrain::SetTextureList: Attempting to load texture from: %s"), *FullPath);
		
		// 检查文件是否存在
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.FileExists(*FullPath))
		{
			// 加载纹理
			TArray<uint8> ImageData;
			if (FFileHelper::LoadFileToArray(ImageData, *FullPath))
			{
				UTexture2D* Texture = FImageUtils::ImportBufferAsTexture2D(ImageData);
				if (Texture)
				{
					TextureList.Add(Texture);
					UE_LOG(LogTemp, Log, TEXT("UVoxelTerrain::SetTextureList: Successfully loaded texture: %s (Size: %dx%d)"), 
						*FullPath, Texture->GetSizeX(), Texture->GetSizeY());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("UVoxelTerrain::SetTextureList: Failed to import texture from: %s"), *FullPath);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UVoxelTerrain::SetTextureList: Failed to load file data from: %s"), *FullPath);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UVoxelTerrain::SetTextureList: Texture file does not exist: %s"), *FullPath);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("UVoxelTerrain::SetTextureList: Total loaded textures: %d"), TextureList.Num());
}

UTexture2D* UVoxelTerrain::GetTextureByID(int32 TextureID) const
{
	if (TextureID > 0 && TextureID <= TextureList.Num())
	{
		return TextureList[TextureID - 1]; // TextureID 从1开始，数组从0开始
	}
	return nullptr;
}

AVoxelTile* UVoxelTerrain::GetTile(int32 TileX, int32 TileY, UWorld* World, bool AutoCreate)
{
	if (!World)
	{
		return nullptr;
	}

	FIntPoint TileKey = GetTileKey(TileX, TileY);
	
	// 检查是否已存在
	if (TileMap.Contains(TileKey))
	{
		AVoxelTile* ExistingTile = TileMap[TileKey];
		if (IsValid(ExistingTile))
			return ExistingTile;
		// 如果已失效，移除
		TileMap.Remove(TileKey);
	}

	if (!AutoCreate)
		return nullptr;

	// 创建新地块
	FActorSpawnParameters SpawnParams;
	// Name 使用 GUID 确保唯一性
	FGuid UniqueID = FGuid::NewGuid();
	FString UniqueName = FString::Printf(TEXT("VoxelTile_%s"), *UniqueID.ToString(EGuidFormats::Short));
	SpawnParams.Name = FName(*UniqueName);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AVoxelTile* NewTile = World->SpawnActor<AVoxelTile>(SpawnParams);
	if (NewTile)
	{
		NewTile->TileCoord = FIntPoint(TileX, TileY);
		NewTile->VoxelSize = VoxelSize;
		NewTile->Material = Material;
		NewTile->SetTerrain(this); // 设置 Terrain 引用
		
		// 设置地块位置
		FVector WorldPos = GetTileWorldPosition(TileY, TileX);
		NewTile->SetActorLocation(WorldPos);
		
		// 设置文件夹路径：VoxelWorld/VoxelTiles
		NewTile->SetFolderPath(FName(TEXT("VoxelWorld/VoxelTiles")));
		
		// 设置 Label（显示名称）：格式为 (0,0) 或 (-1,-1)
		FString TileLabel = FString::Printf(TEXT("Tile(%d,%d)"), TileX, TileY);
		NewTile->SetActorLabel(TileLabel);
		
		// 默认不激活
		NewTile->SetActive(false);
		
		TileMap.Add(TileKey, NewTile);
	}

	return NewTile;
}

void UVoxelTerrain::SetTileActive(int32 TileX, int32 TileY, UWorld* World, bool bActive)
{
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UVoxelTerrain::SetTileActive: World is null"));
		return;
	}

	FIntPoint TileKey = GetTileKey(TileX, TileY);
	
	if (bActive)
	{
		// 激活：获取或创建 Tile，然后激活
		AVoxelTile* Tile = GetTile(TileX, TileY, World, true);
		if (Tile)
		{
			// 尝试从世界中的VoxelWorldBase获取地图数据
			const UCVoxelMapData* MapData = nullptr;
			for (TActorIterator<AVoxelWorldBase> ActorItr(World); ActorItr; ++ActorItr)
			{
				AVoxelWorldBase* VoxelWorld = *ActorItr;
				if (IsValid(VoxelWorld))
				{
					UCVoxelMapManager& MapManager = VoxelWorld->MapManager;
					if (MapManager.Curr)
					{
						MapData = MapManager.Curr;
						break;
					}
				}
			}

			// 如果找到了地图数据，尝试从地图数据中加载TileData
			if (MapData)
			{
				// 遍历地图数据中的Tile，查找匹配的TileData
				int32 TileCount = MapData->_AryTiles.GetSize();
				for (int32 i = 0; i < TileCount; ++i)
				{
					UCVoxelTileData& TileDataFromMap = (UCVoxelTileData&)MapData->_AryTiles.GetAt(i);
					if (TileDataFromMap.TileX == TileX && TileDataFromMap.TileY == TileY)
					{
						// 找到匹配的TileData，初始化Tile的TileData
						Tile->SetTileData(&TileDataFromMap);
						UE_LOG(LogTemp, Log, TEXT("UVoxelTerrain::SetTileActive: Loaded TileData from map for Tile(%d,%d)"), TileX, TileY);
						break;
					}
				}
			}
			
			Tile->SetActive(true);
			ActiveTiles.Add(TileKey);
		}
	}
	else
	{
		// 取消激活：销毁 Actor
		AVoxelTile* Tile = GetTile(TileX, TileY, World, false);
		if (Tile)
		{
			// 销毁 Actor
			World->DestroyActor(Tile);
			UE_LOG(LogTemp, Log, TEXT("UVoxelTerrain::SetTileActive: Destroyed tile at (%d, %d)"), TileX, TileY);
			
			// 从映射表中移除
			TileMap.Remove(TileKey);
		}
		
		// 从激活列表中移除
		ActiveTiles.Remove(TileKey);
	}
}

bool UVoxelTerrain::IsTileActive(int32 TileX, int32 TileY) const
{
	FIntPoint TileKey = GetTileKey(TileX, TileY);
	return ActiveTiles.Contains(TileKey);
}

void UVoxelTerrain::RemoveTile(int32 TileX, int32 TileY)
{
	FIntPoint TileKey = GetTileKey(TileX, TileY);
	if (TileMap.Contains(TileKey))
	{
		AVoxelTile* Tile = TileMap[TileKey];
		if (IsValid(Tile))
		{
			Tile->Destroy();
		}
		TileMap.Remove(TileKey);
		ActiveTiles.Remove(TileKey);
	}
}

FVector UVoxelTerrain::GetTileWorldPosition(int32 TileX, int32 TileY) const
{
	// 每个地块是32*32*64个单元格，每个单元格100单位（厘米）= 1米
	// 地块中心位置：Tile (0,0) 的中心在 (0, 0, 0)，覆盖范围从 -16*100 到 +16*100
	const float TileWorldSize = (float)VOXEL_TILE_SIZE_X * VoxelSize; // 32个单元格 * 100厘米 = 3200厘米 = 32米
	
	float WorldX = TileX * TileWorldSize;
	float WorldY = TileY * TileWorldSize;
	float WorldZ = 0.0f; // Z方向也居中，TileSizeZ = 64，中心在32*100，覆盖0到64*100
	
	return FVector(WorldX, WorldY, WorldZ);
}

FIntVector UVoxelTerrain::GetTileWorldGridPosition(int32 TileX, int32 TileY) const
{
	// 每个地块是32*32*64个单元格，每个单元格100单位（厘米）= 1米
	// 地块中心位置：Tile (0,0) 的中心在 (0, 0, 0)，覆盖范围从 -16*100 到 +16*100
	const float TileWorldSize = (float)VOXEL_TILE_SIZE_X; // 32个单元格 * 100厘米 = 3200厘米 = 32米

	float WorldX = TileX * TileWorldSize + TileWorldSize / 2;
	float WorldY = TileY * TileWorldSize + TileWorldSize / 2;
	float WorldZ = TileWorldSize; // Z方向也居中，TileSizeZ = 64，中心在32*100，覆盖0到64*100

	return FIntVector(WorldX, WorldY, WorldZ);
}

void UVoxelTerrain::FillRegion(const FVector& Min, const FVector& Max, uint8 VoxelType, uint8 Layer, UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UVoxelTerrain::FillRegion: World is null"));
		return;
	}

	// 计算体素坐标范围
	// 将世界坐标转换为体素坐标（除以 VoxelSize）
	int32 MinVoxelX = FMath::FloorToInt(Min.X / VoxelSize);
	int32 MinVoxelY = FMath::FloorToInt(Min.Y / VoxelSize);
	int32 MinVoxelZ = FMath::FloorToInt(Min.Z / VoxelSize);
	
	int32 MaxVoxelX = FMath::FloorToInt(Max.X / VoxelSize);
	int32 MaxVoxelY = FMath::FloorToInt(Max.Y / VoxelSize);
	int32 MaxVoxelZ = FMath::FloorToInt(Max.Z / VoxelSize);

	// 计算涉及的地块范围
	// 每个地块是 32x32x64 个体素，中心在 (0,0,0)，覆盖范围从 -16 到 +15
	const int32 TileSizeX = VOXEL_TILE_SIZE_X;
	const int32 TileSizeY = VOXEL_TILE_SIZE_Y;
	const int32 TileSizeZ = VOXEL_TILE_SIZE_Z;
	const int32 HalfTileSizeX = TileSizeX / 2; // 16
	const int32 HalfTileSizeY = TileSizeY / 2; // 16

	// 使用向下取整除法处理负数坐标
	// Tile坐标 = Floor((Voxel坐标 + 16) / 32)，这样Tile(0,0)覆盖-16到+15
	auto FloorDiv = [](int32 Dividend, int32 Divisor) -> int32 {
		if (Dividend >= 0)
			return Dividend / Divisor;
		else
			return (Dividend - Divisor + 1) / Divisor; // 向下取整：-1/32 = -1, not 0
	};

	// 将体素坐标转换为Tile坐标
	// Tile(0,0)的中心在体素坐标0，覆盖范围-16到+15
	// 所以：TileX = Floor((VoxelX + 16) / 32)
	int32 MinTileX = FloorDiv(MinVoxelX + HalfTileSizeX, TileSizeX);
	int32 MinTileY = FloorDiv(MinVoxelY + HalfTileSizeY, TileSizeY);
	int32 MaxTileX = FloorDiv(MaxVoxelX + HalfTileSizeX, TileSizeX);
	int32 MaxTileY = FloorDiv(MaxVoxelY + HalfTileSizeY, TileSizeY);

	// 遍历所有涉及的地块
	for (int32 TileY = MinTileY; TileY <= MaxTileY; ++TileY)
	{
		for (int32 TileX = MinTileX; TileX <= MaxTileX; ++TileX)
		{
			// 获取或创建地块
			AVoxelTile* Tile = GetTile(TileX, TileY, World, true);
			if (!Tile || !Tile->bIsActive)
				continue;

			// 获取 Tile 的世界位置
			FVector TileWorldPos = GetTileWorldPosition(TileY, TileX);
			
			// 将世界坐标转换为相对于 Tile 的局部坐标（Tile中心为原点）
			FVector LocalMin = Min - TileWorldPos;
			FVector LocalMax = Max - TileWorldPos;
			
			// 转换为体素坐标（相对于 Tile 中心，范围从 -16 到 +15）
			int32 LocalMinVoxelX = FMath::FloorToInt(LocalMin.X / VoxelSize);
			int32 LocalMinVoxelY = FMath::FloorToInt(LocalMin.Y / VoxelSize);
			int32 LocalMinVoxelZ = FMath::FloorToInt(LocalMin.Z / VoxelSize);
			
			int32 LocalMaxVoxelX = FMath::FloorToInt(LocalMax.X / VoxelSize);
			int32 LocalMaxVoxelY = FMath::FloorToInt(LocalMax.Y / VoxelSize);
			int32 LocalMaxVoxelZ = FMath::FloorToInt(LocalMax.Z / VoxelSize);
			
			// 限制在地块范围内（Tile中心为原点，范围从 -16 到 +15）
			const int32 HalfTileSizeZ = TileSizeZ / 2; // 32
			
			int32 LocalMinX = FMath::Max(-HalfTileSizeX, LocalMinVoxelX);
			int32 LocalMinY = FMath::Max(-HalfTileSizeY, LocalMinVoxelY);
			int32 LocalMinZ = FMath::Max(-HalfTileSizeZ, LocalMinVoxelZ);
			
			int32 LocalMaxX = FMath::Min(HalfTileSizeX - 1, LocalMaxVoxelX);
			int32 LocalMaxY = FMath::Min(HalfTileSizeY - 1, LocalMaxVoxelY);
			int32 LocalMaxZ = FMath::Min(HalfTileSizeZ - 1, LocalMaxVoxelZ);

			// 填充该地块内的体素
			// 需要将中心坐标（-16 到 +15）转换为存储坐标（0 到 31）
			for (int32 LocalZ = LocalMinZ; LocalZ <= LocalMaxZ; ++LocalZ)
			{
				for (int32 LocalY = LocalMinY; LocalY <= LocalMaxY; ++LocalY)
				{
					for (int32 LocalX = LocalMinX; LocalX <= LocalMaxX; ++LocalX)
					{
						// 转换为存储坐标：存储坐标 = 局部坐标 + 16（或 + 32 for Z）
						int32 StorageX = LocalX + HalfTileSizeX;
						int32 StorageY = LocalY + HalfTileSizeY;
						int32 StorageZ = LocalZ + HalfTileSizeZ;
						
						Tile->SetVoxelWithBlockType(StorageX, StorageY, StorageZ, VoxelType, Layer, UCVoxelBlockType_Cube, 0, 0, false);
					}
				}
			}

			// 更新网格
			Tile->UpdateMesh(true);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UVoxelTerrain::FillRegion: Filled region from (%d,%d,%d) to (%d,%d,%d) with type %d"), 
		MinVoxelX, MinVoxelY, MinVoxelZ, MaxVoxelX, MaxVoxelY, MaxVoxelZ, VoxelType);
}

bool UVoxelTerrain::SetVoxelAtWorldPosition(const FIntVector& WorldPosition, uint8 TextureID, uint8 Layer, uint8 BlockType, uint8 Roll, uint8 Pitch, UWorld* World, bool bUpdateMesh)
{
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UVoxelTerrain::SetVoxelAtWorldPosition: World is null"));
		return false;
	}

	// 将世界坐标转换为体素坐标
	int32 VoxelX = WorldPosition.X;
	int32 VoxelY = WorldPosition.Y;
	int32 VoxelZ = WorldPosition.Z;

	// 计算所属的Tile坐标
	const int32 TileSizeX = VOXEL_TILE_SIZE_X;
	const int32 TileSizeY = VOXEL_TILE_SIZE_Y;
	const int32 TileSizeZ = VOXEL_TILE_SIZE_Z;
	const int32 HalfTileSizeX = TileSizeX / 2; // 16
	const int32 HalfTileSizeY = TileSizeY / 2; // 16
	const int32 HalfTileSizeZ = TileSizeZ / 2; // 16

	auto FloorDiv = [](int32 Dividend, int32 Divisor) -> int32 {
		if (Dividend >= 0)
			return Dividend / Divisor;
		else
			return (Dividend - Divisor + 1) / Divisor;
	};

	int32 TileX = FloorDiv(VoxelX, TileSizeX);
	int32 TileY = FloorDiv(VoxelY, TileSizeY);

	// 获取或创建Tile
	AVoxelTile* Tile = GetTile(TileX, TileY, World, true);
	if (!Tile)
	{
		UE_LOG(LogTemp, Warning, TEXT("UVoxelTerrain::SetVoxelAtWorldPosition: Failed to get or create tile at (%d, %d)"), TileX, TileY);
		return false;
	}

	// 获取Tile的世界位置
	FIntVector TileWorldPos = GetTileWorldGridPosition(TileX, TileY);

	// 将世界坐标转换为相对于Tile的局部坐标（Tile中心为原点）
	FIntVector LocalPos = WorldPosition - TileWorldPos;

	// 转换为存储坐标（0到31 for X/Y, 0到63 for Z）
	int32 StorageX = LocalPos.X + HalfTileSizeX;
	int32 StorageY = LocalPos.Y + HalfTileSizeY;
	int32 StorageZ = LocalPos.Z + HalfTileSizeZ;

	// 设置体素
	Tile->SetVoxelWithBlockType(StorageX, StorageY, StorageZ, TextureID, Layer, BlockType, Roll, Pitch, bUpdateMesh);

	return true;
}

bool UVoxelTerrain::Intersect(const FVector& RayOrigin, const FVector& RayDirection, FIntVector& OutHitVoxelPosition, FVector& OutHitPosition, FVector& OutHitNormal) const
{
	// 参考UCPixelWorld::Intersect的实现
	// 第一步：AABB预筛选，收集所有可能碰撞的Tiles
	TArray<AVoxelTile*> CandidateTiles;
	
	for (const FIntPoint& TileKey : ActiveTiles)
	{
		if (!TileMap.Contains(TileKey))
			continue;
		
		AVoxelTile* Tile = TileMap[TileKey];
		if (!IsValid(Tile) || !Tile->IsActive())
			continue;
		
		// 快速AABB检测
		float TMin;
		if (Tile->IntersectAABB(RayOrigin, RayDirection, TMin))
		{
			CandidateTiles.Add(Tile);
		}
	}
	
	// 如果没有候选Tile，直接返回
	if (CandidateTiles.Num() == 0)
		return false;
	
	// 第二步：对所有候选Tile进行精确检测，找到最近的hit
	bool bFound = false;
	float MinHitDist = FLT_MAX;
	AVoxelTile* HitTile = nullptr;
	FIntVector HitVoxelPos(0, 0, 0);
	
	for (AVoxelTile* Tile : CandidateTiles)
	{
		float HitDist;
		FIntVector LocalVoxelPos;
		if (Tile->Intersect(RayOrigin, RayDirection, HitDist, LocalVoxelPos))
		{
			if (HitDist < MinHitDist && HitDist >= 0.0f)
			{
				MinHitDist = HitDist;
				HitTile = Tile;
				HitVoxelPos = LocalVoxelPos;
				bFound = true;
			}
		}
	}
	
	if (!bFound || !HitTile)
		return false;
	
	// 获取hit Tile的坐标
	FIntPoint HitTileCoord = HitTile->TileCoord;
	
	
	// 计算体素的局部坐标（从存储坐标转换为局部坐标）
	const int32 TileSizeX = VOXEL_TILE_SIZE_X;
	const int32 TileSizeY = VOXEL_TILE_SIZE_Y;
	const int32 TileSizeZ = VOXEL_TILE_SIZE_Z;
	const int32 HalfTileSizeX = TileSizeX / 2;
	const int32 HalfTileSizeY = TileSizeY / 2;
	const int32 HalfTileSizeZ = TileSizeZ / 2;
	
	int32 LocalVoxelX = HitVoxelPos.X - HalfTileSizeX;
	int32 LocalVoxelY = HitVoxelPos.Y - HalfTileSizeY;
	int32 LocalVoxelZ = HitVoxelPos.Z - HalfTileSizeZ;
	
	// 计算体素的世界坐标边界
	FVector TileWorldPos = GetTileWorldPosition(HitTileCoord.Y, HitTileCoord.X);
	FVector VoxelLocalMin = FVector(LocalVoxelX * VoxelSize, LocalVoxelY * VoxelSize, LocalVoxelZ * VoxelSize);
	FVector VoxelLocalMax = VoxelLocalMin + FVector(VoxelSize, VoxelSize, VoxelSize);
	
	// 转换到世界坐标
	FVector VoxelWorldMin = TileWorldPos + VoxelLocalMin;
	FVector VoxelWorldMax = TileWorldPos + VoxelLocalMax;
	
	// 计算hit位置（参考ucpixelworld的方法）
	FVector LocalRayOrigin = RayOrigin - TileWorldPos;
	FVector LocalRayDir = RayDirection;
	
	// 计算体素的6个面的平面（参考ucpixelworld的方法）
	FPlane Sides[6] = {
		FPlane(0.0f, 0.0f, +1.0f, +VoxelLocalMax.Z),   // -Z face (Top)
		FPlane(0.0f, 0.0f, -1.0f, -VoxelLocalMin.Z),   // +Z face (Bottom)
		FPlane(0.0f, +1.0f, 0.0f, +VoxelLocalMax.Y),   // -Y face
		FPlane(-1.0f, 0.0f, 0.0f, -VoxelLocalMin.X),   // +X face
		FPlane(0.0f, -1.0f, 0.0f, -VoxelLocalMin.Y),   // +Y face
		FPlane(+1.0f, 0.0f, 0.0f, +VoxelLocalMax.X),   // -X face
	};
		
	int32 FindFaceID = -1;
	float MinFaceDist = FLT_MAX;
	
	// 找到射线与6个面的最近交点（参考ucpixelworld的逻辑）
	for (int32 FaceIdx = 0; FaceIdx < 6; ++FaceIdx)
	{
		FVector PlaneNormal = Sides[FaceIdx];
		float CosTheta = FVector::DotProduct(PlaneNormal, LocalRayDir);
		float Dist = Sides[FaceIdx].PlaneDot(LocalRayOrigin);
		
		// 如果几乎平行或几乎相交，跳过
		if (FMath::Abs(Dist) < 0.0001f || FMath::Abs(CosTheta) < 0.0001f)
			continue;
		
		// 只关心沿射线方向的交点
		float FaceHitDist = -Dist / CosTheta;
		if (FaceHitDist < 0.0f)
			continue;
		
		if (FaceHitDist < MinFaceDist)
		{
			FVector FaceHitPt = LocalRayOrigin + LocalRayDir * FaceHitDist;
			
			// 验证交点是否在体素的边界内（参考代码使用+0.00015f容差）
			bool bValid = true;
			for (int32 j = 0; j < 6 && bValid; ++j)
			{
				if (j == FaceIdx)
					continue;
				float D = -Sides[j].PlaneDot(FaceHitPt);
				bValid = ((D + 0.00015f) >= 0.0f);
			}
			
			if (bValid)
			{
				FindFaceID = FaceIdx;
				MinFaceDist = FaceHitDist;
			}
		}
	}
	
	if (FindFaceID == -1)
		return false;
	
	const float TileWorldSize = (float)VOXEL_TILE_SIZE_X; // 32个单元格 * 100厘米 = 3200厘米 = 32米
	FIntVector TilePosition(HitTile->TileCoord.X* TileWorldSize, HitTile->TileCoord.Y* TileWorldSize, 0);
	// 计算世界坐标的hit位置和法线
	FVector LocalHitPos = LocalRayOrigin + LocalRayDir * MinFaceDist;
	OutHitVoxelPosition = HitVoxelPos + TilePosition;
	OutHitPosition = LocalHitPos;
	OutHitNormal = Sides[FindFaceID];
	
	return true;
}
