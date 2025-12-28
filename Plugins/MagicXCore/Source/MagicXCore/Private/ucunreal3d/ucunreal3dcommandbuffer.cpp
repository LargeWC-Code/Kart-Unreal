/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dcommandbuffer.cpp
author:		Auto Generated

purpose:	UCUnreal3D Command Buffer 实现
*********************************************************************/

#include "ucunreal3d/ucunreal3dcommandbuffer.h"
#include "ucunreal3d/ucunreal3ddevice.h"
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RenderingThread.h"
#endif

#define new UCNEW

UC3D_BEGIN

// Command Buffer 不需要 RTTI，因为它不在脚本系统中使用

UCUnreal3DCommandBuffer::UCUnreal3DCommandBuffer(UCUnreal3DDevice* pDevice)
	: m_pDevice(pDevice)
	, m_bRecording(ucFALSE)
	, m_bSubmitted(ucFALSE)
{
}

UCUnreal3DCommandBuffer::~UCUnreal3DCommandBuffer()
{
	// 确保命令已提交和等待
	if (m_bRecording)
	{
		End();
	}
	if (m_bSubmitted)
	{
		Wait();
	}
}

ucHRESULT UCUnreal3DCommandBuffer::Begin()
{
	if (m_bRecording)
		return ucE_FAIL; // 已经在录制中

	m_bRecording = ucTRUE;
	m_bSubmitted = ucFALSE;
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::End()
{
	if (!m_bRecording)
		return ucE_FAIL; // 未在录制中

	m_bRecording = ucFALSE;
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::Submit()
{
	if (m_bRecording)
		return ucE_FAIL; // 必须先调用 End()

	if (m_bSubmitted)
		return ucE_FAIL; // 已经提交过了

	m_bSubmitted = ucTRUE;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 在 Unreal 中，所有通过 ENQUEUE_RENDER_COMMAND 的命令都会自动执行
	// Submit 时刷新渲染命令队列
	FlushRenderingCommands();
#endif

	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::Wait()
{
	if (!m_bSubmitted)
		return ucE_FAIL; // 必须先调用 Submit()

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 等待所有渲染命令完成
	FlushRenderingCommands();
#endif

	m_bSubmitted = ucFALSE;
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::Reset()
{
	// 重置命令缓冲区以便重用
	if (m_bRecording)
		End();
	if (m_bSubmitted)
		Wait();
	m_bRecording = ucFALSE;
	m_bSubmitted = ucFALSE;
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::UpdateTexture(
	UC3DModernTexture* pTexture,
	ucUINT Level,
	ucCONST UCPoint& Offset,
	ucCONST UCSize& Size,
	ucCONST ucVOID* pData)
{
	if (pTexture == nullptr || pData == nullptr)
		return ucE_FAIL;
	
	// 调用纹理的 UpdateRegion 方法
	return pTexture->UpdateRegion(this, Level, Offset, Size, pData);
}

ucHRESULT UCUnreal3DCommandBuffer::UpdateVertexBuffer(
	UC3DModernVertexBuffer* pBuffer,
	ucUINT Offset,
	ucUINT Size,
	ucCONST ucVOID* pData)
{
	if (pBuffer == nullptr || pData == nullptr)
		return ucE_FAIL;
	
	// 调用缓冲区的 Update 方法
	return pBuffer->Update(this, Offset, Size, pData);
}

ucHRESULT UCUnreal3DCommandBuffer::UpdateIndexBuffer(
	UC3DModernIndexBuffer* pBuffer,
	ucUINT Offset,
	ucUINT Size,
	ucCONST ucVOID* pData)
{
	if (pBuffer == nullptr || pData == nullptr)
		return ucE_FAIL;
	
	// 调用缓冲区的 Update 方法
	return pBuffer->Update(this, Offset, Size, pData);
}

ucHRESULT UCUnreal3DCommandBuffer::TransitionTexture(
	UC3DModernTexture* pTexture,
	UC3DRESOURCE_STATE NewState)
{
	if (pTexture == nullptr)
		return ucE_FAIL;
	
	return pTexture->TransitionTo(this, NewState);
}

ucHRESULT UCUnreal3DCommandBuffer::TransitionSurface(
	UC3DModernSurface* pSurface,
	UC3DRESOURCE_STATE NewState)
{
	if (pSurface == nullptr)
		return ucE_FAIL;
	
	// TODO: 实现 Surface 状态转换
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::SetPipelineState(UC3DModernPipelineState* pPipelineState)
{
	// TODO: 实现 Pipeline State 设置
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::SetVertexBuffer(UC3DModernVertexBuffer* pBuffer, ucUINT Slot)
{
	// TODO: 实现 Vertex Buffer 设置
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::SetIndexBuffer(UC3DModernIndexBuffer* pBuffer)
{
	// TODO: 实现 Index Buffer 设置
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::SetRenderTargets(
	ucUINT NumRenderTargets,
	UC3DModernSurface** ppRenderTargets,
	UC3DModernSurface* pDepthStencil)
{
	// TODO: 实现 Render Target 设置
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::DrawIndexed(
	ucUINT IndexCount,
	ucUINT InstanceCount,
	ucINT BaseVertex)
{
	// TODO: 实现索引绘制
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::Draw(
	ucUINT VertexCount,
	ucUINT InstanceCount)
{
	// TODO: 实现非索引绘制
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::ClearRenderTarget(
	UC3DModernSurface* pRenderTarget,
	ucCONST ucFLOAT* pColor)
{
	// TODO: 实现 Render Target 清除
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::ClearDepthStencil(
	UC3DModernSurface* pDepthStencil,
	ucBOOL bClearDepth,
	ucFLOAT Depth,
	ucBOOL bClearStencil,
	ucDWORD Stencil)
{
	// TODO: 实现 Depth Stencil 清除
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::SetViewport(
	ucFLOAT X, ucFLOAT Y, ucFLOAT Width, ucFLOAT Height,
	ucFLOAT MinDepth, ucFLOAT MaxDepth)
{
	// TODO: 实现视口设置
	return ucS_OK;
}

ucHRESULT UCUnreal3DCommandBuffer::SetScissorRect(
	ucINT Left, ucINT Top, ucINT Right, ucINT Bottom)
{
	// TODO: 实现裁剪矩形设置
	return ucS_OK;
}

UC3D_END


