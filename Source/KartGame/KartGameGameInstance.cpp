// Copyright Epic Games, Inc. All Rights Reserved.

#include "KartGameGameInstance.h"
#include "ucunreal3d/ucunreal3ddevice.h"
#include "UnrealClient.h"
#include "UnrealGlobalOverlay.h"  // From MagicXCore plugin

UKartGameGameInstance::UKartGameGameInstance()
{
	MainFiberData = ucNULL;
	UnrealWnd = nullptr;
	bViewportSizeInitialized = false;
}

void UKartGameGameInstance::Init()
{
    Super::Init();
    
	FKartGameGlobalOverlay::Startup();

	// Listen to Unreal viewport resize and sync UCUnrealWnd size automatically.
	ViewportResizedHandle = FViewport::ViewportResizedEvent.AddUObject(this, &UKartGameGameInstance::OnViewportResized);

	UnrealWnd = new UCUnrealWnd;
	UC3DService::HookCreate(UC3D::CreateUCUnreal3DService);
	UCString ContentDir = *FPaths::ProjectContentDir();

	UnrealWnd->InitGame(ucNULL, UCSize(1280, 720), ContentDir + _ucT("Script/Login.h"));
	
	// Set UCUnrealWnd pointer for input event forwarding
	FKartGameGlobalOverlay::SetUnrealWnd(UnrealWnd);
	
	MainFiberData = UCMainFiberData::GetMainFiberData();
	bViewportSizeInitialized = true;
}

void UKartGameGameInstance::Tick(float DeltaTime)
{
	if (MainFiberData)
		MainFiberData->OnDefaultDispatch(ucNULL, ucNULL);

	// Initialize viewport size on first tick when viewport is ready
	// Viewport is typically created after GameInstance::Init(), so we check here
	if (!bViewportSizeInitialized && GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport && UnrealWnd)
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		if (Viewport && Viewport->GetSizeXY().X > 0 && Viewport->GetSizeXY().Y > 0)
		{
			// Viewport is ready, sync initial size
			OnViewportResized(Viewport, 0);
			bViewportSizeInitialized = true;
		}
	}
}

void UKartGameGameInstance::Shutdown()
{
	if (ViewportResizedHandle.IsValid())
	{
		FViewport::ViewportResizedEvent.Remove(ViewportResizedHandle);
		ViewportResizedHandle.Reset();
	}

	if (MainFiberData)
		MainFiberData->Destroy();
	if (UnrealWnd)
	{
		UnrealWnd->ExitGame();
		delete UnrealWnd;
	}

	FKartGameGlobalOverlay::Shutdown();
	Super::Shutdown();
}

void UKartGameGameInstance::OnViewportResized(FViewport* Viewport, uint32)
{
	// Only care about the main game viewport
	if (GEngine == nullptr || GEngine->GameViewport == nullptr)
		return;

	FIntPoint Size = Viewport->GetSizeXY();
	// Mark as initialized if this is the first resize event
// 	if (!bViewportSizeInitialized)
// 	{
// 	}
// 	else if (UnrealWnd)
		UnrealWnd->SetSize(Size.X, Size.Y);
}
