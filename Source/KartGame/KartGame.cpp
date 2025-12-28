// Copyright Epic Games, Inc. All Rights Reserved.

#include "KartGame.h"
#include "Modules/ModuleManager.h"
#include "UnrealGlobalOverlay.h"  // From MagicXCore plugin

// Replace default module impl so we can install a global viewport overlay (all players).
class FKartGameModule final : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
	}

	virtual void ShutdownModule() override
	{
		FDefaultGameModuleImpl::ShutdownModule();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FKartGameModule, KartGame, "KartGame");

DEFINE_LOG_CATEGORY(LogKartGame)