/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3ddevice.h
author:		Auto Generated

purpose:	UCUnreal3D Device - Unreal Engine 实现
*********************************************************************/
#ifndef _UC_UCUNREAL3DDEVICE_H_
#define _UC_UCUNREAL3DDEVICE_H_

#include "MagicXCore.h"
#include "basetype.h"

UC3D_BEGIN

// 前向声明
class UCUnreal3DTexture;
class UCUnreal3DVertexBuffer;
class UCUnreal3DPlaneFont;
class UCUnreal3DIndexBuffer;
class UCUnreal3DStagingBuffer;
class UCUnreal3DSurface;
class UCUnreal3DEffect;
class UCUnreal3DCommandBuffer;
class UCUnreal3DPipelineState;
class UCUnreal3DPipelineStateCache;

// 前向声明 UC3D 类型
class UC3DPlaneFont;
struct UC3DCreateFont;

// ========================================
// UCUnreal3DService
// ========================================

MAGICXCORE_API ucHRESULT CreateUCUnreal3DService(ucHANDLE hDLL, UC3DService** pp3D);

class UCUnreal3DService : public UC3DService
{
	RTTI_DECLARE;
public:
	UCUnreal3DService();
	~UCUnreal3DService();

	virtual ucHRESULT Create3DDevice(
		UC3DDevice**,
		ucHANDLE hWnd,
		ucUINT AdapterToUse,
		ucBOOL bWindow,
		ucINT nWidth,
		ucINT nHeight,
		ucINT nScreenWidth,
		ucINT nScreenHeight,
		ucBOOL bFSA = ucFALSE,
		ucDWORD dwDSFormat = UC3DFMT_D16) override;

	virtual ucINT GetAdapterCount() override;
	UC3DServiceType GetServiceType() const override;
};

// ========================================
// UCUnreal3DDevice
// ========================================

class UCUnreal3DDevice : public UC3DModernDevice
{
	RTTI_DECLARE;
	friend class UCUnreal3DService;
	friend class UCUnreal3DTexture;
	friend class UCUnreal3DSurface;
	friend class UCUnreal3DCommandBuffer;
	friend class UCUnreal3DStagingBuffer;
	friend class UCUnreal3DIndexBuffer;
	friend class UCUnreal3DVertexBuffer;
	friend class UCUnreal3DEffect;
	friend class UCUnreal3DPipelineState;
	friend class UCUnreal3DPipelineStateCache;

	UCUnreal3DDevice();
	~UCUnreal3DDevice();

public:
	// 获取渲染器名称
	virtual ucLPCTSTR GetRendererName() const override;

	// ========== 创建和释放 ==========
	virtual ucHRESULT Create(
		ucHANDLE hWnd,
		ucUINT AdapterToUse,
		ucBOOL bWindow,
		ucINT nWidth,
		ucINT nHeight,
		ucINT nScreenWidth,
		ucINT nScreenHeight,
		ucBOOL bFSA = ucFALSE,
		ucDWORD dwDSFormat = UC3DFMT_D16);

	// ========== 命令缓冲管理 ==========
	virtual UC3DModernCommandBuffer* CreateCommandBuffer() override;
	virtual ucVOID DestroyCommandBuffer(UC3DModernCommandBuffer* pCmdBuffer) override;
	virtual UC3DModernCommandBuffer* GetCurrentCommandBuffer() override;

	// 获取 Modern 设备数据（具体类型）
	// GetModernData() 由基类实现，不需要重写
	virtual UC3DDeviceDataBase::UC3DDataMng_UI* GetUIData() override;

	// ========== 窗口尺寸访问 ==========
	virtual ucUINT GetScreenWidth() override;
	virtual ucUINT GetScreenHeight() override;
	virtual ucUINT GetBackBufferWidth() override;
	virtual ucUINT GetBackBufferHeight() override;

	// ========== UI 缩放和字体管理 ==========
	virtual ucVOID SetDrawScale(ucFLOAT fScale) override;
	virtual ucVOID ResetFontData() override;

	// ========== 资源管理 ==========
	virtual ucHRESULT CreateTexture(
		UC3DModernTexture** ppTexture,
		UC3DPublicDataPtr*,
		ucUINT Width,
		ucUINT Height,
		ucUINT Levels,
		ucDWORD Usage,
		ucDWORD Format,
		ucDWORD Pool = UC3DPOOL_MANAGED,
		ucINT iMultiSamples = 0,
		ucBOOL bLinear = 1) override;

	virtual ucHRESULT CreateTextureFromFile(
		UC3DModernTexture** ppTexture,
		UC3DPublicDataPtr*,
		ucLPCTSTR pSrcFile,
		UCPackFileInfo* pPackFileInfo = ucNULL,
		ucBOOL bUsedThread = ucTRUE) override;

	virtual ucHRESULT CreateTextureFromMemory(
		UC3DModernTexture** ppTexture,
		UC3DPublicDataPtr*,
		ucLPVOID pSrcMem,
		ucINT iLength,
		ucUINT Width = 0,
		ucUINT Height = 0,
		ucUINT MipLevels = 0,
		ucDWORD Usage = 0,
		ucDWORD Format = UC3DFMT_UNKNOWN,
		ucDWORD Pool = UC3DPOOL_MANAGED,
		ucDWORD Filter = UC3D_DEFAULT,
		ucDWORD MipFilter = UC3D_DEFAULT,
		UCCreateTextureArgsExt* pTextureArgsExt = ucNULL,
		UC3DCOLOR ColorKey = 0,
		UC3DIMAGE_INFO* pSrcInfo = ucNULL) override;

	virtual ucHRESULT CreateDepthStencilSurface(
		UC3DModernSurface** ppSurface,
		UC3DPublicDataPtr*,
		ucUINT Width,
		ucUINT Height,
		ucDWORD Format,
		ucDWORD MultiSample,
		ucDWORD MultisampleQuality,
		ucBOOL Discard) override;

	virtual ucHRESULT CreateVertexBuffer(
		ucUINT Length,
		ucUINT Stride,
		ucDWORD Usage,
		ucCONST ucVOID* pInitialData,
		UC3DModernVertexBuffer** ppVertexBuffer,
		UC3DPublicDataPtr*) override;

	virtual ucHRESULT CreateIndexBuffer(
		ucUINT Length,
		ucDWORD Usage,
		UC3DFORMAT Format,
		ucCONST ucVOID* pInitialData,
		UC3DModernIndexBuffer** ppIndexBuffer,
		UC3DPublicDataPtr*) override;

	// StagingBuffer
	virtual ucHRESULT CreateStagingBuffer(
		ucUINT Size,
		ucDWORD Usage,
		ucCONST ucVOID* pInitialData,
		UC3DModernBuffer** ppBuffer) override;

	// UniformBuffer
	virtual ucHRESULT CreateUniformBuffer(
		ucUINT Size,
		ucDWORD Usage,
		ucCONST ucVOID* pInitialData,
		UC3DModernBuffer** ppBuffer) override;

	// StorageBuffer
	virtual ucHRESULT CreateStorageBuffer(
		ucUINT Size,
		ucDWORD Usage,
		ucCONST ucVOID* pInitialData,
		UC3DModernBuffer** ppBuffer) override;

	// ========== Buffer 资源管理 ==========
	virtual ucVOID RegisterBuffer(UC3DModernBuffer* pBuffer) override;
	virtual ucVOID UnregisterBuffer(UC3DModernBuffer* pBuffer) override;
	virtual ucVOID TrackBufferUsage(UC3DModernBuffer* pBuffer) override;
	virtual ucVOID ReleaseAllBuffers() override;

	virtual ucHRESULT CreatePipelineState(
		ucCONST UC3DModernPipelineStateDesc& Desc,
		UC3DModernPipelineState** ppPipelineState) override;

	virtual ucHRESULT CreatePipelineStateFromEffect(
		UC3DModernEffect* pEffect,
		UC3DVertexDeclaration* pVertexDecl,
		ucDWORD FVF,
		UC3DPRIMITIVETYPE PrimitiveType,
		UC3DModernPipelineState** ppPipelineState) override;

	// ========== Modern UI 渲染接口 ==========
	// Unreal 后端：UI 渲染由 Slate Overlay 执行，这里不需要做清屏/填充，直接返回成功即可。
	virtual ucHRESULT ColorFill(
		ucCONST UCFRect& Rect,
		ucDWORD dwBack,
		ucCONST UCFRect& ClipRect) override;

	virtual ucHRESULT DrawRectsBatch(
		ucCONST UCFRect* pRects,
		ucUINT Count,
		ucDWORD dwBack[4],
		ucCONST UCFRect& ClipRect) override;

	// ========== 底层绘制接口（供高层使用，如字体渲染）==========
	virtual ucHRESULT DrawVerticesWithTexture(
		ucCONST UCFontVertex* pVertices,
		ucUINT VertexCount,
		UC3DModernTexture* pTexture,
		ucDWORD dwColor,
		ucFLOAT fZ = 0.0f) override;

	virtual ucVOID Draw9Patch(
		ucCONST UCFRect& Rect,
		UC3DModernTexture* pTexture,
		ucLPVOID ImageData,
		ucCONST uc3dxVector2& vScaling,
		ucCONST UCFPoint& Offset,
		ucCONST UCFPoint& Center,
		ucCONST UCFRect& ClipRect,
		ucDWORD bAlpha,
		ucCONST UCIntIntMap& mapU,
		ucCONST UCIntIntMap& mapV,
		ucFLOAT fZ = 0.0f,
		ucBYTE iMirror = 0x00) override;

	// ========== Pipeline State ==========
	virtual ucHRESULT SetBlendState(const UC3DModernBlendStateDesc& desc) override;
	virtual ucHRESULT SetDepthStencilState(const UC3DModernDepthStencilStateDesc& desc) override;
	virtual ucHRESULT SetRasterizerState(const UC3DModernRasterizerStateDesc& desc) override;

	// ========== 视口和裁剪 ==========
	virtual ucHRESULT SetViewport(const UC3DModernViewportDesc& viewport) override;
	virtual ucHRESULT GetViewport(UC3DModernViewportDesc& viewport) override;
	virtual ucHRESULT SetScissorRect(ucCONST UCFRect& rect) override;
	virtual ucHRESULT GetScissorRect(UCFRect& rect) override;

	// ========== 渲染目标 ==========
	virtual ucHRESULT SetRenderTarget(ucDWORD RenderTargetIndex, UC3DModernSurface* pRenderTarget) override;
	virtual ucHRESULT GetRenderTarget(ucDWORD RenderTargetIndex, UC3DModernSurface** ppRenderTarget, UC3DPublicDataPtr* pPublicDataPtr) override;
	virtual ucHRESULT SetDepthStencilSurface(UC3DModernSurface* pNewZStencil) override;
	virtual ucHRESULT GetDepthStencilSurface(UC3DModernSurface** ppZStencilSurface, UC3DPublicDataPtr* pPublicDataPtr) override;

	// ========== 绘制命令 ==========
	virtual ucHRESULT DrawPrimitive(
		UC3DPRIMITIVETYPE PrimitiveType,
		ucUINT StartVertex,
		ucUINT PrimitiveCount);

	virtual ucHRESULT DrawIndexedPrimitive(
		UC3DPRIMITIVETYPE Type,
		ucINT BaseVertexIndex,
		ucUINT MinIndex,
		ucUINT NumVertices,
		ucUINT StartIndex,
		ucUINT PrimitiveCount);

	// ========== 场景管理 ==========
	virtual ucHRESULT BeginScene() override;
	virtual ucHRESULT EndScene() override;
	virtual ucHRESULT Flush() override;
	virtual ucHRESULT Present(
		ucCONST RECT* pSourceRect,
		ucCONST RECT* pDestRect,
		ucHANDLE hDestWindowOverride,
		ucCONST ucRGNDATA* pDirtyRegion) override;
	virtual ucHRESULT Clear(
		ucDWORD Flags,
		ucDWORD Color,
		ucFLOAT Z,
		ucDWORD Stencil) override;

	// ========== 设备能力 ==========
	virtual ucVOID TestDeviceCapability() override;
	virtual UCString GetErrorDescription(ucLONG error) override;

	// ========== 重置 ==========
	virtual ucHRESULT Reset(
		ucUINT nWindowWidth,
		ucUINT nWindowHeight,
		ucUINT nScreenWidth,
		ucUINT nScreenHeight,
		ucBOOL bFullScreen,
		ucBOOL bFSA = ucFALSE) override;

	// ========== 字体 ==========
	virtual ucHRESULT CreateDefFont(
		UC3DPlaneFont**,
		UC3DPublicDataPtr*,
		ucCONST UC3DCreateFont&);

	virtual ucHRESULT CreateFont(
		UC3DPlaneFont** ppFont,
		ucCONST UC3DCreateFont& createInfo) override;

	virtual ucHRESULT CreateEntityEmpty(UC3DEntity** ppEntity) override;

	virtual ucHRESULT CreateEffectFromFile(
		ucLPCTSTR pSrcFile,
		ucCONST UC3DMACRO* pDefines,
		ucDWORD Flags,
		UC3DModernEffect** ppEffect,
		UC3DPublicDataPtr* pEffectData,
		UC3DModernBuffer** ppCompilationErrors) override;

	ucFLOAT GetCharHeight(
		UC3DPlaneFont* pFont,
		ucINT iHeight,
		ucINT iOutlineSize);

	ucFLOAT GetCharWidth(
		UC3DPlaneFont* pFont,
		ucINT iWidth,
		ucINT iOutlineSize,
		ucTCHAR szChar);

protected:
	// ========== 共享 Buffer 管理（实现基类接口）==========
	virtual ucHRESULT GetSharedStagingBuffer(
		ucUINT DataSize,
		ucUINT Alignment,
		UC3DModernBuffer** ppStagingBuffer,
		ucVOID** ppMappedData,
		ucUINT* pOffset);

	virtual ucHRESULT GetSharedVertexBuffer(
		ucUINT DataSize,
		ucUINT Stride,
		ucUINT Alignment,
		UC3DModernVertexBuffer** ppVertexBuffer,
		ucUINT* pOffset);

	virtual ucVOID ResetSharedBufferOffsets();

private:
	// Unreal Engine 相关资源
	// 注意：这里先不定义具体的 Unreal 类型，后续挂接时再添加
	// class UWorld* m_pWorld;
	// class UTextureRenderTarget2D* m_pRenderTarget;
	
	// 窗口尺寸
	ucUINT m_WindowWidth;
	ucUINT m_WindowHeight;
	ucUINT m_ScreenWidth;
	ucUINT m_ScreenHeight;
	ucBOOL m_bFullScreen;
	
	// Command Buffer 管理（双缓冲）
	static const ucINT MAX_FRAMES_IN_FLIGHT = 2;
	struct FrameInFlight
	{
		UC3DModernCommandBuffer* CommandBuffer;  // 使用基类指针，实际类型为 UCUnreal3DCommandBuffer*
		// Unreal 相关的同步对象
		// FGPUFence* FrameFence;
		// FRenderCommandFence* RenderFence;
		UCSimpleMap<UC3DModernBuffer*, UC3DModernBuffer*> PendingRelease;
		ucUINT StagingBufferOffset;
		ucUINT VertexBufferOffset;
	};
	FrameInFlight m_Frames[MAX_FRAMES_IN_FLIGHT];
	ucINT m_CurrentFrameIndex;
	
	// 预分配的大 Staging Buffer（使用基类指针，避免前向声明问题）
	UC3DModernBuffer* m_pSharedStagingBuffer;
	static const ucUINT SHARED_STAGING_BUFFER_SIZE = 2 * 1024 * 1024;  // 2MB
	ucVOID* m_pSharedStagingBufferMappedData;
	
	// 预分配的大 Vertex Buffer（使用基类指针，避免前向声明问题）
	UC3DModernVertexBuffer* m_pSharedVertexBuffer;
	static const ucUINT SHARED_VERTEX_BUFFER_SIZE = 2 * 1024 * 1024;  // 2MB
	
	// 当前状态
	UC3DModernBlendStateDesc m_CurrentBlendState;
	UC3DModernDepthStencilStateDesc m_CurrentDepthStencilState;
	UC3DModernRasterizerStateDesc m_CurrentRasterizerState;
	UC3DModernViewportDesc m_CurrentViewport;
	UCFRect m_CurrentScissorRect;

	// 当前绑定的资源
	UCUnreal3DTexture* m_BoundTextures[16];
	UCUnreal3DVertexBuffer* m_BoundVertexBuffer;
	UCUnreal3DIndexBuffer* m_BoundIndexBuffer;

	// Buffer 资源管理
	UCSimpleMap<UC3DModernBuffer*, UC3DModernBuffer*> m_Buffers;

	// UI 数据管理
	UC3DDeviceDataBase::UC3DDataMng_UI m_UIData;
	
	// PipelineState 缓存管理器
	UC3DModernPipelineStateCache* m_pPipelineStateCache;
	
	// 初始化函数
	ucHRESULT InitializeUnrealResources(ucHANDLE hWnd);
	ucVOID CleanupUnrealResources();
	
	// 辅助函数
	ucHRESULT ConvertFormat(UC3DFORMAT format, /* Unreal format type */ void* pOutFormat);
	UC3DFORMAT ConvertFormat(/* Unreal format type */ void* pFormat);
};

UC3D_END

// 导出函数声明
extern "C" {
	ucHRESULT ucCreate3DService(UC3D::UC3DService** pp3D);
}

SCRIPT_NAMESPACE_DECLARE(, UC3D, UCE_UCUnreal3DService, UCUnreal3DService, ucFALSE);
SCRIPT_NAMESPACE_DECLARE(, UC3D, UCE_UCUnreal3DDevice, UCUnreal3DDevice, ucFALSE);

#endif //_UC_UCUNREAL3DDEVICE_H_

