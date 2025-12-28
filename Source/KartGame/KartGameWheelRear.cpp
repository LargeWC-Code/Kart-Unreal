// Copyright Epic Games, Inc. All Rights Reserved.

#include "KartGameWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UKartGameWheelRear::UKartGameWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}