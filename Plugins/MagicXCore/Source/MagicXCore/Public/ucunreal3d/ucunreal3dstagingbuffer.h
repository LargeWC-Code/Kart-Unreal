/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dstagingbuffer.h
author:		Auto Generated

purpose:	UCUnreal3D Staging Buffer - 临时上传缓冲区
*********************************************************************/
#ifndef _UC_UCUNREAL3DSTAGINGBUFFER_H_
#define _UC_UCUNREAL3DSTAGINGBUFFER_H_

#include "basetype.h"

// 注意：后续挂接 Unreal 时需要包含相关头文件
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RHI.h"
#endif

UC3D_BEGIN

// 前向声明
class UCUnreal3DDevice;
class UCUnreal3DCommandBuffer;

// ========================================
// UCUnreal3DStagingBuffer - Unreal Staging Buffer 实现
// ========================================
class UCUnreal3DStagingBuffer : public UC3DModernBuffer
{
	RTTI_DECLARE;
	friend class UCUnreal3DDevice;
	friend class UCUnreal3DCommandBuffer;

	UCUnreal3DStagingBuffer();
	~UCUnreal3DStagingBuffer();
public:

	// ========== 基本信息 ==========
	virtual UC3DBufferType GetType() const override { return UC3D_BUFFER_STAGING; }
	virtual ucUINT GetSize() const override { return m_Size; }
	virtual ucDWORD GetUsage() const override { return m_Usage; }

	// ========== 映射 ==========
	virtual ucHRESULT Map(
		UC3DModernCommandBuffer* pCmdBuffer,
		ucUINT Offset,
		ucUINT Size,
		ucVOID** ppData) override;

	virtual ucHRESULT Unmap(UC3DModernCommandBuffer* pCmdBuffer) override;

	// ========== 更新 ==========
	// Staging Buffer 通常不需要 Update，直接 Map 后写入即可
	virtual ucHRESULT Update(
		UC3DModernCommandBuffer* pCmdBuffer,
		ucUINT Offset,
		ucUINT Size,
		ucCONST ucVOID* pData) override;

	// ========== 获取底层资源句柄 ==========
	virtual ucHANDLE GetRHIBufferHandle() const override;

	// ========== 生命周期 ==========
	virtual ucBOOL IsValid() const override;

	// ========== 内部访问（仅设备内部使用）==========
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 获取 Unreal RHI Buffer
	FBufferRHIRef GetRHIBuffer() const { return m_BufferRHI; }
	
	// 设置 RHI Buffer（由 Device 在创建时调用）
	void SetRHIBuffer(FBufferRHIRef BufferRHI) { m_BufferRHI = BufferRHI; }
#endif

private:
	UCUnreal3DDevice* m_pDevice;  // 所属设备

	// Unreal RHI 资源
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	FBufferRHIRef m_BufferRHI;   // Unreal RHI Buffer
#endif

	// Buffer 信息
	ucUINT m_Size;                // Buffer 大小（字节）
	ucDWORD m_Usage;              // 使用标志

	// 映射状态
	ucVOID* m_pMappedData;        // 映射的数据指针
	ucBOOL m_bMapped;             // 是否已映射
};

UC3D_END

SCRIPT_NAMESPACE_INHERIT_DECLARE(MAGICXCORE_API, UC3D, UCE_UCUnreal3DStagingBuffer, UCUnreal3DStagingBuffer, UCE_UC3DModernBuffer, ucFALSE);

#endif //_UC_UCUNREAL3DSTAGINGBUFFER_H_

