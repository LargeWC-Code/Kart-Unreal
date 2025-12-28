/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dvertexbuffer.h
author:		Auto Generated

purpose:	UCUnreal3D VertexBuffer
*********************************************************************/
#ifndef _UC_UCUNREAL3DVERTEXBUFFER_H_
#define _UC_UCUNREAL3DVERTEXBUFFER_H_

#include "basetype.h"

// 注意：后续挂接 Unreal 时需要包含相关头文件
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RHI.h"
#endif

UC3D_BEGIN

// 前向声明
class UCUnreal3DDevice;
class UCUnreal3DCommandBuffer;

class UCUnreal3DVertexBuffer : public UC3DModernVertexBuffer
{
	RTTI_DECLARE;
	friend class UCUnreal3DDevice;
	friend class UCUnreal3DCommandBuffer;

	UCUnreal3DVertexBuffer();
	~UCUnreal3DVertexBuffer();

public:
	// ========== 基本信息 ==========
	virtual ucUINT GetSize() const override { return m_Size; }
	virtual ucUINT GetStride() const override { return m_Stride; }

	// ========== 更新 ==========
	virtual ucHRESULT Update(
		UC3DModernCommandBuffer* pCmdBuffer,
		ucUINT Offset,
		ucUINT Size,
		ucCONST ucVOID* pData) override;

	// ========== 映射 ==========
	virtual ucHRESULT Map(
		UC3DModernCommandBuffer* pCmdBuffer,
		ucUINT Offset,
		ucUINT Size,
		ucVOID** ppData) override;

	virtual ucHRESULT Unmap(UC3DModernCommandBuffer* pCmdBuffer) override;

	// ========== 获取底层资源句柄 ==========
	virtual ucHANDLE GetRHIBufferHandle() const override;

	// ========== 生命周期 ==========
	virtual ucBOOL IsValid() const override;

	// ========== 基类方法（继承自 UC3DModernBuffer）==========
	// GetType() 已在基类中实现，返回 UC3D_BUFFER_VERTEX
	// GetUsage() 需要实现
	virtual ucDWORD GetUsage() const override { return m_Usage; }

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
	ucUINT m_Stride;              // 顶点步长（字节）
	ucDWORD m_Usage;              // 使用标志

	// 映射状态
	ucVOID* m_pMappedData;        // 映射的数据指针（如果支持）
	ucBOOL m_bMapped;             // 是否已映射
};

UC3D_END

SCRIPT_NAMESPACE_INHERIT_DECLARE(MAGICXCORE_API, UC3D, UCE_UCUnreal3DVertexBuffer, UCUnreal3DVertexBuffer, UCE_UC3DModernVertexBuffer, ucFALSE);

#endif //_UC_UCUNREAL3DVERTEXBUFFER_H_


