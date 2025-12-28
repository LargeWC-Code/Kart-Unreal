// Copyright Epic Games, Inc. All Rights Reserved.

#include "KartGameWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UKartGameWheelFront::UKartGameWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}