/********************************************************************
created:	2024/12/XX
filename: 	VoxelEditVolume.h
author:		Auto Generated

purpose:	VoxelEditVolume - 体素编辑体积，可在编辑器中拖拽和调整大小
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "VoxelEditVolume.generated.h"

/**
 * AVoxelEditVolume - 体素编辑体积
 * 可在编辑器中拖拽到场景，调整大小，然后在 Edit 分页中应用来填充体素
 */
UCLASS(BlueprintType, Blueprintable, Placeable, Category = "Voxel")
class VOXELCORE_API AVoxelEditVolume : public AActor
{
	GENERATED_BODY()

public:
	AVoxelEditVolume(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif

	// ========== 编辑属性 ==========

	/** 体素类型（填充时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelEditVolume", meta = (ClampMin = "1"))
	uint8 VoxelType;

	/** 体素层 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelEditVolume")
	uint8 VoxelLayer;

	/** 获取编辑体积的边界框（世界坐标） */
	UFUNCTION(BlueprintCallable, Category = "VoxelEditVolume")
	FBox GetVolumeBounds() const;

#if WITH_EDITOR
	/** 对齐大小和位置到体素网格（100的整数倍） */
	void AlignToVoxelGrid();
#endif

private:
	// ========== 组件 ==========

	/** 盒子组件（用于显示和碰撞） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxComponent;
	
	// 允许外部访问 BoxComponent（用于调试）
	friend class FVoxelEditorEditorModeToolkit;
	friend class UVoxelEditorEditTool;
};

