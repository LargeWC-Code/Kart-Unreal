/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dstagingbuffer.cpp
author:		Auto Generated

purpose:	UCUnreal3D Staging Buffer 实现
*********************************************************************/
#include "ucunreal3d/ucunreal3dstagingbuffer.h"
#include "ucunreal3d/ucunreal3ddevice.h"

// 注意：后续挂接 Unreal 时需要包含相关头文件
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RHI.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#endif

#define new UCNEW

UC3D_BEGIN

RTTI_IMPLEMENT(UCUnreal3DStagingBuffer, UCE_UCUnreal3DStagingBuffer);
SCRIPT_IMPLEMENT_BEGIN(UCE_UCUnreal3DStagingBuffer, UCUnreal3DStagingBuffer);
SCRIPT_IMPLEMENT_END(UCE_UCUnreal3DStagingBuffer);

UCUnreal3DStagingBuffer::UCUnreal3DStagingBuffer()
	: m_pDevice(ucNULL)
	, m_Size(0)
	, m_Usage(0)
	, m_pMappedData(ucNULL)
	, m_bMapped(ucFALSE)
{
	DebugInfo = &UCE_UCUnreal3DStagingBuffer::I;
}

UCUnreal3DStagingBuffer::~UCUnreal3DStagingBuffer()
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
	m_Usage = 0;
}

ucHRESULT UCUnreal3DStagingBuffer::Map(
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
	ENQUEUE_RENDER_COMMAND(MapStagingBufferCommand)(
		[this, Offset, Size, ppData, &Result](FRHICommandListImmediate& RHICmdList)
		{
			// 锁定 Buffer 进行映射
			// 注意：Unreal 的 Buffer 映射方式与 Vulkan 不同
			// 使用 LockBuffer 进行映射
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

ucHRESULT UCUnreal3DStagingBuffer::Unmap(UC3DModernCommandBuffer* pCmdBuffer)
{
	if (!m_bMapped || !m_BufferRHI.IsValid())
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 在渲染线程中取消映射
	ENQUEUE_RENDER_COMMAND(UnmapStagingBufferCommand)(
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

ucHRESULT UCUnreal3DStagingBuffer::Update(
	UC3DModernCommandBuffer* pCmdBuffer,
	ucUINT Offset,
	ucUINT Size,
	ucCONST ucVOID* pData)
{
	// Staging Buffer 通常不需要 Update，直接 Map 后写入即可
	// 但为了接口一致性，提供此方法
	if (pData == nullptr || Size == 0)
		return ucE_FAIL;

	ucVOID* pMappedData = ucNULL;
	ucHRESULT hr = Map(pCmdBuffer, Offset, Size, &pMappedData);
	if (ucFAILED(hr))
		return hr;

	memcpy(pMappedData, pData, Size);
	Unmap(pCmdBuffer);

	return ucS_OK;
}

ucHANDLE UCUnreal3DStagingBuffer::GetRHIBufferHandle() const
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	return reinterpret_cast<ucHANDLE>(m_BufferRHI.GetReference());
#else
	return ucNULL;
#endif
}

ucBOOL UCUnreal3DStagingBuffer::IsValid() const
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	return m_BufferRHI.IsValid() && m_pDevice != ucNULL;
#else
	return m_pDevice != ucNULL;
#endif
}

UC3D_END

