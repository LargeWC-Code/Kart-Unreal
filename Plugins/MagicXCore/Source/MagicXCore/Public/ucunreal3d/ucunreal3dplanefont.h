/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dplanefont.h
author:		Auto Generated

purpose:	UCUnreal3D PlaneFont - 平面字体实现（用于UI文本渲染）
*********************************************************************/
#ifndef _UC_UCUNREAL3DPLANEFONT_H_
#define _UC_UCUNREAL3DPLANEFONT_H_

#include "basetype.h"

UC3D_BEGIN

// 前向声明
class UCUnreal3DDevice;
struct UC3DCreateFont;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "Fonts/SlateFontInfo.h"
#include "Fonts/FontCache.h"
#endif

// ========================================
// UCUnreal3DPlaneFont - Unreal Plane Font 实现
// 参考 UCVulkan3DPlaneFont 的实现，使用 Unreal Engine 的字体系统
// ========================================
class UCUnreal3DPlaneFont : public UC3DPlaneFont
{
	RTTI_DECLARE;
	friend class UCUnreal3DDevice;

	UCUnreal3DPlaneFont(UC3DDevice* pDevice, UC3DPublicDataPtr* pPublicDataPtr);
	~UCUnreal3DPlaneFont();

public:
	// 静态创建方法（类似 UCVulkan3DPlaneFont::Create）
	static UCUnreal3DPlaneFont* Create(UCUnreal3DDevice* pDevice, UC3DPublicDataPtr* pPublicDataPtr, ucCONST UC3DCreateFont& createInfo);

	// ========== 基类纯虚函数实现 ==========
	virtual ucFLOAT _GetCharWidth(ucTCHAR ch) override;
	virtual ucBOOL _RenderChar(ucTCHAR ch, ucFontInfo& info) override;
	virtual ucVOID OnLostDevice() override;
	virtual ucBOOL OnResetDevice() override;
	virtual ucVOID OnReload() override;
	virtual ucVOID DrawText(UC3DDevice* Device, ucCONST UCFPoint& pt, ucCONST UCString& strText, ucCONST UC3DFont& nFont, ucFLOAT fZ) override;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 获取 Slate 字体信息
	const FSlateFontInfo& GetSlateFontInfo() const { return m_SlateFontInfo; }
#endif

private:
	UCUnreal3DDevice* m_pDevice;  // 所属设备

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	FSlateFontInfo m_SlateFontInfo;  // Unreal Slate 字体信息
	TSharedPtr<const FCompositeFont> m_CompositeFont;  // 复合字体（用于渲染）
#endif
};

UC3D_END

SCRIPT_NAMESPACE_DECLARE(MAGICXCORE_API, UC3D, UCE_UCUnreal3DPlaneFont, UCUnreal3DPlaneFont, ucFALSE);

#endif //_UC_UCUNREAL3DPLANEFONT_H_


