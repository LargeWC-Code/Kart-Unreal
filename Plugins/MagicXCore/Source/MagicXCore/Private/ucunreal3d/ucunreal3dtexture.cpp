/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3dtexture.cpp
author:		Auto Generated

purpose:	UCUnreal3D Texture 实现
*********************************************************************/

#include "ucunreal3d/ucunreal3dtexture.h"
#include "ucunreal3d/ucunreal3ddevice.h"
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RHI.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderUtils.h"
#endif

#define new UCNEW

UC3D_BEGIN

SCRIPT_IMPLEMENT_BEGIN(UCE_UCUnreal3DTexture, UCUnreal3DTexture);
SCRIPT_IMPLEMENT_END(UCE_UCUnreal3DTexture);

RTTI_IMPLEMENT(UCUnreal3DTexture, UCE_UCUnreal3DTexture);

UCUnreal3DTexture::UCUnreal3DTexture()
	: m_pDevice(nullptr)
	, m_Width(0)
	, m_Height(0)
	, m_Levels(0)
	, m_Format(UC3DFMT_UNKNOWN)
	, m_CurrentState(UC3DRESOURCE_STATE_COMMON)
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	m_TextureRHI = nullptr;
#endif
}

UCUnreal3DTexture::~UCUnreal3DTexture()
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// FTextureRHIRef 是智能指针，使用引用计数管理资源
	// SafeRelease() 会减少引用计数，当引用计数为0时，RHI系统会在渲染线程自动释放资源
	// 在析构函数中，也可以直接让智能指针析构，它会自动处理引用计数
	// 但使用 SafeRelease() 更明确，表明我们主动释放资源
	m_TextureRHI.SafeRelease();
	// 或者使用 Reset()，效果相同：
	// m_TextureRHI.Reset();
#endif
	// 注意：UCString m_Name 有自己的析构函数，会自动释放
	// m_pDevice 只是指针，不需要释放
}

ucUINT UCUnreal3DTexture::GetWidth() const
{
	return m_Width;
}

ucUINT UCUnreal3DTexture::GetHeight() const
{
	return m_Height;
}

ucUINT UCUnreal3DTexture::GetMipLevels() const
{
	return m_Levels;
}

UCString UCUnreal3DTexture::GetName() const
{
	return m_Name;
}

UC3DFORMAT UCUnreal3DTexture::GetFormat() const
{
	return m_Format;
}

ucHRESULT UCUnreal3DTexture::UpdateRegion(
	UC3DModernCommandBuffer* pCmdBuffer,
	ucUINT Level,
	ucCONST UCPoint& Offset,
	ucCONST UCSize& Size,
	ucCONST ucVOID* pData)
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	if (pData == nullptr || !m_TextureRHI.IsValid())
		return ucE_FAIL;

	// 检查边界
	if ((ucINT)Offset.x + (ucINT)Size.cx > (ucINT)m_Width || 
		(ucINT)Offset.y + (ucINT)Size.cy > (ucINT)m_Height)
		return ucE_FAIL;

	// 计算像素大小（字节）
	ucUINT bytesPerPixel = 1;
	switch (m_Format)
	{
	case UC3DFMT_A8:
		bytesPerPixel = 1;
		break;
	case UC3DFMT_R8G8B8:
		bytesPerPixel = 3;
		break;
	case UC3DFMT_A8R8G8B8:
	case UC3DFMT_A8B8G8R8:
	case UC3DFMT_X8R8G8B8:
	case UC3DFMT_X8B8G8R8:
		bytesPerPixel = 4;
		break;
	default:
		// 不支持其他格式
		return ucE_FAIL;
	}

	// 计算源数据的行对齐（4 字节对齐，与字体渲染缓冲区一致）
	ucUINT srcRowPitch = ((Size.cx * bytesPerPixel + 3) >> 2) << 2;
	
	// 创建临时缓冲区来存储数据（在渲染线程上需要使用拷贝）
	ucUINT dataSize = srcRowPitch * Size.cy;
	TArray<uint8> DataCopy;
	DataCopy.AddUninitialized(dataSize);
	FMemory::Memcpy(DataCopy.GetData(), pData, dataSize);

	// 获取纹理引用
	FTextureRHIRef TextureRHI = m_TextureRHI;

	// 在渲染线程上更新纹理数据
	// 使用 RHIUpdateTexture2D 的带 Region 参数版本，支持局部更新
	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionCommand)(
		[TextureRHI, Level, Offset, Size, srcRowPitch, DataCopy = MoveTemp(DataCopy)](FRHICommandListImmediate& RHICmdList)
		{
			// 构造更新区域结构
			FUpdateTextureRegion2D UpdateRegion;
			UpdateRegion.SrcX = 0;  // 源数据从 (0,0) 开始
			UpdateRegion.SrcY = 0;
			UpdateRegion.DestX = Offset.x;  // 目标位置
			UpdateRegion.DestY = Offset.y;
			UpdateRegion.Width = Size.cx;   // 区域大小
			UpdateRegion.Height = Size.cy;

			// 使用带 Region 参数的 UpdateTexture2D，直接更新指定区域
			// 这比 LockTexture2D 更高效，因为支持局部更新
			RHICmdList.UpdateTexture2D(
				TextureRHI,
				Level,
				UpdateRegion,
				srcRowPitch,
				DataCopy.GetData()
			);
		});

	// 如果提供了命令缓冲区，可以立即刷新；否则等待后续刷新
	// 这里为了保持与 Vulkan 实现的一致性，立即刷新
	if (pCmdBuffer == nullptr)
	{
		FlushRenderingCommands();
	}

	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DTexture::TransitionTo(
	UC3DModernCommandBuffer* pCmdBuffer,
	UC3DRESOURCE_STATE NewState)
{
	m_CurrentState = NewState;
	// TODO: 实现资源状态转换
	return ucS_OK;
}

UC3DRESOURCE_STATE UCUnreal3DTexture::GetCurrentState() const
{
	return m_CurrentState;
}

ucHRESULT UCUnreal3DTexture::SaveToFile(UCString Filename, UC3DIMAGE_FILEFORMAT fmt)
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	if (!m_TextureRHI.IsValid())
		return ucE_FAIL;

	// 转换文件格式枚举（假设 UC3DIMAGE_FILEFORMAT 是简单的整数值）
	// 0=PNG, 1=BMP, 2=TGA, 3=JPEG 等（根据实际情况调整）
	EImageFormat ImageFormat = EImageFormat::PNG;
	switch ((int)fmt)
	{
	case 0: // 假设 0 是 PNG
		ImageFormat = EImageFormat::PNG;
		break;
	case 1: // 假设 1 是 BMP
		ImageFormat = EImageFormat::BMP;
		break;
	case 2: // 假设 2 是 TGA
		ImageFormat = EImageFormat::TGA;
		break;
	case 3: // 假设 3 是 JPEG
		ImageFormat = EImageFormat::JPEG;
		break;
	default:
		ImageFormat = EImageFormat::PNG; // 默认使用 PNG
		break;
	}

	// 创建同步事件
	FEvent* ReadCompleteEvent = FGenericPlatformProcess::GetSynchEventFromPool(false);
	check(ReadCompleteEvent);

	// 用于存储读取的像素数据
	TArray<FColor> OutPixelData;
	uint32 ExpectedPixelCount = 0;

	// 获取纹理尺寸
	uint32 TextureWidth = m_TextureRHI->GetSizeX();
	uint32 TextureHeight = m_TextureRHI->GetSizeY();
	ExpectedPixelCount = TextureWidth * TextureHeight;

	// 在渲染线程读取纹理数据
	// 使用 LockTexture2D 而不是 ReadSurfaceData，因为字体纹理可能不支持 ReadSurfaceData
	ENQUEUE_RENDER_COMMAND(ReadTextureDataForSave)(
		[TextureRHI = m_TextureRHI, &OutPixelData, TextureWidth, TextureHeight, ReadCompleteEvent](FRHICommandListImmediate& RHICmdList)
		{
			if (TextureRHI.IsValid())
			{
				// 分配输出数据
				OutPixelData.Empty(TextureWidth * TextureHeight);
				OutPixelData.AddUninitialized(TextureWidth * TextureHeight);

				// 确保纹理可读
				RHICmdList.Transition(FRHITransitionInfo(TextureRHI, ERHIAccess::Unknown, ERHIAccess::CPURead));

				// 使用 LockTexture2D 读取纹理数据（更可靠，支持更多格式）
				FTexture2DRHIRef Texture2D = TextureRHI->GetTexture2D();
				if (Texture2D.IsValid())
				{
					uint32 Stride = 0;
					void* TextureData = RHICmdList.LockTexture2D(Texture2D, 0, RLM_ReadOnly, Stride, false);
					if (TextureData)
					{
						EPixelFormat PixelFormat = Texture2D->GetFormat();
						
						// 根据格式读取数据
						if (PixelFormat == PF_G8)
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
							// 根据常见格式估算字节数（如果无法获取精确值）
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
										// 默认白色
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

	// 检查是否成功读取数据（通过检查数组大小）
	if (OutPixelData.Num() != ExpectedPixelCount)
	{
		return ucE_FAIL;
	}

	// 转换像素格式（根据纹理格式）
	EPixelFormat PixelFormat = m_TextureRHI->GetFormat();
	ERGBFormat RGBFormat = ERGBFormat::BGRA;
	uint8 BitDepth = 8;
	TArray<uint8> RawPixelData;
	bool bUseRawData = false;

	switch (PixelFormat)
	{
	case PF_G8: // A8 格式
		{
			// 对于 G8 格式，提取灰度值并转换为 RGBA
			RGBFormat = ERGBFormat::RGBA;
			BitDepth = 8;
			RawPixelData.Empty(TextureWidth * TextureHeight * 4);
			RawPixelData.AddUninitialized(TextureWidth * TextureHeight * 4);

			for (uint32 i = 0; i < TextureWidth * TextureHeight; ++i)
			{
				uint8 Gray = OutPixelData[i].R; // 对于 G8，R/G/B 都相同，使用 R 通道
				RawPixelData[i * 4 + 0] = Gray; // R
				RawPixelData[i * 4 + 1] = Gray; // G
				RawPixelData[i * 4 + 2] = Gray; // B
				RawPixelData[i * 4 + 3] = 255;  // A (全不透明)
			}
			bUseRawData = true;
		}
		break;
	case PF_B8G8R8A8:
	default:
		{
			// BGRA 格式直接使用
			RGBFormat = ERGBFormat::BGRA;
			BitDepth = 8;
			RawPixelData.Empty(TextureWidth * TextureHeight * 4);
			RawPixelData.AddUninitialized(TextureWidth * TextureHeight * 4);

			for (uint32 i = 0; i < TextureWidth * TextureHeight; ++i)
			{
				RawPixelData[i * 4 + 0] = OutPixelData[i].B;
				RawPixelData[i * 4 + 1] = OutPixelData[i].G;
				RawPixelData[i * 4 + 2] = OutPixelData[i].R;
				RawPixelData[i * 4 + 3] = OutPixelData[i].A;
			}
			bUseRawData = true;
		}
		break;
	}

	if (!bUseRawData || RawPixelData.Num() == 0)
	{
		return ucE_FAIL;
	}

	// 转换文件路径
	FString FilePath = FString(Filename.GetBuffer());

	// 根据文件格式选择保存方式
	if (ImageFormat == EImageFormat::TGA)
	{
		// TGA 格式：直接写入原始数据（简单格式，不需要 ImageWrapper）
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
		if (!FFileHelper::SaveArrayToFile(TGAData, *FilePath))
		{
			return ucE_FAIL;
		}
	}
	else
	{
		// 其他格式使用 ImageWrapper
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
		if (!ImageWrapper.IsValid())
		{
			return ucE_FAIL;
		}

		// 设置原始数据 - 使用正确的数据大小（字节数）
		uint32 RawDataSize = TextureWidth * TextureHeight * 4; // RGBA/BGRA 都是 4 字节/像素
		if (!ImageWrapper->SetRaw(RawPixelData.GetData(), RawDataSize, TextureWidth, TextureHeight, RGBFormat, BitDepth))
		{
			return ucE_FAIL;
		}

		// 压缩图像数据
		const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed(100); // 100 是质量（对于 PNG 和 BMP 无效）
		if (CompressedData.Num() == 0)
		{
			return ucE_FAIL;
		}

		// 保存到文件
		if (!FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
		{
			return ucE_FAIL;
		}
	}

	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHANDLE UCUnreal3DTexture::GetRHITextureHandle() const
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	if (m_TextureRHI.IsValid())
		return (ucHANDLE)m_TextureRHI.GetReference();
#endif
	return ucNULL;
}

ucBOOL UCUnreal3DTexture::IsValid() const
{
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	return m_TextureRHI.IsValid();
#else
	return ucFALSE;
#endif
}

UC3D_END

