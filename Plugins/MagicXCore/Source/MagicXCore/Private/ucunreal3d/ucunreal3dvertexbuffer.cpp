/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dvertexbuffer.cpp
author:		Auto Generated

purpose:	UCUnreal3D VertexBuffer 实现
*********************************************************************/
#include "ucunreal3d/ucunreal3dvertexbuffer.h"
#include "ucunreal3d/ucunreal3ddevice.h"

// 注意：后续挂接 Unreal 时需要包含相关头文件
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RHI.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#endif

#define new UCNEW

UC3D_BEGIN

RTTI_IMPLEMENT(UCUnreal3DVertexBuffer, UCE_UCUnreal3DVertexBuffer);
SCRIPT_IMPLEMENT_BEGIN(UCE_UCUnreal3DVertexBuffer, UCUnreal3DVertexBuffer);
SCRIPT_IMPLEMENT_END(UCE_UCUnreal3DVertexBuffer);

UCUnreal3DVertexBuffer::UCUnreal3DVertexBuffer()
	: m_pDevice(ucNULL)
	, m_Size(0)
	, m_Stride(0)
	, m_Usage(0)
	, m_pMappedData(ucNULL)
	, m_bMapped(ucFALSE)
{
	DebugInfo = &UCE_UCUnreal3DVertexBuffer::I;
}

UCUnreal3DVertexBuffer::~UCUnreal3DVertexBuffer()
{
	if (m_pDevice)
		m_pDevice->UnregisterBuffer(this);

	if (m_bMapped)
	{
		Unmap(ucNULL);
	}

	// Unreal RHI Buffer 会自动释放（通过智能指针）
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	m_BufferRHI.SafeRelease();
#endif

	m_pDevice = ucNULL;
	m_Size = 0;
	m_Stride = 0;
	m_Usage = 0;
}

ucHRESULT UCUnreal3DVertexBuffer::Update(
	UC3DModernCommandBuffer* pCmdBuffer,
	ucUINT Offset,
	ucUINT Size,
	ucCONST ucVOID* pData)
{
	if (pData == nullptr || Size == 0 || !m_BufferRHI.IsValid())
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 在渲染线程中更新 Buffer
	ENQUEUE_RENDER_COMMAND(UpdateVertexBufferCommand)(
		[this, Offset, Size, pData](FRHICommandListImmediate& RHICmdList)
		{
			// 锁定 Buffer 进行更新
			void* pMappedData = RHICmdList.LockBuffer(
				m_BufferRHI.GetReference(),
				Offset,
				Size,
				RLM_WriteOnly
			);

			if (pMappedData != nullptr)
			{
				memcpy(pMappedData, pData, Size);
				RHICmdList.UnlockBuffer(m_BufferRHI.GetReference());
			}
		});

	// 等待渲染命令完成
	FlushRenderingCommands();

	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DVertexBuffer::Map(
	UC3DModernCommandBuffer* pCmdBuffer,
	ucUINT Offset,
	ucUINT Size,
	ucVOID** ppData)
{
	if (ppData == nullptr || !m_BufferRHI.IsValid())
		return ucE_FAIL;

	if (m_bMapped)
		return ucE_FAIL;  // 已经映射

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 在渲染线程中映射 Buffer
	ucHRESULT Result = ucE_FAIL;
	ENQUEUE_RENDER_COMMAND(MapVertexBufferCommand)(
		[this, Offset, Size, ppData, &Result](FRHICommandListImmediate& RHICmdList)
		{
			// 锁定 Buffer 进行映射
			void* pMappedData = RHICmdList.LockBuffer(
				m_BufferRHI.GetReference(),
				Offset,
				Size == 0 ? m_Size : Size,
				RLM_WriteOnly
			);

			if (pMappedData != nullptr)
			{
				m_pMappedData = pMappedData;
				m_bMapped = ucTRUE;
				*ppData = pMappedData;
				Result = ucS_OK;
			}
			else
			{
				Result = ucE_FAIL;
			}
		});

	// 等待渲染命令完成
	FlushRenderingCommands();

	return Result;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DVertexBuffer::Unmap(UC3DModernCommandBuffer* pCmdBuffer)
{
	if (!m_bMapped || !m_BufferRHI.IsValid())
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 在渲染线程中取消映射
	ENQUEUE_RENDER_COMMAND(UnmapVertexBufferCommand)(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			// 解锁 Buffer
			RHICmdList.UnlockBuffer(m_BufferRHI.GetReference());
		});

	// 等待渲染命令完成
	FlushRenderingCommands();

	m_pMappedData = ucNULL;
	m_bMapped = ucFALSE;
	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHANDLE UCUnreal3DVertexBuffer::GetRHIBufferHandle() const
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	return reinterpret_cast<ucHANDLE>(m_BufferRHI.GetReference());
#else
	return ucNULL;
#endif
}

ucBOOL UCUnreal3DVertexBuffer::IsValid() const
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	return m_BufferRHI.IsValid() && m_pDevice != ucNULL;
#else
	return m_pDevice != ucNULL;
#endif
}

UC3D_END


