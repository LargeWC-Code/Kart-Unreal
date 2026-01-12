/********************************************************************
created:	2024/12/XX
filename: 	VoxelWorld.cpp
author:		Auto Generated

purpose:	VoxelWorld - 体素世界管理类实现
*********************************************************************/

#include "VoxelWorldBase.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"

AVoxelWorldBase::AVoxelWorldBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TerrainObject(nullptr)
	, bAutoCreateTerrain(true)
	, InitialTerrainSize(64, 64, 64)
	, VoxelSize(100.0f)
	, TileMaterial(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// 自动加载默认材质 M_VoxelTile
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Game/Materials/M_VoxelTile"));
	if (MaterialFinder.Succeeded())
	{
		TileMaterial = MaterialFinder.Object;
		UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Auto-loaded material M_VoxelTile"));
	}
	else
	{
		// 如果 ConstructorHelpers 失败，尝试使用 LoadObject（运行时加载）
		TileMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_VoxelTile"));
		if (TileMaterial)
		{
			UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Loaded material M_VoxelTile using LoadObject"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("VoxelWorld: Failed to load material M_VoxelTile. Please set TileMaterial manually in the editor."));
		}
	}

	// 初始化地图管理器
	InitializeMapManager();
}

void AVoxelWorldBase::BeginPlay()
{
	Super::BeginPlay();

	// 如果启用自动创建地形，则创建地形管理对象
	if (bAutoCreateTerrain && !TerrainObject)
	{
		CreateTerrain();
	}
}

void AVoxelWorldBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVoxelWorldBase::InitializeMapManager()
{
	// UCVoxelMapManager 的初始化（如果需要）
	// 目前 MapManager 是值类型，不需要特殊初始化
}

bool AVoxelWorldBase::LoadMap(const FString& Filename)
{
	UCString UCFilename = FStringToUCString(Filename);
	
	if (MapManager.LoadMap(UCFilename))
		return true;
	return false;
}

void AVoxelWorldBase::SaveMap(const FString& Filename)
{
	UCString UCFilename = FStringToUCString(Filename);
	
	// 确保MapManager有当前地图数据
	if (!MapManager.Curr)
		return;
	
	// 保存地图
	MapManager.SaveMap(UCFilename);
}

UVoxelTerrain* AVoxelWorldBase::CreateTerrain()
{
	if (TerrainObject)
	{
		return TerrainObject;
	}

	// 创建地形管理对象
	TerrainObject = NewObject<UVoxelTerrain>(this);
	if (TerrainObject)
	{
		// 设置地形属性
		TerrainObject->VoxelSize = VoxelSize;
		
		// 设置材质（从 VoxelWorldBase 传递）
		TerrainObject->Material = TileMaterial;
		
		// 从 MapManager 加载纹理列表
		TArray<FString> TexturePaths;
		const UCStringArray& TexturePathArray = MapManager.TextureConfig.AryTexturePaths;
		for (ucINT i = 0; i < TexturePathArray.GetSize(); ++i)
		{
			UCString TexturePathUC = TexturePathArray[i];
			FString TexturePath = UCStringToFString(TexturePathUC);
			TexturePaths.Add(TexturePath);
		}
		TerrainObject->SetTextureList(TexturePaths);
		
		UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Created terrain manager with %d textures"), TexturePaths.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VoxelWorld: Failed to create terrain manager"));
	}

	return TerrainObject;
}

int32 AVoxelWorldBase::AddPrefab(const FString& Name, int32 Type)
{
	UCVoxelPrefabData PrefabData;
	PrefabData.Name = FStringToUCString(Name);
	PrefabData.Type = Type;

	int32 Index = PrefabList.Add(PrefabData);
	
	UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Added prefab [%d] Name=%s Type=%d"), Index, *Name, Type);
	
	return Index;
}

void AVoxelWorldBase::RemovePrefab(int32 Index)
{
	if (Index >= 0 && Index < PrefabList.Num())
	{
		PrefabList.RemoveAt(Index);
		
		// 如果对应的 Actor 存在，也删除它
		if (Index < PrefabActors.Num() && PrefabActors[Index])
		{
			PrefabActors[Index]->Destroy();
			PrefabActors.RemoveAt(Index);
		}
		
		UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Removed prefab [%d]"), Index);
	}
}

int32 AVoxelWorldBase::GetPrefabCount() const
{
	return PrefabList.Num();
}

void AVoxelWorldBase::GetPrefab(int32 Index, FString& OutName, int32& OutType) const
{
	if (Index >= 0 && Index < PrefabList.Num())
	{
		const UCVoxelPrefabData& Prefab = PrefabList[Index];
		OutName = UCStringToFString(Prefab.Name);
		OutType = Prefab.Type;
	}
	else
	{
		OutName = TEXT("");
		OutType = 0;
	}
}

FString AVoxelWorldBase::UCStringToFString(const UCString& UCStr) const
{
	// UCString 到 FString 的转换
	// UCString 有隐式转换操作符 operator PCXSTR() const，可以直接转换为 const wchar_t*
	if (UCStr.GetLength() > 0)
	{
		// 使用隐式转换操作符，UCString 会自动转换为 const wchar_t*
		const wchar_t* Buffer = UCStr;  // 隐式转换 operator PCXSTR()
		return FString(UCStr.GetLength(), Buffer);
	}
	return FString();
}

UCString AVoxelWorldBase::FStringToUCString(const FString& FStr) const
{
	// FString 到 UCString 的转换
	// 参考 KartGameGameInstance.cpp: UCString ContentDir = *FPaths::ProjectContentDir();
	// FString 的 *FStr 返回 const TCHAR*，在 Windows 上 TCHAR 就是 wchar_t
	// UCString 有接受 const wchar_t* 的构造函数
	return UCString(*FStr);
}

