/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dplanefont.cpp
author:		Auto Generated

purpose:	UCUnreal3D PlaneFont 实现
参考 UCVulkan3DPlaneFont 的实现，使用 Unreal Engine 的字体系统
*********************************************************************/

#include "ucunreal3d/ucunreal3dplanefont.h"
#include "ucunreal3d/ucunreal3ddevice.h"
#include "ucunreal3d/ucunreal3dtexture.h"
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Fonts/FontCache.h"
#include "Rendering/SlateRenderer.h"
#include "Internationalization/Text.h"
#include "Internationalization/Internationalization.h"
#include "Textures/SlateShaderResource.h"
#include "Engine/Engine.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformProcess.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "Slate/SlateTextures.h"
#include "TextureResource.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/FileHelper.h"
#endif

#define new UCNEW

UC3D_BEGIN

SCRIPT_IMPLEMENT_BEGIN(UCE_UCUnreal3DPlaneFont, UCUnreal3DPlaneFont);
SCRIPT_IMPLEMENT_END(UCE_UCUnreal3DPlaneFont);

RTTI_IMPLEMENT(UCUnreal3DPlaneFont, UCE_UCUnreal3DPlaneFont);

UCUnreal3DPlaneFont::UCUnreal3DPlaneFont(UC3DDevice* pDevice, UC3DPublicDataPtr* pPublicDataPtr)
	: UC3DPlaneFont(pDevice)
	, m_pDevice((UCUnreal3DDevice*)pDevice)
{
	ClonePtr(pPublicDataPtr);
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	m_CompositeFont = nullptr;
#endif
}

UCUnreal3DPlaneFont::~UCUnreal3DPlaneFont()
{
	// 清理 Unreal 字体资源
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// TSharedPtr 是智能指针，使用引用计数管理资源
	// Reset() 会减少引用计数，当引用计数为0时，资源会被自动释放
	m_CompositeFont.Reset();
	// 注意：FSlateFontInfo m_SlateFontInfo 是值类型，有自己的析构函数，会自动释放
	// m_pDevice 只是指针，不需要释放
#endif
}

ucFLOAT UCUnreal3DPlaneFont::_GetCharWidth(ucTCHAR ch)
{
	UC3DPlaneFontData* FontPlaneData = GetData();
	if (!FontPlaneData)
		return 0.0f;

	UC3DDeviceDataBase::UC3DDataMng_UI* pUIData = m_pDevice ? m_pDevice->GetUIData() : nullptr;
	ucFLOAT Scale = pUIData ? pUIData->Scale : 1.0f;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 使用 Unreal 字体系统获取字符宽度
	if (FSlateApplication::IsInitialized())
	{
		TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		FString CharStr(1, &ch);
		FText CharText = FText::FromString(CharStr);
		FVector2D Size = FontMeasure->Measure(CharText, m_SlateFontInfo);
		return (ucFLOAT)Size.X / Scale;
	}
#endif
	return 0.0f;
}

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
// 调试函数：保存字体纹理到 TGA 文件
// 这个函数可以方便地注释/取消注释来调试字体纹理
static void DebugSaveFontTextureToTGA(ISlateFontTexture* FontTexture, const FString& FilePath)
{
	if (!FontTexture)
		return;

	FTextureResource* TextureResource = FontTexture->GetEngineTexture();
	if (!TextureResource || !TextureResource->TextureRHI)
		return;

	FRHITexture* RHITexture = TextureResource->TextureRHI;
	
	// 创建同步事件
	FEvent* ReadCompleteEvent = FGenericPlatformProcess::GetSynchEventFromPool(false);
	if (!ReadCompleteEvent)
		return;

	// 用于存储读取的像素数据
	TArray<FColor> OutPixelData;
	uint32 TextureWidth = 0;
	uint32 TextureHeight = 0;
	EPixelFormat PixelFormat = PF_Unknown;

	// 在渲染线程读取纹理数据
	ENQUEUE_RENDER_COMMAND(ReadFontTextureForDebug)(
		[RHITexture, &OutPixelData, &TextureWidth, &TextureHeight, &PixelFormat, ReadCompleteEvent](FRHICommandListImmediate& RHICmdList)
		{
			if (RHITexture)
			{
				FTexture2DRHIRef Texture2D = RHITexture->GetTexture2D();
				if (Texture2D.IsValid())
				{
					TextureWidth = Texture2D->GetSizeX();
					TextureHeight = Texture2D->GetSizeY();
					PixelFormat = Texture2D->GetFormat();

					// 分配输出数据
					OutPixelData.Empty(TextureWidth * TextureHeight);
					OutPixelData.AddUninitialized(TextureWidth * TextureHeight);

					// 确保纹理可读
					RHICmdList.Transition(FRHITransitionInfo(RHITexture, ERHIAccess::Unknown, ERHIAccess::CPURead));

					// 使用 LockTexture2D 读取纹理数据
					uint32 Stride = 0;
					void* TextureData = RHICmdList.LockTexture2D(Texture2D, 0, RLM_ReadOnly, Stride, false);
					if (TextureData)
					{
						// 根据格式读取数据
						if (PixelFormat == PF_G8 || PixelFormat == PF_A8)
						{
							// G8/A8 格式：1 字节每像素
							uint8* pSrc = (uint8*)TextureData;
							for (uint32 y = 0; y < TextureHeight; ++y)
							{
								for (uint32 x = 0; x < TextureWidth; ++x)
								{
									uint8 Gray = pSrc[y * Stride + x];
									OutPixelData[y * TextureWidth + x] = FColor(Gray, Gray, Gray, Gray);
								}
							}
						}
						else if (PixelFormat == PF_B8G8R8A8)
						{
							// BGRA 格式：4 字节每像素
							uint8* pSrc = (uint8*)TextureData;
							for (uint32 y = 0; y < TextureHeight; ++y)
							{
								for (uint32 x = 0; x < TextureWidth; ++x)
								{
									uint32 SrcIdx = (y * Stride + x * 4);
									uint8 B = pSrc[SrcIdx + 0];
									uint8 G = pSrc[SrcIdx + 1];
									uint8 R = pSrc[SrcIdx + 2];
									uint8 A = pSrc[SrcIdx + 3];
									OutPixelData[y * TextureWidth + x] = FColor(R, G, B, A);
								}
							}
						}
						else
						{
							// 其他格式：尝试根据格式信息读取
							uint8* pSrc = (uint8*)TextureData;
							uint32 BytesPerPixel = 4; // 默认假设 BGRA
							if (PixelFormat == PF_G8 || PixelFormat == PF_A8)
								BytesPerPixel = 1;
							else if (PixelFormat == PF_R8G8B8A8 || PixelFormat == PF_B8G8R8A8)
								BytesPerPixel = 4;
							
							for (uint32 y = 0; y < TextureHeight; ++y)
							{
								for (uint32 x = 0; x < TextureWidth; ++x)
								{
									uint32 SrcIdx = y * Stride + x * BytesPerPixel;
									if (BytesPerPixel >= 4)
									{
										uint8 B = pSrc[SrcIdx + 0];
										uint8 G = pSrc[SrcIdx + 1];
										uint8 R = pSrc[SrcIdx + 2];
										uint8 A = pSrc[SrcIdx + 3];
										OutPixelData[y * TextureWidth + x] = FColor(R, G, B, A);
									}
									else if (BytesPerPixel == 1)
									{
										uint8 Gray = pSrc[SrcIdx];
										OutPixelData[y * TextureWidth + x] = FColor(Gray, Gray, Gray, Gray);
									}
									else
									{
										OutPixelData[y * TextureWidth + x] = FColor::White;
									}
								}
							}
						}

						RHICmdList.UnlockTexture2D(Texture2D, 0, false);
					}
				}
			}

			// 通知读取完成
			ReadCompleteEvent->Trigger();
		});

	// 等待渲染线程完成
	ReadCompleteEvent->Wait();
	FGenericPlatformProcess::ReturnSynchEventToPool(ReadCompleteEvent);

	// 检查是否成功读取数据
	if (OutPixelData.Num() != TextureWidth * TextureHeight || TextureWidth == 0 || TextureHeight == 0)
	{
		return;
	}

	// 转换像素格式为 BGRA
	TArray<uint8> RawPixelData;
	RawPixelData.Empty(TextureWidth * TextureHeight * 4);
	RawPixelData.AddUninitialized(TextureWidth * TextureHeight * 4);

	for (uint32 i = 0; i < TextureWidth * TextureHeight; ++i)
	{
		RawPixelData[i * 4 + 0] = OutPixelData[i].B;
		RawPixelData[i * 4 + 1] = OutPixelData[i].G;
		RawPixelData[i * 4 + 2] = OutPixelData[i].R;
		RawPixelData[i * 4 + 3] = OutPixelData[i].A;
	}

	// 创建 TGA 文件
	TArray<uint8> TGAData;
	
	// TGA 文件头（18 字节）
	TGAData.Add(0);  // ID length
	TGAData.Add(0);  // Color map type
	TGAData.Add(2);  // Image type: 2 = uncompressed RGB
	TGAData.Add(0); TGAData.Add(0);  // Color map first entry index
	TGAData.Add(0); TGAData.Add(0);  // Color map length
	TGAData.Add(0);  // Color map entry size
	TGAData.Add(0); TGAData.Add(0);  // X origin
	TGAData.Add(0); TGAData.Add(0);  // Y origin
	TGAData.Add((TextureWidth & 0xFF)); TGAData.Add((TextureWidth >> 8) & 0xFF);  // Width
	TGAData.Add((TextureHeight & 0xFF)); TGAData.Add((TextureHeight >> 8) & 0xFF); // Height
	TGAData.Add(32);  // Pixel depth (32-bit BGRA)
	TGAData.Add(0);   // Image descriptor

	// 添加像素数据（TGA 是从左下角到右上角，需要翻转 Y 轴）
	uint32 PixelDataSize = TextureWidth * TextureHeight * 4;
	TGAData.AddUninitialized(PixelDataSize);
	
	// 翻转 Y 轴（TGA 格式要求）
	for (uint32 y = 0; y < TextureHeight; ++y)
	{
		uint32 SrcY = TextureHeight - 1 - y;  // 翻转 Y
		for (uint32 x = 0; x < TextureWidth; ++x)
		{
			uint32 SrcIdx = (SrcY * TextureWidth + x) * 4;
			uint32 DstIdx = (y * TextureWidth + x) * 4;
			TGAData[18 + DstIdx + 0] = RawPixelData[SrcIdx + 0]; // B
			TGAData[18 + DstIdx + 1] = RawPixelData[SrcIdx + 1]; // G
			TGAData[18 + DstIdx + 2] = RawPixelData[SrcIdx + 2]; // R
			TGAData[18 + DstIdx + 3] = RawPixelData[SrcIdx + 3]; // A
		}
	}

	// 保存 TGA 文件
	FFileHelper::SaveArrayToFile(TGAData, *FilePath);
}
#endif

ucBOOL UCUnreal3DPlaneFont::_RenderChar(ucTCHAR ch, ucFontInfo& info)
{
	UC3DPlaneFontData* FontPlaneData = GetData();
	if (!FontPlaneData)
		return ucFALSE;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	if (!FSlateApplication::IsInitialized())
		return ucFALSE;

	// 使用 Unreal 的字体缓存系统渲染字符
	TSharedRef<FSlateFontCache> FontCache = FSlateApplication::Get().GetRenderer()->GetFontCache();
	

	// 先测量字符，这会触发字符在字体缓存中的生成
	TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	FString CharStr(1, &ch);
	FText CharText = FText::FromString(CharStr);
	FVector2D MeasuredSize = FontMeasure->Measure(CharText, m_SlateFontInfo);

	// 使用 ShapeUnidirectionalText 对字符进行形状化处理
	// 这会返回 FShapedGlyphSequenceRef，其中包含字符的 glyph 信息
	float FontScale = 1.0f;
	TextBiDi::ETextDirection TextDirection = TextBiDi::ETextDirection::LeftToRight;
	ETextShapingMethod TextShapingMethod = ETextShapingMethod::Auto;
	
	FShapedGlyphSequenceRef ShapedGlyphSequence = FontCache->ShapeUnidirectionalText(
		CharStr,
		m_SlateFontInfo,
		FontScale,
		TextDirection,
		TextShapingMethod
	);
	
	// 从形状化的字符序列中获取第一个（也是唯一的）glyph
	// TSharedRef 总是有效的，只需要检查是否有 glyph 可以渲染
	if (ShapedGlyphSequence->GetGlyphsToRender().Num() > 0)
	{
		const FShapedGlyphEntry& GlyphEntry = ShapedGlyphSequence->GetGlyphsToRender()[0];

		FShapedGlyphFontAtlasData AtlasData = FontCache->GetShapedGlyphFontAtlasData(
			GlyphEntry,
			m_SlateFontInfo.OutlineSettings
		);
		
		FCharacterList& CharacterList = FontCache->GetCharacterList(m_SlateFontInfo, 1.0f, m_SlateFontInfo.OutlineSettings);

		FStringView StringView = CharStr;
		TCHAR CurrentChar = StringView[0];

		const FCharacterEntry Entry = CharacterList.GetCharacter(CurrentChar, m_SlateFontInfo.FontFallback);

		if (AtlasData.Valid)
		{
			// 使用测量得到的尺寸
			info.Size = UCSize((ucINT)AtlasData.USize, (ucINT)AtlasData.VSize);
			info.Offset = UCPoint(AtlasData.HorizontalOffset, AtlasData.VerticalOffset);
			info.Adv = (ucFLOAT)MeasuredSize.X;
			
			// 设置 UV 坐标（AtlasData.StartU, AtlasData.StartV 是字符在纹理中的像素坐标）
			info.UV = UCPoint(AtlasData.StartU, AtlasData.StartV);
			
			// 参考 UCGL3DPlaneFont 的实现，需要填充 m_RenderBuffer
			// 计算 pitch（4 字节对齐）
			ucINT iPitch = ((info.Size.cx + 3) >> 2) << 2;
			FontPlaneData->m_RenderBuffer.SetSize(iPitch * info.Size.cy);
			ucBYTE* pBuffer = FontPlaneData->m_RenderBuffer.GetData();
			
			// 清零缓冲区
			ZeroMemory(pBuffer, iPitch * info.Size.cy);
			
			// 从 Slate 字体纹理中提取像素数据填充到 m_RenderBuffer
			// AtlasData 包含字符在纹理中的信息，需要从纹理读取
			ISlateFontTexture* FontTexture = FontCache->GetFontTexture(AtlasData.TextureIndex);
						
			if (FontTexture)
			{
				FTextureResource* TextureResource = FontTexture->GetEngineTexture();
				if (TextureResource && TextureResource->TextureRHI)
				{
					FRHITexture* RHITexture = TextureResource->TextureRHI;
					
					ucINT BufferSize = iPitch * info.Size.cy;
					ucBYTE* ReadBufferData = new ucBYTE[BufferSize];
					ZeroMemory(ReadBufferData, BufferSize);
					
					FEvent* ReadCompleteEvent = FGenericPlatformProcess::GetSynchEventFromPool(false);
					
					// 使用 AtlasData 中的尺寸信息
					uint32 AtlasSrcX = AtlasData.StartU;
					uint32 AtlasSrcY = AtlasData.StartV;
					uint32 AtlasUSize = AtlasData.USize;
					uint32 AtlasVSize = AtlasData.VSize;
					
					ENQUEUE_RENDER_COMMAND(ReadFontTextureData)(
						[RHITexture, AtlasSrcX, AtlasSrcY, AtlasUSize, AtlasVSize, iPitch, ReadBufferData, BufferSize, ReadCompleteEvent](FRHICommandListImmediate& RHICmdList)
						{
							FTexture2DRHIRef Texture2D = RHITexture->GetTexture2D();
							if (Texture2D.IsValid())
							{
								// 转换纹理为可读状态
								RHICmdList.Transition(FRHITransitionInfo(RHITexture, ERHIAccess::SRVMask, ERHIAccess::CPURead));
								
								uint32 TextureWidth = Texture2D->GetSizeX();
								uint32 TextureHeight = Texture2D->GetSizeY();
								EPixelFormat PixelFormat = Texture2D->GetFormat();
								
								// 使用 AtlasData 中的坐标和尺寸
								// AtlasData.StartU/StartV 是像素坐标，AtlasUSize/VSize 是字符在纹理中的实际尺寸
								uint32 SrcX = AtlasSrcX;
								uint32 SrcY = AtlasSrcY;
								
								// 使用 AtlasData 中的尺寸
								uint32 CopyWidth = FMath::Min(AtlasUSize, TextureWidth - SrcX);
								uint32 CopyHeight = FMath::Min(AtlasVSize, TextureHeight - SrcY);
								
								// 锁定纹理以读取数据
								uint32 Stride = 0;
								void* TextureData = RHICmdList.LockTexture2D(Texture2D, 0, RLM_ReadOnly, Stride, false);
								if (TextureData)
								{
									uint32 srcRowPitch = Stride;
									
									// 根据纹理格式处理
									if (PixelFormat == PF_G8 || PixelFormat == PF_A8)
									{
										// A8 格式（PF_G8）：每像素 1 字节，直接是 alpha 值
										uint32 bytesPerPixel = 1;
										uint8* pSrc = (uint8*)TextureData + SrcY * srcRowPitch + SrcX * bytesPerPixel;
										
										// 直接复制数据
										for (uint32 y = 0; y < CopyHeight; ++y)
										{
											for (uint32 x = 0; x < CopyWidth; ++x)
											{
												uint8 Alpha = pSrc[y * srcRowPitch + x * bytesPerPixel];
												ReadBufferData[y * iPitch + x] = Alpha;
											}
										}
									}
									else if (PixelFormat == PF_B8G8R8A8)
									{
										// BGRA 格式：每像素 4 字节，Alpha 在索引 3
										uint32 bytesPerPixel = 4;
										uint8* pSrc = (uint8*)TextureData + SrcY * srcRowPitch + SrcX * bytesPerPixel;
										
										// 提取 alpha 通道
										for (uint32 y = 0; y < CopyHeight; ++y)
										{
											for (uint32 x = 0; x < CopyWidth; ++x)
											{
												// BGRA 格式：B, G, R, A，Alpha 在索引 3
												uint8 Alpha = pSrc[y * srcRowPitch + x * bytesPerPixel + 3];
												ReadBufferData[y * iPitch + x] = Alpha;
											}
										}
									}
									
									RHICmdList.UnlockTexture2D(Texture2D, 0, false);
								}
							}
							
							ReadCompleteEvent->Trigger();
						});
					
					ReadCompleteEvent->Wait();
					FGenericPlatformProcess::ReturnSynchEventToPool(ReadCompleteEvent);
					
					// 将读取的数据拷贝到 m_RenderBuffer
					FMemory::Memcpy(pBuffer, ReadBufferData, BufferSize);
					delete[] ReadBufferData;
				}

// 				// 调试：保存字体纹理到文件（可以注释掉这行来禁用调试）
// 				DebugSaveFontTextureToTGA(FontTexture, TEXT("E:/fonttexture-dbg.tga"));
			}
			
			return ucTRUE;
		}
	}
#endif
	return ucFALSE;
}

UCUnreal3DPlaneFont* UCUnreal3DPlaneFont::Create(UCUnreal3DDevice* pDevice, UC3DPublicDataPtr* pPublicDataPtr, ucCONST UC3DCreateFont& nFont)
{
	// 参考 UCVulkan3DPlaneFont::Create 的实现结构
	UC3DDeviceDataBase::UC3DDataMng_UI* pUIData = pDevice->GetUIData();
	ucFLOAT Scale = pUIData ? pUIData->Scale : 1.0f;
	ucINT nFontSize = (ucINT)((ucFLOAT)nFont.Size * Scale);

	if (nFontSize < 1)
		return ucNULL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	if (!FSlateApplication::IsInitialized())
		return ucNULL;

	// 获取字体高度（类似 Vulkan 版本中的 GetTextMetrics）
	ucINT iMaxHeight = nFont.Size;
	ucINT iUnderlinePos = 0;

	// 创建字体对象
	UCUnreal3DPlaneFont* pFont = new UCUnreal3DPlaneFont(pDevice, pPublicDataPtr);
	if (!pFont)
		return ucNULL;

	UC3DPlaneFontData* FontPlaneData = pFont->GetData();
	
	// 设置字体数据（与 Vulkan 版本保持一致）
	FontPlaneData->File = nFont.File;
	FontPlaneData->Name = nFont.Name;
	FontPlaneData->Size = nFont.Size;
	FontPlaneData->Weight = nFont.Weight;
	FontPlaneData->Smooth = nFont.Smooth;
	FontPlaneData->Device = pDevice;

	// 设置 Slate 字体信息
	FSlateFontInfo SlateFontInfo;
	SlateFontInfo.Size = nFontSize / 2;

	// 如果有字体名称，使用指定字体
// 	if (nFont.Name.GetLength() > 0)
// 	{
// 		const ucTCHAR* pFontName = nFont.Name;
// 		FString FontName(pFontName);
// 		SlateFontInfo.TypefaceFontName = FName(*FontName);
// 	}
// 	else
	{
		SlateFontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("/Engine/EngineFonts/Roboto"), nFontSize);
	}

	pFont->m_SlateFontInfo = SlateFontInfo;

	// 使用字体测量服务获取实际的字体高度（类似 Vulkan 版本中的 GetTextMetrics）
	TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	FText TestText = FText::FromString(TEXT("A"));
	FVector2D TextSize = FontMeasure->Measure(TestText, SlateFontInfo);
	
	iMaxHeight = (ucINT)TextSize.Y;
	iUnderlinePos = 0;  // TODO: 从 Slate 获取下划线位置（类似 tmDescent）

	// 设置缩放后的高度（与 Vulkan 版本保持一致）
	FontPlaneData->MaxHeight = (ucINT)((ucFLOAT)iMaxHeight / Scale);
	FontPlaneData->UnderlinePos = (ucINT)((ucFLOAT)iUnderlinePos / Scale);
	FontPlaneData->ScaledMaxHeight = iMaxHeight;
	FontPlaneData->ScaledUnderlinePos = iUnderlinePos;

	return pFont;
#else
	return ucNULL;
#endif
}

ucVOID UCUnreal3DPlaneFont::OnLostDevice()
{
	// Unreal 设备不需要处理设备丢失
}

ucBOOL UCUnreal3DPlaneFont::OnResetDevice()
{
	return ucTRUE;
}

ucVOID UCUnreal3DPlaneFont::OnReload()
{
	UC3DPlaneFontData* FontPlaneData = GetData();
	if (!FontPlaneData)
		return;

	UC3DDeviceDataBase::UC3DDataMng_UI* pUIData = m_pDevice ? m_pDevice->GetUIData() : nullptr;
	ucFLOAT Scale = pUIData ? pUIData->Scale : 1.0f;
	ucINT nFontSize = (ucINT)((ucFLOAT)FontPlaneData->Size * Scale);

	if (nFontSize < 1)
		return;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	if (!FSlateApplication::IsInitialized())
		return;

	// 更新 Slate 字体信息
	m_SlateFontInfo.Size = nFontSize;

	// 重新获取字体高度
	TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	FText TestText = FText::FromString(TEXT("A"));
	FVector2D TextSize = FontMeasure->Measure(TestText, m_SlateFontInfo);
	
	ucINT iMaxHeight = (ucINT)TextSize.Y;
	ucINT iUnderlinePos = 0;  // TODO: 从 Slate 获取下划线位置

	// 清理字体缓存
	FontPlaneData->FreeCache();

	// 更新高度
	FontPlaneData->MaxHeight = (ucINT)((ucFLOAT)iMaxHeight / Scale);
	FontPlaneData->UnderlinePos = (ucINT)((ucFLOAT)iUnderlinePos / Scale);
	FontPlaneData->ScaledMaxHeight = iMaxHeight;
	FontPlaneData->ScaledUnderlinePos = iUnderlinePos;
#endif
}

ucVOID UCUnreal3DPlaneFont::DrawText(UC3DDevice* Device, ucCONST UCFPoint& pt, ucCONST UCString& strText, ucCONST UC3DFont& nFont, ucFLOAT fZ)
{
	// Modern 设备的 DrawText 实现在 UC3DPlaneFont::DrawText 中统一处理
	// 这里不需要特殊实现，直接调用基类方法即可
	UC3DPlaneFont::DrawText(Device, pt, strText, nFont, fZ);
}

UC3D_END

