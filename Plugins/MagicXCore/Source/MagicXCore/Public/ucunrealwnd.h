/************************************************************************
created:		2024/12/XX
file:			ucunrealwnd.h
author:			Auto Generated

purpose:		UCUnrealWnd - Unreal Engine 窗口外壳接口
				仿照 UCMainWnd，提供内嵌引擎的外壳接口
************************************************************************/
#ifndef _UCUNREALWND_2024_12_XX
#define _UCUNREALWND_2024_12_XX

#include "MagicXCore.h"
#include "ucgamebase.h"

// ========================================
// UCWnd - 窗口通信结构（与 UCMainWnd 保持一致）
// ========================================
struct UCWnd
{
public:
	// 多线程使用，所以必须一个读，一个写，不能混用
	// 输入
	ucINT64	Input;
	// 输出
	ucINT64	Output;
	UCWnd();
};

SCRIPT_DECLARE(, UCE_UCWnd, UCWnd, ucFALSE);

UCWnd* UCGetCustomWnd();

// ========================================
// UCUnrealWnd - Unreal Engine 窗口外壳类
// ========================================
class MAGICXCORE_API UCUnrealWnd
{
public:
	UCWnd* Wnd;

public:
	UCUnrealWnd();
	virtual ~UCUnrealWnd();
	
	static ucBOOL GetUnrealViewportSize(ucINT& OutWidth, ucINT& OutHeight);
		
	// ========== 运行接口 ==========
	ucBOOL InitGame(ucHANDLE hWnd, UCSize Size, ucCONST UCString& strCmdLine);
	ucVOID ExitGame();

	// ========== 窗口管理 ==========
	// 设置窗口大小（父窗口大小变化时调用）
	ucVOID SetSize(ucINT cx, ucINT cy);
	
	// 窗口是否有效
	ucBOOL IsValid() { return InterlockedExchange(&m_Valid, m_Valid); }
	
	// 设置为无效
	ucVOID SetUnValid() { InterlockedExchange(&m_Valid, 0); }
	
	// 关闭窗口
	ucVOID CloseWnd(ucINT* pClose);
	
	// ========== 消息处理 ==========
	// Peek 消息（非阻塞）
	ucVOID PeekMessage(MSG& msg);
	
	// Get 消息（阻塞）
	ucBOOL GetMessage(MSG& msg);
	
	// 处理窗口消息
	LRESULT RealWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	
	// ========== Unreal 事件转发 ==========
	// 转发鼠标事件到 UC 系统
	ucVOID ForwardMouseEvent(ucINT EventType, ucINT Button, ucINT X, ucINT Y, ucINT TapCount = 1);
	
	// 转发键盘事件到 UC 系统
	ucVOID ForwardKeyEvent(ucINT EventType, ucWCHAR Key);
	
	// ========== 编辑器支持 ==========
	ucVOID AddEditorInfo(UC3DEntityData* pEntityData, UC3DAnimationData* pAnimationData);
	ucVOID RemoveEditorInfo(UC3DEntityData* pEntityData, UC3DAnimationData* pAnimationData);
	
	// ========== 光标管理 ==========
	HCURSOR GetMyCursor(UCCursor* pCursor);

private:
	LONG			m_Valid;		// 窗口是否还有效
	UCScreenForm*	m_ScreenForm;	// 游戏UI
	UCVMService*	m_VMService;	// 虚拟机服务
	UC3DDevice*		m_3DDevice;		// 3D设备（Unreal后端）
	
	// 输入状态
	ucBOOL			m_bAlt;			// 是否Alt
	ucBOOL			m_bCtrl;		// 是否Ctrl
	UCPoint			m_pPos;			// 坐标
	ucINT			m_nLen;			// 长度
	
	// 协程/线程管理
	UCFiber			m_FiberRun;
	
	// 时间管理
	ucUINT64		m_dwLastActiveTime;
	
	// 窗口状态
	ucBOOL			m_bFirstResize;
	ucINT			m_nOutputCount;	// 输出计数器
	UCString		m_strDebug;		// 调试信息
	
	// Unreal 相关资源（后续挂接时添加）
	// class UWorld* m_pWorld;
	// class UGameInstance* m_pGameInstance;
	// class UTextureRenderTarget2D* m_pRenderTarget;
	
	// 事件处理
	ucVOID UC_PROC OnMainCurChange(UCObject*, UCEventArgs*);
	ucVOID UC_PROC OnMainVisible(UCObject*, UCEventArgs*);
	ucVOID UC_PROC OnMainSize(UCObject*, UCEventArgs*);
	ucVOID UC_PROC OnLocalDebugBreak(UCObject*, UCEventArgs*);
	ucVOID UC_PROC OnFiberRun(UCObject*, UCEventArgs*);
	ucVOID UC_PROC OnDebugOutput(UCObject*, UCEventArgs*);
};

#endif //_UCUNREALWND_2024_12_XX

