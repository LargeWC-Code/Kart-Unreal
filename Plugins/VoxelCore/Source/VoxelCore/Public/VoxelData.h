/********************************************************************
created:	2024/12/XX
filename: 	VoxelData.h
author:		Auto Generated

purpose:	VoxelData - 体素数据类型定义
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "VoxelData.generated.h"

// 体素数据结构
USTRUCT(BlueprintType)
struct VOXELCORE_API FVoxelData
{
	GENERATED_BODY()

	// 体素类型（0表示空，非0表示有体素）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Type;

	// 体素层（可用于多层地形）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Layer;

	FVoxelData()
		: Type(0)
		, Layer(0)
	{
	}

	FVoxelData(uint8 InType, uint8 InLayer)
		: Type(InType)
		, Layer(InLayer)
	{
	}

	bool IsEmpty() const { return Type == 0; }
};
