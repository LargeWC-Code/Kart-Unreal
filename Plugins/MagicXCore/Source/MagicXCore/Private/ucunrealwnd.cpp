/************************************************************************
created:		2024/12/XX
file:			ucunrealwnd.cpp
author:			Auto Generated

purpose:		UCUnrealWnd 实现
				仿照 UCMainWnd，提供内嵌引擎的外壳接口
************************************************************************/
#include "ucunrealwnd.h"

#define new UCNEW

SCRIPT_IMPLEMENT_BEGIN(UCE_UCWnd, UCWnd)
SCRIPT_CONSTRUCT_0()
SCRIPT_PROPERTY(ucCONST UCE_INT64, Input)
SCRIPT_PROPERTY(UCE_INT64, Output)
SCRIPT_DECONSTRUCT()
SCRIPT_IMPLEMENT_END(UCE_UCWnd)

// ========================================
// UCWnd 实现
// ========================================

UCWnd* UCGetCustomWnd()
{
	static SingletonThread<UCWnd> StaticWnd;
	return &(UCWnd&)StaticWnd;
}

SCRIPT_RFUNCTION_0(UCE_UCWnd*, UCGetCustomWnd);

UCWnd::UCWnd()
{
	Input = 0;
	Output = 0;
}

UCUnrealWnd::UCUnrealWnd() : m_FiberRun(FIBERPriority_IDLE, 0x200000)
{
	Wnd = ucNULL;

	m_bAlt = ucFALSE;
	m_bCtrl = ucFALSE;
	m_pPos = UCPoint(0, 0);
	m_nLen = 20;

	m_bFirstResize = ucTRUE;
	m_nOutputCount = 0;

	InterlockedExchange(&m_Valid, 1);
	m_ScreenForm = GetScreenForm();
	m_VMService = ucNULL;
	m_3DDevice = ucNULL;

	m_dwLastActiveTime = ucTimer::GetTime() - 10000;

	// 绑定事件
	if (m_ScreenForm)
	{
		if (m_ScreenForm->ControlStztic->Device)
		{
			m_ScreenForm->ControlStztic->Device->OnCursorChange += UCEvent(this, &UCUnrealWnd::OnMainCurChange);
		}
		m_ScreenForm->Visible.OnSet += UCEvent(this, &UCUnrealWnd::OnMainVisible);
		m_ScreenForm->OnWinSize += UCEvent(this, &UCUnrealWnd::OnMainSize);
	}

	m_FiberRun.FiberEvent = UCEvent(this, &UCUnrealWnd::OnFiberRun);
}

UCUnrealWnd::~UCUnrealWnd()
{
	InterlockedExchange(&m_Valid, 0);

	// 清理事件绑定
	if (m_ScreenForm)
	{
		if (m_ScreenForm->ControlStztic->Device)
		{
			m_ScreenForm->ControlStztic->Device->OnCursorChange -= UCEvent(this, &UCUnrealWnd::OnMainCurChange);
		}
		m_ScreenForm->Visible.OnSet -= UCEvent(this, &UCUnrealWnd::OnMainVisible);
		m_ScreenForm->OnWinSize -= UCEvent(this, &UCUnrealWnd::OnMainSize);
		SF_DELETE(m_ScreenForm);
	}

	SF_DELETE(m_VMService);
	m_3DDevice = ucNULL;  // 设备由系统管理，不在这里删除
}

ucBOOL UCUnrealWnd::InitGame(ucHANDLE Handle, UCSize Size, ucCONST UCString& strCmdLine)
{
	seed(ucTimer::GetTime());

	if (m_Valid == ucFALSE)
		return ucFALSE;

	m_ScreenForm = new UCScreenForm;
	if (!m_ScreenForm->Init(Handle, Size, ucNULL, ucTRUE))
	{
		m_Valid = ucFALSE;
		return ucFALSE;
	}

	if (m_ScreenForm->ControlStztic->Device)
		m_ScreenForm->ControlStztic->Device->OnCursorChange += UCEvent(this, &UCUnrealWnd::OnMainCurChange);
	m_ScreenForm->Visible.OnSet += UCEvent(this, &UCUnrealWnd::OnMainVisible);
	m_ScreenForm->OnWinSize += UCEvent(this, &UCUnrealWnd::OnMainSize);

	m_FiberRun.Run((UCEventArgs*)&strCmdLine);
	UCMainFiberData::GetMainFiberData()->OnDefaultDispatch(ucNULL, ucNULL);

	return m_Valid;
}

ucVOID UCUnrealWnd::ExitGame()
{
	m_ScreenForm->ExitSPL();
}

ucVOID UCUnrealWnd::SetSize(ucINT cx, ucINT cy)
{
	if (!IsValid() || !m_ScreenForm)
		return;

	// 通知 ScreenForm 窗口大小变化（不要在这里销毁 ScreenForm）
	m_ScreenForm->SetSize(UCSize(cx, cy));
}

ucVOID UCUnrealWnd::CloseWnd(ucINT* pClose)
{
	if (pClose)
		*pClose = 0;

	SetUnValid();

	// TODO: 清理资源并关闭窗口
}

ucVOID UCUnrealWnd::PeekMessage(MSG& msg)
{
	// TODO: 从 Unreal 消息队列中 Peek 消息
	// 在 Unreal 中，消息处理通常通过 Tick 或事件系统
}

ucBOOL UCUnrealWnd::GetMessage(MSG& msg)
{
	// TODO: 从 Unreal 消息队列中 Get 消息（阻塞）
	// 在 Unreal 中，通常使用 Tick 系统，不阻塞
	return ucFALSE;
}

LRESULT UCUnrealWnd::RealWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 处理窗口消息
	// 在 Unreal 中，消息处理通常通过 InputComponent 或 Widget 系统
	return 0;
}

ucVOID UCUnrealWnd::AddEditorInfo(UC3DEntityData* pEntityData, UC3DAnimationData* pAnimationData)
{
	// TODO: 添加编辑器信息
}

ucVOID UCUnrealWnd::RemoveEditorInfo(UC3DEntityData* pEntityData, UC3DAnimationData* pAnimationData)
{
	// TODO: 移除编辑器信息
}

HCURSOR UCUnrealWnd::GetMyCursor(UCCursor* pCursor)
{
	// TODO: 获取光标句柄
	// 在 Unreal 中，光标管理通过 Slate 系统
	return NULL;
}

// ========================================
// 事件处理函数
// ========================================

ucVOID UC_PROC UCUnrealWnd::OnFiberRun(UCObject* Object, UCEventArgs* e)
{
	UCTimeFiberData* FiberData = (UCTimeFiberData*)Object;
	UCString strName = *(UCString*)e;

	if (m_ScreenForm->RunGame(strName))
	{
		//运行失败
		SetUnValid();
		return;
	}

	if (!m_ScreenForm->LoadSPL())
	{
		//运行失败
		SetUnValid();
		return;
	}
}

ucVOID UC_PROC UCUnrealWnd::OnMainCurChange(UCObject* pSender, UCEventArgs* pArgs)
{
	// TODO: 处理光标变化事件
}

ucVOID UC_PROC UCUnrealWnd::OnMainVisible(UCObject* pSender, UCEventArgs* pArgs)
{
	// TODO: 处理可见性变化事件
}

ucVOID UC_PROC UCUnrealWnd::OnMainSize(UCObject* pSender, UCEventArgs* pArgs)
{
	// TODO: 处理窗口大小变化事件
}

ucVOID UC_PROC UCUnrealWnd::OnLocalDebugBreak(UCObject* pSender, UCEventArgs* pArgs)
{
	// TODO: 处理调试断点事件
}

ucVOID UC_PROC UCUnrealWnd::OnDebugOutput(UCObject* pSender, UCEventArgs* pArgs)
{
	// TODO: 处理调试输出事件
}

// ========================================
// 辅助函数实现
// ========================================

ucBOOL UCUnrealWnd::GetUnrealViewportSize(ucINT& OutWidth, ucINT& OutHeight)
{
	// TODO: 从 Unreal Engine 获取实际视口大小
	// 
	// 方法1: 通过 GEngine 获取（需要在游戏线程中调用）
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		OutWidth = (ucINT)ViewportSize.X;
		OutHeight = (ucINT)ViewportSize.Y;
		return (OutWidth > 0 && OutHeight > 0) ? ucTRUE : ucFALSE;
	}
	//
	// 方法2: 通过 FDisplayMetrics 获取主显示器分辨率
	// #include "GenericPlatform/GenericApplication.h"
	// FDisplayMetrics DisplayMetrics;
	// FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
	// OutWidth = DisplayMetrics.PrimaryDisplayWidth;
	// OutHeight = DisplayMetrics.PrimaryDisplayHeight;
	// return ucTRUE;
	//
	// 方法3: 通过 Slate 获取（需要在游戏线程中调用）
	// #include "Framework/Application/SlateApplication.h"
	// if (FSlateApplication::IsInitialized())
	// {
	//     FVector2D ViewportSize = FSlateApplication::Get().GetRenderer()->GetViewportSize();
	//     OutWidth = (ucINT)ViewportSize.X;
	//     OutHeight = (ucINT)ViewportSize.Y;
	//     return ucTRUE;
	// }
	//
	// 方法4: 通过 UGameplayStatics（需要在有 World 的情况下）
	// #include "Kismet/GameplayStatics.h"
	// if (UWorld* World = GEngine->GetWorldFromContextObject(...))
	// {
	//     int32 ViewportSizeX, ViewportSizeY;
	//     UGameplayStatics::GetViewportSize(World, ViewportSizeX, ViewportSizeY);
	//     OutWidth = ViewportSizeX;
	//     OutHeight = ViewportSizeY;
	//     return ucTRUE;
	// }

	// 暂时返回 false，使用默认值
	return ucFALSE;
}

ucVOID UCUnrealWnd::ForwardMouseEvent(ucINT EventType, ucINT Button, ucINT X, ucINT Y, ucINT TapCount)
{
	if (!m_ScreenForm || !IsValid())
		return;

	UCMouseEventArgs arg;
	arg.Button = Button;
	arg.TapCount = TapCount;
	arg.Position = UCPoint(X, Y);
	arg.FltPosition = UCFPoint((ucFLOAT)X, (ucFLOAT)Y);
	arg.AbsolutePos = arg.Position;
	arg.FltAbsolutePos = UCFPoint((ucFLOAT)X, (ucFLOAT)Y);
	arg.ID = 0; // 鼠标事件 ID 固定为 0

	m_ScreenForm->Run(EventType, &arg);
}

ucVOID UCUnrealWnd::ForwardKeyEvent(ucINT EventType, ucWCHAR Key)
{
	if (!m_ScreenForm || !IsValid())
		return;

	UCKeyEventArgs arg;
	arg.Key = Key;

	m_ScreenForm->Run(EventType, &arg);
}
