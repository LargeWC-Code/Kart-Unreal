// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "KartGameKartPawn.h"
#include "KartGameKartCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class AKartGameKartCar : public AKartGameKartPawn
{
	GENERATED_BODY()
public:

	AKartGameKartCar();
};
