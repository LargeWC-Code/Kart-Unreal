/********************************************************************
created:	2025/12/18
filename: 	UnrealGlobalOverlay.cpp

purpose:	Global Slate overlay (viewport-level, all players)
*********************************************************************/

#include "UnrealGlobalOverlay.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/RenderingCommon.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeRWLock.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Input/Events.h"
#include "Layout/Geometry.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"

#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#include "RHICommandList.h"
#include "RHIResources.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "StaticBoundShaderState.h"
#include "TextureResource.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderUtils.h"

// 着色器类的头文件
#include "UCUnrealUIRectVertexShader.h"
#include "UCUnrealUIRectPixelShader.h"

#include "ucunrealwnd.h"
#include "ucunreal3d/ucunreal3ddevice.h"

// RDG pass params (UE5.6): use Shader Parameter Struct style (no FRenderGraphPassParameters base class).
BEGIN_SHADER_PARAMETER_STRUCT(FUCUnrealOverlayDrawBatchesPassParameters, )
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

// Custom Slate element: executed on RenderThread (UE5.6 uses RDG here).
class FUCUnrealOverlayDrawer final : public ICustomSlateElement
{
public:
	virtual void Draw_RenderThread(FRDGBuilder& GraphBuilder, const FDrawPassInputs& Inputs) override
	{
		TArray<FKartGameGlobalOverlay::FUIBatch> LocalBatches = FKartGameGlobalOverlay::GetBatchesSnapshot();

		if (LocalBatches.Num() == 0 || Inputs.OutputTexture == nullptr)
		{
			return;
		}

		FRDGTexture* Output = Inputs.OutputTexture;
		const FIntRect ViewRect = Inputs.SceneViewRect;

		FUCUnrealOverlayDrawBatchesPassParameters* PassParameters = GraphBuilder.AllocParameters<FUCUnrealOverlayDrawBatchesPassParameters>();
		PassParameters->RenderTargets[0] = FRenderTargetBinding(Output, ERenderTargetLoadAction::ELoad);

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("UCUnrealOverlay_DrawBatches"),
			PassParameters,
			ERDGPassFlags::Raster,
			[LocalBatches = MoveTemp(LocalBatches), ViewRect](FRHICommandList& RHICmdList)
			{
				RHICmdList.SetViewport(
					(float)ViewRect.Min.X, (float)ViewRect.Min.Y, 0.0f,
					(float)ViewRect.Max.X, (float)ViewRect.Max.Y, 1.0f);

				FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
				if (ShaderMap == nullptr)
				{
					return;
				}

				TShaderMapRef<FUCUnrealOverlayRectVS> VS(ShaderMap);

				// UC3DFMT_A8 = 1
				constexpr int32 UC3DFMT_A8 = 1;

				FGraphicsPipelineStateInitializer PSOInit;
				PSOInit.PrimitiveType = PT_TriangleList;
				PSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_One>::GetRHI();
				PSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
				PSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				PSOInit.BoundShaderState.VertexDeclarationRHI = GetUCUnrealOverlayRectVertexDecl();
				PSOInit.BoundShaderState.VertexShaderRHI = VS.GetVertexShader();

				for (const FKartGameGlobalOverlay::FUIBatch& Batch : LocalBatches)
				{
					if (Batch.VertexData.Num() == 0 || Batch.VertexStride == 0 || Batch.VertexCount == 0)
					{
						continue;
					}

					// 根据 Batch 条件选择着色器变体
					bool bUseTexture = Batch.TextureRHI.IsValid();
					
					// 直接从 RHI 纹理获取格式来判断是否为 A8
					// 字体纹理可能是 PF_G8 (A8/Gray8) 或其他格式
					bool bIsA8Format = false;
					if (bUseTexture && Batch.TextureRHI.IsValid())
					{
						// UE 5.1+ 中可以直接在 FTextureRHIRef 上调用 GetFormat()
						EPixelFormat PixelFormat = Batch.TextureRHI->GetFormat();
						// PF_G8 是 Unreal 中的 8-bit grayscale/alpha 格式，对应 UC3DFMT_A8
						// 也可以通过 Batch.TextureFormat 判断（如果传递了的话）
						bIsA8Format = (PixelFormat == PF_G8) || (Batch.TextureFormat == 1); // 1 = UC3DFMT_A8
					}

					// 构建着色器变体域
					FUCUnrealOverlayRectPSPermutationDomain PermutationVector;
					PermutationVector.Set<FUCUnrealOverlayRectPS_UseTexture>(bUseTexture);
					PermutationVector.Set<FUCUnrealOverlayRectPS_IsA8Format>(bIsA8Format && bUseTexture); // 只有使用纹理时才需要 A8 格式

					// 获取对应的着色器变体
					TShaderMapRef<FUCUnrealOverlayRectPS> PS(ShaderMap, PermutationVector);
					if (!PS.IsValid())
					{
						continue; // 跳过无效的着色器变体
					}

					// 准备像素着色器参数（先准备，稍后绑定着色器后再设置）
					FUCUnrealOverlayRectPS::FParameters PSParameters;
					if (bUseTexture && Batch.TextureRHI.IsValid())
					{
						// 在 RHI 命令列表中，直接使用 RHI 纹理
						PSParameters.UITexture = Batch.TextureRHI;
						PSParameters.UITextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					}
					else
					{
						// 无纹理变体可能不需要纹理参数，但为了兼容性设置占位符
						PSParameters.UITexture = GBlackTexture->TextureRHI;
						PSParameters.UITextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					}

					// 设置像素着色器到 PSOInit
					PSOInit.BoundShaderState.PixelShaderRHI = PS.GetPixelShader();

					// RDG will have a render pass active via the bindings above.
					RHICmdList.ApplyCachedRenderTargets(PSOInit);
					
					// 先绑定着色器到渲染管线
					SetGraphicsPipelineState(RHICmdList, PSOInit, 0);
					
					// 着色器绑定后，再设置参数
					SetShaderParameters(RHICmdList, PS, PS.GetPixelShader(), PSParameters);

					// 使用新的 FRHIBufferCreateDesc API
					// 构造函数：FRHIBufferCreateDesc(const TCHAR* InDebugName, uint32 InSize, uint32 InStride, EBufferUsageFlags InUsage)
					FRHIBufferCreateDesc BufferDesc(
						TEXT("UCUnrealOverlayVB"),
						(uint32)Batch.VertexData.Num(),
						Batch.VertexStride,
						BUF_Volatile);
					FBufferRHIRef VB = RHICmdList.CreateBuffer(BufferDesc);
					void* Dest = RHICmdList.LockBuffer(VB.GetReference(), 0, (uint32)Batch.VertexData.Num(), RLM_WriteOnly);
					if (Dest)
					{
						FMemory::Memcpy(Dest, Batch.VertexData.GetData(), Batch.VertexData.Num());
						RHICmdList.UnlockBuffer(VB.GetReference());
					}

					RHICmdList.SetStreamSource(0, VB, 0);
					RHICmdList.DrawPrimitive(0, Batch.VertexCount / 3, 1);
				}
			});
	}
};

static TSharedPtr<ICustomSlateElement, ESPMode::ThreadSafe> GUCUnrealOverlayDrawer;

// ------------------------------------------------------------
// Minimal overlay widget (placeholder)
// Later: replace content with your engine UI host widget.
// ------------------------------------------------------------
class SKartGameGlobalOverlayWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SKartGameGlobalOverlayWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		ChildSlot
		[
			SNew(SBorder)
			.Visibility(EVisibility::Visible)  // Changed to Visible to receive mouse events
			.BorderBackgroundColor(FLinearColor(0, 0, 0, 0))
			.Padding(0)
			[
				// Keep empty by default. Uncomment to see it's installed.
				// SNew(STextBlock).Text(FText::FromString(TEXT("KartGame Global Overlay")))
				SNew(STextBlock).Text(FText::GetEmpty())
			]
		];
	}

	// Optional: access to device pointer (for future rendering hook)
	UC3D::UCUnreal3DDevice* GetDevice() const
	{
		return FKartGameGlobalOverlay::GetDevice();
	}

	virtual int32 OnPaint(const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		if (!GUCUnrealOverlayDrawer.IsValid())
		{
			GUCUnrealOverlayDrawer = MakeShared<FUCUnrealOverlayDrawer, ESPMode::ThreadSafe>();
		}

		FSlateDrawElement::MakeCustom(OutDrawElements, LayerId, GUCUnrealOverlayDrawer);
		return LayerId;
	}

	// Mouse event handlers
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		// Get mouse position in screen space
		FVector2D ScreenPosition = MouseEvent.GetScreenSpacePosition();
		FVector2D WidgetAbsolutePos = MyGeometry.GetAbsolutePosition();
		// Get mouse position in local widget space
		FVector2D LocalPosition = ScreenPosition - WidgetAbsolutePos;
		
		// 获取视口大小
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		// Forward to UC system
		if (UCUnrealWnd* UnrealWnd = FKartGameGlobalOverlay::GetUnrealWnd())
		{
			ucINT Button = BUTTON_NULL;
			if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
				Button = BUTTON_LEFT;
			else if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
				Button = BUTTON_RIGHT;
			else if (MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton))
				Button = BUTTON_MIDDLE;

			if (Button != BUTTON_NULL)
			{
				UnrealWnd->ForwardMouseEvent(UC_TOUCH_DOWN, Button, 
					(ucINT)LocalPosition.X, (ucINT)LocalPosition.Y, 1);
			}
		}

		// Call delegate if set
		FKartGameGlobalOverlay::ExecuteMouseButtonDown(MouseEvent);
		return FReply::Handled();
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		FVector2D ScreenPosition = MouseEvent.GetScreenSpacePosition();
		FVector2D WidgetAbsolutePos = MyGeometry.GetAbsolutePosition();
		// Get mouse position in local widget space
		FVector2D LocalPosition = ScreenPosition - WidgetAbsolutePos;
		
		// Forward to UC system
		if (UCUnrealWnd* UnrealWnd = FKartGameGlobalOverlay::GetUnrealWnd())
		{
			ucINT Button = BUTTON_NULL;
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
				Button = BUTTON_LEFT;
			else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
				Button = BUTTON_RIGHT;
			else if (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
				Button = BUTTON_MIDDLE;

			if (Button != BUTTON_NULL)
			{
				UnrealWnd->ForwardMouseEvent(UC_TOUCH_UP, Button, 
					(ucINT)LocalPosition.X, (ucINT)LocalPosition.Y, 1);
			}
		}

		FKartGameGlobalOverlay::ExecuteMouseButtonUp(MouseEvent);
		return FReply::Handled();
	}

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		FVector2D ScreenPosition = MouseEvent.GetScreenSpacePosition();
		FVector2D WidgetAbsolutePos = MyGeometry.GetAbsolutePosition();
		// Get mouse position in local widget space
		FVector2D LocalPosition = ScreenPosition - WidgetAbsolutePos;

		FVector2D Delta = MouseEvent.GetCursorDelta();

		if (UCUnrealWnd* UnrealWnd = FKartGameGlobalOverlay::GetUnrealWnd())
		{
			UnrealWnd->ForwardMouseEvent(UC_TOUCH_MOVE, BUTTON_NULL, 
				(ucINT)LocalPosition.X, (ucINT)LocalPosition.Y, 1);
		}

		FKartGameGlobalOverlay::ExecuteMouseMove(MouseEvent);
		return FReply::Unhandled();
	}

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		FVector2D ScreenPosition = MouseEvent.GetScreenSpacePosition();
		FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		
		// Forward to UC system (double click as DOWN with TapCount=2)
		if (UCUnrealWnd* UnrealWnd = FKartGameGlobalOverlay::GetUnrealWnd())
		{
			ucINT Button = BUTTON_NULL;
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
				Button = BUTTON_LEFT;
			else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
				Button = BUTTON_RIGHT;
			else if (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
				Button = BUTTON_MIDDLE;

			if (Button != BUTTON_NULL)
			{
				UnrealWnd->ForwardMouseEvent(UC_TOUCH_DOWN, Button, 
					(ucINT)LocalPosition.X, (ucINT)LocalPosition.Y, 2);
			}
		}

		if (FKartGameGlobalOverlay::ExecuteMouseButtonDoubleClick(MouseEvent))
		{
			return FReply::Handled();
		}

		return FReply::Unhandled();
	}

	// Override to support mouse cursor visibility
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override
	{
		// Return default cursor, or customize if needed
		return FCursorReply::Unhandled();
	}

	// Keyboard event handlers
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		// Forward to UC system
		if (UCUnrealWnd* UnrealWnd = FKartGameGlobalOverlay::GetUnrealWnd())
		{
			// Get key code directly from FKeyEvent
			ucWCHAR KeyCode = (ucWCHAR)InKeyEvent.GetKeyCode();
			UnrealWnd->ForwardKeyEvent(UC_KEYDOWN, KeyCode);
		}

		return FReply::Unhandled();
	}

	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		// Forward to UC system
		if (UCUnrealWnd* UnrealWnd = FKartGameGlobalOverlay::GetUnrealWnd())
		{
			// Get key code directly from FKeyEvent
			ucWCHAR KeyCode = (ucWCHAR)InKeyEvent.GetKeyCode();
			UnrealWnd->ForwardKeyEvent(UC_KEYUP, KeyCode);
		}

		return FReply::Unhandled();
	}

	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) override
	{
		// Forward to UC system
		if (UCUnrealWnd* UnrealWnd = FKartGameGlobalOverlay::GetUnrealWnd())
		{
			TCHAR Char = InCharacterEvent.GetCharacter();
			UnrealWnd->ForwardKeyEvent(UC_CHAR, (ucWCHAR)Char);
		}

		return FReply::Unhandled();
	}
};

bool FKartGameGlobalOverlay::bInstalled = false;
TSharedPtr<SWidget> FKartGameGlobalOverlay::OverlayWidget;
FTSTicker::FDelegateHandle FKartGameGlobalOverlay::TickerHandle;
TAtomic<UC3D::UCUnreal3DDevice*> FKartGameGlobalOverlay::DevicePtr{ nullptr };
TAtomic<UCUnrealWnd*> FKartGameGlobalOverlay::UnrealWndPtr{ nullptr };
FRWLock FKartGameGlobalOverlay::BatchesLock;
TArray<FKartGameGlobalOverlay::FUIBatch> FKartGameGlobalOverlay::Batches;

// Mouse event delegates
FOnOverlayMouseEvent FKartGameGlobalOverlay::OnMouseButtonDownDelegate;
FOnOverlayMouseEvent FKartGameGlobalOverlay::OnMouseButtonUpDelegate;
FOnOverlayMouseEvent FKartGameGlobalOverlay::OnMouseMoveDelegate;
FOnOverlayMouseEvent FKartGameGlobalOverlay::OnMouseButtonDoubleClickDelegate;

void FKartGameGlobalOverlay::SetDevice(UC3D::UCUnreal3DDevice* InDevice)
{
	DevicePtr.Store(InDevice);
}

UC3D::UCUnreal3DDevice* FKartGameGlobalOverlay::GetDevice()
{
	return DevicePtr.Load();
}

void FKartGameGlobalOverlay::SetUnrealWnd(UCUnrealWnd* InWnd)
{
	UnrealWndPtr.Store(InWnd);
}

UCUnrealWnd* FKartGameGlobalOverlay::GetUnrealWnd()
{
	return UnrealWndPtr.Load();
}

void FKartGameGlobalOverlay::BeginFrame()
{
	FWriteScopeLock Lock(BatchesLock);
	Batches.Reset();
}

void FKartGameGlobalOverlay::AppendBatch(FUIBatch&& Batch)
{
	if (Batch.VertexData.Num() == 0 || Batch.VertexStride == 0 || Batch.VertexCount == 0)
	{
		return;
	}
	FWriteScopeLock Lock(BatchesLock);
	Batches.Add(MoveTemp(Batch));
}

TArray<FKartGameGlobalOverlay::FUIBatch> FKartGameGlobalOverlay::GetBatchesSnapshot()
{
	FReadScopeLock Lock(BatchesLock);
	return Batches;
}

void FKartGameGlobalOverlay::Startup()
{
	if (TickerHandle.IsValid())
	{
		return;
	}

	// Poll until Slate + GameViewport are ready.
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateStatic(&FKartGameGlobalOverlay::Tick),
		0.0f);
}

void FKartGameGlobalOverlay::Shutdown()
{
	TryRemove();

	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
}

bool FKartGameGlobalOverlay::Tick(float DeltaTime)
{
	TryInstall();
	return true; // keep ticking (safe; does nothing after install)
}

void FKartGameGlobalOverlay::TryInstall()
{
	if (bInstalled)
	{
		return;
	}

	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	if (GEngine == nullptr || GEngine->GameViewport == nullptr)
	{
		return;
	}

	OverlayWidget = SNew(SKartGameGlobalOverlayWidget);
	GEngine->GameViewport->AddViewportWidgetContent(OverlayWidget.ToSharedRef(), /*ZOrder*/ INT32_MAX);
	bInstalled = true;
}

void FKartGameGlobalOverlay::TryRemove()
{
	if (!bInstalled)
	{
		return;
	}

	if (GEngine && GEngine->GameViewport && OverlayWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(OverlayWidget.ToSharedRef());
	}

	OverlayWidget.Reset();
	bInstalled = false;
}

void FKartGameGlobalOverlay::SetOnMouseButtonDown(FOnOverlayMouseEvent InDelegate)
{
	OnMouseButtonDownDelegate = InDelegate;
}

void FKartGameGlobalOverlay::SetOnMouseButtonUp(FOnOverlayMouseEvent InDelegate)
{
	OnMouseButtonUpDelegate = InDelegate;
}

void FKartGameGlobalOverlay::SetOnMouseMove(FOnOverlayMouseEvent InDelegate)
{
	OnMouseMoveDelegate = InDelegate;
}

void FKartGameGlobalOverlay::SetOnMouseButtonDoubleClick(FOnOverlayMouseEvent InDelegate)
{
	OnMouseButtonDoubleClickDelegate = InDelegate;
}

bool FKartGameGlobalOverlay::ExecuteMouseButtonDown(const FPointerEvent& MouseEvent)
{
	if (OnMouseButtonDownDelegate.IsBound())
	{
		return OnMouseButtonDownDelegate.Execute(MouseEvent);
	}
	return false;
}

bool FKartGameGlobalOverlay::ExecuteMouseButtonUp(const FPointerEvent& MouseEvent)
{
	if (OnMouseButtonUpDelegate.IsBound())
	{
		return OnMouseButtonUpDelegate.Execute(MouseEvent);
	}
	return false;
}

bool FKartGameGlobalOverlay::ExecuteMouseMove(const FPointerEvent& MouseEvent)
{
	if (OnMouseMoveDelegate.IsBound())
	{
		return OnMouseMoveDelegate.Execute(MouseEvent);
	}
	return false;
}

bool FKartGameGlobalOverlay::ExecuteMouseButtonDoubleClick(const FPointerEvent& MouseEvent)
{
	if (OnMouseButtonDoubleClickDelegate.IsBound())
	{
		return OnMouseButtonDoubleClickDelegate.Execute(MouseEvent);
	}
	return false;
}


