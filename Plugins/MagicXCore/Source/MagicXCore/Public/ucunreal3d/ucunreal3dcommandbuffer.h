/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dcommandbuffer.h
author:		Auto Generated

purpose:	UCUnreal3D Command Buffer
*********************************************************************/
#ifndef _UC_UCUNREAL3DCOMMANDBUFFER_H_
#define _UC_UCUNREAL3DCOMMANDBUFFER_H_

#include "basetype.h"

UC3D_BEGIN

// 前向声明
class UCUnreal3DDevice;

// ========================================
// UCUnreal3DCommandBuffer - Unreal Command Buffer 实现
// ========================================
class UCUnreal3DCommandBuffer : public UC3DModernCommandBuffer
{
	// Command Buffer 不需要 RTTI，因为它不在脚本系统中使用
	friend class UCUnreal3DDevice;

	UCUnreal3DCommandBuffer(UCUnreal3DDevice* pDevice);
	~UCUnreal3DCommandBuffer();

public:
	// ========== Command Buffer 生命周期 ==========
	virtual ucHRESULT Begin() override;
	virtual ucHRESULT End() override;
	virtual ucHRESULT Submit() override;
	virtual ucHRESULT Wait() override;
	virtual ucHRESULT Reset() override;

	// ========== 资源更新 ==========
	virtual ucHRESULT UpdateTexture(
		UC3DModernTexture* pTexture,
		ucUINT Level,
		ucCONST UCPoint& Offset,
		ucCONST UCSize& Size,
		ucCONST ucVOID* pData) override;

	virtual ucHRESULT UpdateVertexBuffer(
		UC3DModernVertexBuffer* pBuffer,
		ucUINT Offset,
		ucUINT Size,
		ucCONST ucVOID* pData) override;

	virtual ucHRESULT UpdateIndexBuffer(
		UC3DModernIndexBuffer* pBuffer,
		ucUINT Offset,
		ucUINT Size,
		ucCONST ucVOID* pData) override;

	// ========== 资源状态转换 ==========
	virtual ucHRESULT TransitionTexture(
		UC3DModernTexture* pTexture,
		UC3DRESOURCE_STATE NewState) override;

	virtual ucHRESULT TransitionSurface(
		UC3DModernSurface* pSurface,
		UC3DRESOURCE_STATE NewState) override;

	// ========== 渲染状态设置 ==========
	virtual ucHRESULT SetPipelineState(UC3DModernPipelineState* pPipelineState) override;
	virtual ucHRESULT SetVertexBuffer(UC3DModernVertexBuffer* pBuffer, ucUINT Slot) override;
	virtual ucHRESULT SetIndexBuffer(UC3DModernIndexBuffer* pBuffer) override;
	virtual ucHRESULT SetRenderTargets(
		ucUINT NumRenderTargets,
		UC3DModernSurface** ppRenderTargets,
		UC3DModernSurface* pDepthStencil) override;

	// ========== 绘制命令 ==========
	virtual ucHRESULT DrawIndexed(
		ucUINT IndexCount,
		ucUINT InstanceCount,
		ucINT BaseVertex) override;

	virtual ucHRESULT Draw(
		ucUINT VertexCount,
		ucUINT InstanceCount) override;

	// ========== 清除命令 ==========
	virtual ucHRESULT ClearRenderTarget(
		UC3DModernSurface* pRenderTarget,
		ucCONST ucFLOAT* pColor) override;

	virtual ucHRESULT ClearDepthStencil(
		UC3DModernSurface* pDepthStencil,
		ucBOOL bClearDepth,
		ucFLOAT Depth,
		ucBOOL bClearStencil,
		ucDWORD Stencil) override;

	// ========== 视口和裁剪 ==========
	virtual ucHRESULT SetViewport(
		ucFLOAT X, ucFLOAT Y, ucFLOAT Width, ucFLOAT Height,
		ucFLOAT MinDepth, ucFLOAT MaxDepth) override;

	virtual ucHRESULT SetScissorRect(
		ucINT Left, ucINT Top, ucINT Right, ucINT Bottom) override;

	// ========== 状态查询 ==========
	ucBOOL IsRecording() const { return m_bRecording; }

private:
	UCUnreal3DDevice* m_pDevice;
	ucBOOL m_bRecording;
	ucBOOL m_bSubmitted;
};

UC3D_END

#endif //_UC_UCUNREAL3DCOMMANDBUFFER_H_


