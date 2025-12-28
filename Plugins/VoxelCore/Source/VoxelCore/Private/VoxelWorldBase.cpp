/********************************************************************
created:	2024/12/XX
filename: 	VoxelWorld.cpp
author:		Auto Generated

purpose:	VoxelWorld - 体素世界管理类实现
*********************************************************************/

#include "VoxelWorldBase.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AVoxelWorldBase::AVoxelWorldBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TerrainActor(nullptr)
	, bAutoCreateTerrain(true)
	, InitialTerrainSize(64, 64, 64)
	, VoxelSize(100.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// 初始化地图管理器
	InitializeMapManager();
}

void AVoxelWorldBase::BeginPlay()
{
	Super::BeginPlay();

	// 如果启用自动创建地形，则创建地形对象
	if (bAutoCreateTerrain && !TerrainActor)
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
	
	if (MapManager.LoadFromFile(UCFilename))
	{
		// 加载成功后，可以根据地图数据初始化地形和预制件
		// TODO: 实现从地图数据加载地形和预制件的逻辑
		
		UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Successfully loaded map from %s"), *Filename);
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("VoxelWorld: Failed to load map from %s"), *Filename);
	return false;
}

void AVoxelWorldBase::SaveMap(const FString& Filename)
{
	UCString UCFilename = FStringToUCString(Filename);
	
	MapManager.SaveToFile(UCFilename);
	
	UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Saved map to %s"), *Filename);
}

AVoxelTerrain* AVoxelWorldBase::CreateTerrain()
{
	if (TerrainActor)
	{
		return TerrainActor;
	}

	// 创建地形 Actor 作为子对象
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TerrainActor = GetWorld()->SpawnActor<AVoxelTerrain>(SpawnParams);
	if (TerrainActor)
	{
		// 设置地形属性
		TerrainActor->TerrainSize = InitialTerrainSize;
		TerrainActor->VoxelSize = VoxelSize;
		TerrainActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		
		UE_LOG(LogTemp, Log, TEXT("VoxelWorld: Created terrain actor"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VoxelWorld: Failed to create terrain actor"));
	}

	return TerrainActor;
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

