/********************************************************************
created:	2025/12/18
filename: 	UnrealGlobalOverlay.h

purpose:	Global Slate overlay (viewport-level, all players)
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RHI.h"
#endif

class SWidget;
struct FPointerEvent;
struct FGeometry;
struct FKeyEvent;
namespace UC3D { class UCUnreal3DDevice; }

// Forward declaration
class UCUnrealWnd;

/**
 * Mouse event callback delegate
 */
DECLARE_DELEGATE_RetVal_OneParam(bool, FOnOverlayMouseEvent, const FPointerEvent&);

/**
 * Installs a global overlay widget into GEngine->GameViewport once Slate + viewport are ready.
 * This is suitable for "global UI" that should not be occluded by 3D scene rendering.
 */
class MAGICXCORE_API FKartGameGlobalOverlay
{
public:
	static void Startup();
	static void Shutdown();

	// Bridge: let game code provide the UCUnreal3DDevice pointer after Create3DDevice().
	// The overlay can later use it to issue RHI draws at UI time (via custom Slate drawer).
	static void SetDevice(UC3D::UCUnreal3DDevice* InDevice);
	static UC3D::UCUnreal3DDevice* GetDevice();

	// Bridge: let game code provide the UCUnrealWnd pointer for input event forwarding
	static void SetUnrealWnd(UCUnrealWnd* InWnd);
	static UCUnrealWnd* GetUnrealWnd();

	// UI batch submission from UC3D device (game thread) -> rendered via Slate overlay (render thread).
	// A frame begins at UC3D device BeginScene() (we clear old batches), then each EndUIRender() appends.
	struct FUIBatch
	{
		// Raw vertex bytes; interpreted by the overlay drawer (currently FUCUnrealUIRectVertex).
		TArray<uint8> VertexData;
		uint32 VertexStride = 0;
		uint32 VertexCount = 0;
		
		// 纹理引用（用于 DrawVerticesWithTexture）
		// 如果为空，表示不绑定纹理（纯色渲染）
		FTextureRHIRef TextureRHI;
		int32 TextureFormat = 0;  // UC3DFORMAT 值（0=UC3DFMT_UNKNOWN, 1=UC3DFMT_A8），用于判断格式
	};

	static void BeginFrame();
	static void AppendBatch(FUIBatch&& Batch);
	static TArray<FUIBatch> GetBatchesSnapshot();

	// Mouse event callbacks
	static void SetOnMouseButtonDown(FOnOverlayMouseEvent InDelegate);
	static void SetOnMouseButtonUp(FOnOverlayMouseEvent InDelegate);
	static void SetOnMouseMove(FOnOverlayMouseEvent InDelegate);
	static void SetOnMouseButtonDoubleClick(FOnOverlayMouseEvent InDelegate);

	// Internal access functions for widget class
	static bool ExecuteMouseButtonDown(const FPointerEvent& MouseEvent);
	static bool ExecuteMouseButtonUp(const FPointerEvent& MouseEvent);
	static bool ExecuteMouseMove(const FPointerEvent& MouseEvent);
	static bool ExecuteMouseButtonDoubleClick(const FPointerEvent& MouseEvent);

private:
	static bool Tick(float DeltaTime);
	static void TryInstall();
	static void TryRemove();

private:
	static bool bInstalled;
	static TSharedPtr<SWidget> OverlayWidget;
	static FTSTicker::FDelegateHandle TickerHandle;
	static TAtomic<UC3D::UCUnreal3DDevice*> DevicePtr;
	static TAtomic<UCUnrealWnd*> UnrealWndPtr;

	static FRWLock BatchesLock;
	static TArray<FUIBatch> Batches;

	// Mouse event delegates
	static FOnOverlayMouseEvent OnMouseButtonDownDelegate;
	static FOnOverlayMouseEvent OnMouseButtonUpDelegate;
	static FOnOverlayMouseEvent OnMouseMoveDelegate;
	static FOnOverlayMouseEvent OnMouseButtonDoubleClickDelegate;
};


