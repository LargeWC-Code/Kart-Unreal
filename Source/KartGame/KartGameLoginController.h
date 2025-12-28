// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KartGameLoginController.generated.h"

/**
 * 
 */
UCLASS()
class KARTGAME_API AKartGameLoginController : public APlayerController
{
	GENERATED_BODY()
protected:
	/** Gameplay initialization */
	virtual void BeginPlay() override;
};
