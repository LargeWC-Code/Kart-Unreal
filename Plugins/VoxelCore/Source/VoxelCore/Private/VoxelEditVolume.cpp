/********************************************************************
created:	2024/12/XX
filename: 	VoxelEditVolume.cpp
author:		Auto Generated

purpose:	VoxelEditVolume 实现
*********************************************************************/
#include "VoxelEditVolume.h"
#include "Components/BoxComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AVoxelEditVolume::AVoxelEditVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, VoxelType(1)
	, VoxelLayer(0)
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建盒子组件
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;
	
	// 设置默认大小为 100x100x100 厘米（1米 x 1米 x 1米）
	BoxComponent->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	// 设置可视化（使用线框模式）
	BoxComponent->SetHiddenInGame(true); // 游戏中隐藏
	BoxComponent->SetVisibility(true);   // 编辑器中可见
}

void AVoxelEditVolume::BeginPlay()
{
	Super::BeginPlay();
}

void AVoxelEditVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#if WITH_EDITOR
void AVoxelEditVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	// 对齐大小和位置到体素网格
	AlignToVoxelGrid();
}

void AVoxelEditVolume::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	
	// 移动后对齐位置到体素网格
	AlignToVoxelGrid();
}

void AVoxelEditVolume::AlignToVoxelGrid()
{
	if (!BoxComponent)
	{
		return;
	}

	const float VoxelSize = 100.0f; // 体素大小（厘米）
	
	// 获取当前的大小（半尺寸）
	FVector CurrentExtent = BoxComponent->GetUnscaledBoxExtent();
	
	// 计算完整大小（半尺寸 * 2）
	FVector FullSize = CurrentExtent * 2.0f;
	
	// 对齐大小到100的整数倍
	FVector AlignedSize;
	AlignedSize.X = FMath::FloorToInt(FullSize.X / VoxelSize) * VoxelSize;
	AlignedSize.Y = FMath::FloorToInt(FullSize.Y / VoxelSize) * VoxelSize;
	AlignedSize.Z = FMath::FloorToInt(FullSize.Z / VoxelSize) * VoxelSize;
	
	// 确保最小大小为100
	AlignedSize.X = FMath::Max(AlignedSize.X, VoxelSize);
	AlignedSize.Y = FMath::Max(AlignedSize.Y, VoxelSize);
	AlignedSize.Z = FMath::Max(AlignedSize.Z, VoxelSize);
	
	// 计算新的半尺寸
	FVector NewExtent = AlignedSize * 0.5f;
	
	// 获取当前位置
	FVector CurrentLocation = GetActorLocation();
	
	// 对齐位置到100的整数倍
	FVector AlignedLocation;
	AlignedLocation.X = FMath::FloorToInt(CurrentLocation.X / VoxelSize) * VoxelSize;
	AlignedLocation.Y = FMath::FloorToInt(CurrentLocation.Y / VoxelSize) * VoxelSize;
	AlignedLocation.Z = FMath::FloorToInt(CurrentLocation.Z / VoxelSize) * VoxelSize;
	
	// 如果大小/100是奇数，需要+50来对齐到网格中心
	// 检查每个轴的大小是否是奇数个100
	int32 VoxelCountX = FMath::FloorToInt(AlignedSize.X / VoxelSize);
	int32 VoxelCountY = FMath::FloorToInt(AlignedSize.Y / VoxelSize);
	int32 VoxelCountZ = FMath::FloorToInt(AlignedSize.Z / VoxelSize);
	
	if (VoxelCountX % 2 == 1) // 奇数
	{
		AlignedLocation.X += VoxelSize * 0.5f; // +50
	}
	if (VoxelCountY % 2 == 1) // 奇数
	{
		AlignedLocation.Y += VoxelSize * 0.5f; // +50
	}
	if (VoxelCountZ % 2 == 1) // 奇数
	{
		AlignedLocation.Z += VoxelSize * 0.5f; // +50
	}
	
	// 应用对齐后的值
	BoxComponent->SetBoxExtent(NewExtent);
	SetActorLocation(AlignedLocation);
}
#endif

FBox AVoxelEditVolume::GetVolumeBounds() const
{
	if (!BoxComponent)
	{
		return FBox(ForceInit);
	}

	FVector Origin = GetActorLocation();
	// GetScaledBoxExtent() 返回的是半尺寸（从中心到边缘的距离）
	FVector BoxExtent = BoxComponent->GetScaledBoxExtent();
	
	// 计算边界框：Min 和 Max 应该相差 2 * BoxExtent
	FVector Min = Origin - BoxExtent;
	FVector Max = Origin + BoxExtent;
	
	// 验证：Max - Min = 2 * BoxExtent（完整大小）
	// 这个计算是正确的，因为 BoxExtent 是半尺寸
	
	return FBox(Min, Max);
}

