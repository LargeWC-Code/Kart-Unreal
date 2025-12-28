// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ucunrealwnd.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "KartGameGameInstance.generated.h"

UCLASS()
class KARTGAME_API UKartGameGameInstance : public UGameInstance, public FTickableGameObject
{
    GENERATED_BODY()

public:
    UKartGameGameInstance();

    virtual void Init() override;
    virtual void Shutdown() override;
    
    // FTickableGameObject interface
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UKartGameGameInstance, STATGROUP_Tickables); }
    
private:
	// Unreal viewport resize callback -> sync UCUnrealWnd size
	void OnViewportResized(class FViewport* Viewport, uint32);

private:
	UCMainFiberData*    MainFiberData;
    UCUnrealWnd*        UnrealWnd;
	FDelegateHandle     ViewportResizedHandle;
	bool                bViewportSizeInitialized;  // Track if we've synced initial viewport size
};
