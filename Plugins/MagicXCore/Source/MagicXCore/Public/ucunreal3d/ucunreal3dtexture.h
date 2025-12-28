/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dtexture.h
author:		Auto Generated

purpose:	UCUnreal3D Texture
*********************************************************************/
#ifndef _UC_UCUNREAL3DTEXTURE_H_
#define _UC_UCUNREAL3DTEXTURE_H_

#include "basetype.h"

// 注意：后续挂接 Unreal 时需要包含相关头文件
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RHI.h"
#include "TextureResource.h"
#endif

UC3D_BEGIN

// 前向声明
class UCUnreal3DDevice;
class UCUnreal3DCommandBuffer;

// ========================================
// UCUnreal3DTexture - Unreal Texture 实现
// ========================================
class UCUnreal3DTexture : public UC3DModernTexture
{
	RTTI_DECLARE;
	friend class UCUnreal3DDevice;
	friend class UCUnreal3DCommandBuffer;

	UCUnreal3DTexture();
	~UCUnreal3DTexture();

public:
	// ========== 基本信息 ==========
	virtual ucUINT GetWidth() const override;
	virtual ucUINT GetHeight() const override;
	virtual ucUINT GetMipLevels() const override;
	virtual UC3DFORMAT GetFormat() const override;
	virtual UCString GetName() const override;

	// ========== 更新纹理数据 ==========
	virtual ucHRESULT UpdateRegion(
		UC3DModernCommandBuffer* pCmdBuffer,
		ucUINT Level,
		ucCONST UCPoint& Offset,
		ucCONST UCSize& Size,
		ucCONST ucVOID* pData) override;
	
	virtual ucHRESULT TransitionTo(
		UC3DModernCommandBuffer* pCmdBuffer,
		UC3DRESOURCE_STATE NewState) override;
	
	virtual UC3DRESOURCE_STATE GetCurrentState() const override;

	// ========== 保存纹理到文件 ==========
	virtual ucHRESULT SaveToFile(UCString Filename, UC3DIMAGE_FILEFORMAT fmt) override;

	// ========== 获取底层资源句柄 ==========
	virtual ucHANDLE GetRHITextureHandle() const override;

	// ========== 生命周期 ==========
	virtual ucBOOL IsValid() const override;

	// ========== 内部访问（仅设备内部使用）==========
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 获取 Unreal RHI Texture
	FTextureRHIRef GetRHITexture() const { return m_TextureRHI; }
	
	// 设置 RHI Texture（由 Device 在创建时调用）
	void SetRHITexture(FTextureRHIRef TextureRHI) { m_TextureRHI = TextureRHI; }
#endif

private:
	UCUnreal3DDevice* m_pDevice;  // 所属设备

	// Unreal RHI 资源
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	FTextureRHIRef m_TextureRHI;   // Unreal RHI Texture
#endif

	// Texture 信息
	ucUINT m_Width;                // 宽度
	ucUINT m_Height;               // 高度
	ucUINT m_Levels;               // Mip 级别数
	UC3DFORMAT m_Format;           // 格式
	UCString m_Name;               // 纹理名称
	UC3DRESOURCE_STATE m_CurrentState;  // 当前资源状态
};

UC3D_END

SCRIPT_NAMESPACE_INHERIT_DECLARE(MAGICXCORE_API, UC3D, UCE_UCUnreal3DTexture, UCUnreal3DTexture, UCE_UC3DModernTexture, ucFALSE);

#endif //_UC_UCUNREAL3DTEXTURE_H_


