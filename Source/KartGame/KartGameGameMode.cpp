// Copyright Epic Games, Inc. All Rights Reserved.

#include "KartGameGameMode.h"
#include "KartGamePlayerController.h"
#include "KartGamePawn.h"

AKartGameGameMode::AKartGameGameMode()
{
	PlayerControllerClass = AKartGamePlayerController::StaticClass();
	DefaultPawnClass = AKartGamePawn::StaticClass();
}
