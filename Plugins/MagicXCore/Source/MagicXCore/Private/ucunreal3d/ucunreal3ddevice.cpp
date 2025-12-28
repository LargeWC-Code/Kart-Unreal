/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3ddevice.cpp
author:		Auto Generated

purpose:	UCUnreal3D Device 实现
*********************************************************************/

#include "ucunreal3d/ucunreal3ddevice.h"
#include "ucunreal3d/ucunreal3dcommandbuffer.h"
#include "ucunreal3d/ucunreal3dstagingbuffer.h"
#include "ucunreal3d/ucunreal3dvertexbuffer.h"
#include "ucunreal3d/ucunreal3dtexture.h"
#include "ucunreal3d/ucunreal3dplanefont.h"

#include "UnrealGlobalOverlay.h"
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "Fonts/FontCache.h"
#include "Fonts/FontMeasure.h"
#include "Engine/Engine.h"
#include "Slate/SlateGameResources.h"
#include "Engine/Texture2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "TextureResource.h"
#include "Rendering/Texture2DResource.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Text/STextBlock.h"
#endif

// 注意：后续挂接 Unreal 时需要包含相关头文件
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
#include "RHI.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterUtils.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#endif

// UC3D 基础头文件（需要根据实际路径调整）
// #include "uc3d.h"

#define new UCNEW

// ============================================================
// UE Global Shaders / Vertex Layout (must be at global scope)
// ============================================================
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)

struct FUCUnrealUIRectVertex
{
	FVector2f Position; // ATTRIBUTE0
	FVector2f UV;       // ATTRIBUTE1
	FColor Color;       // ATTRIBUTE2 (VET_Color)
};

class FUCUnrealUIRectVS : public FGlobalShader
{
	DECLARE_EXPORTED_SHADER_TYPE(FUCUnrealUIRectVS, Global, /*MYMODULE_API*/);

	FUCUnrealUIRectVS() {}
	FUCUnrealUIRectVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}
};

class FUCUnrealUIRectPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FUCUnrealUIRectPS);
	SHADER_USE_PARAMETER_STRUCT(FUCUnrealUIRectPS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	END_SHADER_PARAMETER_STRUCT()
};

// 项目根目录 Shaders/ 会映射为虚拟路径 /Project
IMPLEMENT_SHADER_TYPE(, FUCUnrealUIRectVS, TEXT("/Project/UIRectVertexShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FUCUnrealUIRectPS, TEXT("/Project/UIRectPixelShader.usf"), TEXT("MainPS"), SF_Pixel);

static FVertexDeclarationRHIRef GUCUnrealUIRectVertexDecl;
static FVertexDeclarationRHIRef GetUCUnrealUIRectVertexDecl()
{
	if (!GUCUnrealUIRectVertexDecl.IsValid())
	{
		FVertexDeclarationElementList Elements;
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUCUnrealUIRectVertex, Position), VET_Float2, 0, sizeof(FUCUnrealUIRectVertex)));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUCUnrealUIRectVertex, UV),       VET_Float2, 1, sizeof(FUCUnrealUIRectVertex)));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUCUnrealUIRectVertex, Color),    VET_Color,  2, sizeof(FUCUnrealUIRectVertex)));
		GUCUnrealUIRectVertexDecl = RHICreateVertexDeclaration(Elements);
	}
	return GUCUnrealUIRectVertexDecl;
}

#endif

UC3D_BEGIN

SCRIPT_IMPLEMENT_BEGIN(UCE_UCUnreal3DService, UCUnreal3DService);
SCRIPT_IMPLEMENT_END(UCE_UCUnreal3DService);

RTTI_IMPLEMENT(UCUnreal3DService, UCE_UCUnreal3DService);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCUnreal3DDevice, UCUnreal3DDevice);
SCRIPT_IMPLEMENT_END(UCE_UCUnreal3DDevice);

RTTI_IMPLEMENT(UCUnreal3DDevice, UCE_UCUnreal3DDevice);

// ========================================
// UCUnreal3DService
// ========================================

	ucHRESULT CreateUCUnreal3DService(ucHANDLE hDLL, UC3DService** pp3D)
	{
		if (!pp3D)
			return ucE_FAIL;

		UCUnreal3DService* pService = new UCUnreal3DService();
		if (!pService)
			return ucE_FAIL;

		*pp3D = pService;
		return ucS_OK;
	}

	UCUnreal3DService::UCUnreal3DService()
	{
	}

	UCUnreal3DService::~UCUnreal3DService()
	{
		// UCUnreal3DService 是一个工厂类，没有成员变量需要释放
		// 它创建的 Device 对象通过 Create3DDevice 返回给调用者，由调用者管理生命周期
		// Service 本身不持有任何资源的所有权，因此析构函数为空是正确的
	}

	ucHRESULT UCUnreal3DService::Create3DDevice(
		UC3DDevice** ppDevice,
		ucHANDLE hWnd,
		ucUINT AdapterToUse,
		ucBOOL bWindow,
		ucINT nWidth,
		ucINT nHeight,
		ucINT nScreenWidth,
		ucINT nScreenHeight,
		ucBOOL bFSA,
		ucDWORD dwDSFormat)
	{
		if (ppDevice == ucNULL)
			return ucE_FAIL;

		UCUnreal3DDevice* pDevice = new UCUnreal3DDevice();
		if (pDevice == ucNULL)
			return ucE_FAIL;

		// 调用 Device 的 Create 函数进行常规初始化
		ucHRESULT hr = pDevice->Create(hWnd, AdapterToUse, bWindow, nWidth, nHeight, nScreenWidth, nScreenHeight, bFSA, dwDSFormat);
		if (ucFAILED(hr))
		{
			delete pDevice;
			return hr;
		}

		*ppDevice = pDevice;

		// Bridge device to global overlay (global UI)
		FKartGameGlobalOverlay::SetDevice(pDevice);

		return ucS_OK;
	}

	ucINT UCUnreal3DService::GetAdapterCount()
	{
		// TODO: 枚举 Unreal 渲染设备
		return 1;
	}

	UC3DServiceType UCUnreal3DService::GetServiceType() ucCONST
	{
		return UC3D_SERVICE_MODERN;
	}

// ========================================
// UCUnreal3DDevice
// ========================================

UCUnreal3DDevice::UCUnreal3DDevice()
	: m_WindowWidth(0)
	, m_WindowHeight(0)
	, m_ScreenWidth(0)
	, m_ScreenHeight(0)
	, m_bFullScreen(ucFALSE)
	, m_CurrentFrameIndex(0)
	, m_pSharedStagingBuffer(nullptr)
	, m_pSharedStagingBufferMappedData(nullptr)
	, m_pSharedVertexBuffer(nullptr)
	, m_UIData()
	, m_pPipelineStateCache(nullptr)
{
	// 初始化 Command Buffer 数组
	for (ucINT i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		m_Frames[i].CommandBuffer = nullptr;
		m_Frames[i].StagingBufferOffset = 0;
		m_Frames[i].VertexBufferOffset = 0;
	}

	// 初始化绑定的资源
	for (ucINT i = 0; i < 16; ++i)
		m_BoundTextures[i] = nullptr;
	m_BoundVertexBuffer = nullptr;
	m_BoundIndexBuffer = nullptr;

	// 初始化默认状态
	m_CurrentBlendState = UC3DModernBlendStateDesc();
	m_CurrentDepthStencilState = UC3DModernDepthStencilStateDesc();
	m_CurrentRasterizerState = UC3DModernRasterizerStateDesc();
	m_CurrentViewport = UC3DModernViewportDesc();
	m_CurrentScissorRect = UCFRect();

	GetData()->DeviceType = 4;  // Unreal3D Device Type
}

UCUnreal3DDevice::~UCUnreal3DDevice()
{
	// 清理绑定的资源引用
	for (ucINT i = 0; i < 16; ++i)
		m_BoundTextures[i] = nullptr;
	m_BoundVertexBuffer = nullptr;
	m_BoundIndexBuffer = nullptr;

	// 清理 UI Pipeline States（TODO: 实现）
	
	// 清理 Unreal 资源
	CleanupUnrealResources();

	// 清理共享 Buffer
	if (m_pSharedStagingBuffer)
	{
		if (m_pSharedStagingBufferMappedData)
		{
			m_pSharedStagingBuffer->Unmap(nullptr);
		}
		m_pSharedStagingBuffer->Release();
		m_pSharedStagingBuffer = nullptr;
		m_pSharedStagingBufferMappedData = nullptr;
	}
	if (m_pSharedVertexBuffer)
	{
		// TODO: 释放 Vertex Buffer
		m_pSharedVertexBuffer = nullptr;
	}

	// 清理 Command Buffer
	for (ucINT i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (m_Frames[i].CommandBuffer)
		{
			DestroyCommandBuffer(m_Frames[i].CommandBuffer);
			m_Frames[i].CommandBuffer = nullptr;
		}
	}
}

ucLPCTSTR UCUnreal3DDevice::GetRendererName() const
{
	return _T("UCUnreal3D");
}

ucHRESULT UCUnreal3DDevice::Create(
	ucHANDLE hWnd,
	ucUINT AdapterToUse,
	ucBOOL bWindow,
	ucINT nWidth,
	ucINT nHeight,
	ucINT nScreenWidth,
	ucINT nScreenHeight,
	ucBOOL bFSA,
	ucDWORD dwDSFormat)
{
	m_WindowWidth = nWidth;
	m_WindowHeight = nHeight;
	m_ScreenWidth = nScreenWidth;
	m_ScreenHeight = nScreenHeight;
	m_bFullScreen = bFSA;

	// 初始化 Unreal 资源
	ucHRESULT hr = InitializeUnrealResources(hWnd);
	if (ucFAILED(hr))
		return hr;

	// TODO: 创建共享 Buffer
	// TODO: 创建 Command Buffer
	// TODO: 初始化 Pipeline State Cache

	return ucS_OK;
}

UC3DModernCommandBuffer* UCUnreal3DDevice::CreateCommandBuffer()
{
	UCUnreal3DCommandBuffer* pCmdBuffer = new UCUnreal3DCommandBuffer(this);
	return pCmdBuffer;
}

ucVOID UCUnreal3DDevice::DestroyCommandBuffer(UC3DModernCommandBuffer* pCmdBuffer)
{
	if (pCmdBuffer == nullptr)
		return;

	UCUnreal3DCommandBuffer* pUnrealCmdBuffer = (UCUnreal3DCommandBuffer*)pCmdBuffer;
	delete pUnrealCmdBuffer;
}

UC3DModernCommandBuffer* UCUnreal3DDevice::GetCurrentCommandBuffer()
{
	return m_Frames[m_CurrentFrameIndex].CommandBuffer;
}

// GetModernData() 由基类实现，不需要重写

UC3DDeviceDataBase::UC3DDataMng_UI* UCUnreal3DDevice::GetUIData()
{
	return &m_UIData;
}

ucUINT UCUnreal3DDevice::GetScreenWidth()
{
	return m_ScreenWidth;
}

ucUINT UCUnreal3DDevice::GetScreenHeight()
{
	return m_ScreenHeight;
}

ucUINT UCUnreal3DDevice::GetBackBufferWidth()
{
	return m_WindowWidth;
}

ucUINT UCUnreal3DDevice::GetBackBufferHeight()
{
	return m_WindowHeight;
}

ucVOID UCUnreal3DDevice::SetDrawScale(ucFLOAT fScale)
{
	m_UIData.Scale = fScale;
}

ucVOID UCUnreal3DDevice::ResetFontData()
{
	m_UIData.FontReSize = ucTRUE;
}

// ========== 资源创建接口（占位实现）==========

ucHRESULT UCUnreal3DDevice::CreateTexture(
	UC3DModernTexture** ppTexture,
	UC3DPublicDataPtr*,
	ucUINT Width,
	ucUINT Height,
	ucUINT Levels,
	ucDWORD Usage,
	ucDWORD Format,
	ucDWORD Pool,
	ucINT iMultiSamples,
	ucBOOL bLinear)
{
	if (ppTexture == nullptr || Width == 0 || Height == 0)
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 创建 UCUnreal3DTexture 对象
	UCUnreal3DTexture* pTexture = new UCUnreal3DTexture();
	if (pTexture == nullptr)
		return ucE_FAIL;

	pTexture->m_pDevice = this;
	pTexture->m_Width = Width;
	pTexture->m_Height = Height;
	pTexture->m_Levels = (Levels > 0) ? Levels : 1;
	pTexture->m_Format = (UC3DFORMAT)Format;
	pTexture->m_Name = UCString(_T("UCUnreal3DTexture"));

	// 转换 UC3D 格式到 Unreal EPixelFormat
	EPixelFormat UnrealFormat = PF_B8G8R8A8;  // 默认格式
	switch (Format)
	{
	case UC3DFMT_A8:
		UnrealFormat = PF_G8;
		break;
	case UC3DFMT_A8R8G8B8:
	case UC3DFMT_X8R8G8B8:
		UnrealFormat = PF_B8G8R8A8;
		break;
	case UC3DFMT_DXT1:
		UnrealFormat = PF_DXT1;
		break;
	case UC3DFMT_DXT3:
		UnrealFormat = PF_DXT3;
		break;
	case UC3DFMT_DXT5:
		UnrealFormat = PF_DXT5;
		break;
	default:
		UnrealFormat = PF_B8G8R8A8;
		break;
	}

	// 在渲染线程中创建纹理
	FTextureRHIRef TextureRHI;
	ENQUEUE_RENDER_COMMAND(CreateTextureCommand)(
		[Width, Height, UnrealFormat, Levels, &TextureRHI](FRHICommandListImmediate& RHICmdList)
		{
			FRHITextureCreateDesc CreateDesc =
				FRHITextureCreateDesc::Create2D(TEXT("UCUnreal3DTexture"), Width, Height, UnrealFormat)
				.SetNumMips(Levels)
				.SetFlags(ETextureCreateFlags::ShaderResource);

			TextureRHI = RHICreateTexture(CreateDesc);
		});

	// 等待渲染命令完成
	FlushRenderingCommands();

	if (!TextureRHI.IsValid())
	{
		delete pTexture;
		return ucE_FAIL;
	}

	// 设置 RHI Texture
	pTexture->SetRHITexture(TextureRHI);

	// 初始化纹理为黑色
	// 根据格式计算每像素字节数
	ucUINT bytesPerPixel = 4;  // 默认 BGRA
	switch (UnrealFormat)
	{
	case PF_G8:
	case PF_A8:
		bytesPerPixel = 1;
		break;
	case PF_B8G8R8A8:
	case PF_R8G8B8A8:
		bytesPerPixel = 4;
		break;
	default:
		bytesPerPixel = 4;
		break;
	}

	// 创建全黑的数据缓冲区
	ucUINT dataSize = Width * Height * bytesPerPixel;
	TArray<uint8> BlackData;
	BlackData.AddZeroed(dataSize);  // 初始化为0（黑色）

	// 使用 UpdateRegion 将整个纹理初始化为黑色
	UCPoint Offset = { 0, 0 };
	UCSize Size = { (ucINT)Width, (ucINT)Height };
	ucHRESULT hr = pTexture->UpdateRegion(nullptr, 0, Offset, Size, BlackData.GetData());
	if (hr != ucS_OK)
	{
		// 如果更新失败，仍然返回成功（纹理已创建，只是未初始化为黑色）
		// 可以根据需要决定是否返回错误
	}

	*ppTexture = pTexture;
	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DDevice::CreateTextureFromFile(
	UC3DModernTexture** ppTexture,
	UC3DPublicDataPtr*,
	ucLPCTSTR pSrcFile,
	UCPackFileInfo* pPackFileInfo,
	ucBOOL bUsedThread)
{
	if (ppTexture == nullptr || pSrcFile == nullptr)
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 读取文件到内存
	UCArray<ucBYTE, ucBYTE> FileData;
	// TODO: 实现文件读取逻辑（可能需要使用Unreal的文件系统）
	// 这里先返回失败，等待文件系统集成
	return ucE_FAIL;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DDevice::CreateTextureFromMemory(
	UC3DModernTexture** ppTexture,
	UC3DPublicDataPtr*,
	ucLPVOID pSrcMem,
	ucINT iLength,
	ucUINT Width,
	ucUINT Height,
	ucUINT MipLevels,
	ucDWORD Usage,
	ucDWORD Format,
	ucDWORD Pool,
	ucDWORD Filter,
	ucDWORD MipFilter,
	UCCreateTextureArgsExt* pTextureArgsExt,
	UC3DCOLOR ColorKey,
	UC3DIMAGE_INFO* pSrcInfo)
{
	if (ppTexture == nullptr || pSrcMem == nullptr || iLength <= 0)
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 如果提供了尺寸和格式，直接使用
	if (Width > 0 && Height > 0 && Format != UC3DFMT_UNKNOWN)
	{
		// 创建纹理
		ucHRESULT hr = CreateTexture(ppTexture, nullptr, Width, Height, MipLevels, Usage, Format, Pool, 0, ucTRUE);
		if (ucFAILED(hr))
			return hr;

		// 上传数据
		UCUnreal3DTexture* pTexture = (UCUnreal3DTexture*)(*ppTexture);
		if (pTexture)
		{
			// 计算行间距
			ucUINT BytesPerPixel = 4;  // 默认RGBA
			if (Format == UC3DFMT_A8)
				BytesPerPixel = 1;
			else if (Format == UC3DFMT_R8G8B8 || Format == UC3DFMT_X8R8G8B8)
				BytesPerPixel = 3;

			ucUINT RowPitch = Width * BytesPerPixel;
			
			// TODO: 使用UpdateSubresource上传数据
			// pTexture->UpdateSubresource(nullptr, 0, nullptr, pSrcMem, RowPitch, 0);
		}
		return hr;
	}

	// 尝试从内存数据解析图像（PNG/JPEG等）
	// 使用Unreal的IImageWrapper模块解析
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	
	// 检测图像格式
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(pSrcMem, iLength);
	if (ImageFormat == EImageFormat::Invalid)
	{
		return ucE_FAIL;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(pSrcMem, iLength))
	{
		return ucE_FAIL;
	}

	// 获取解压后的RGBA数据
	TArray<uint8> UncompressedBGRA;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
	{
		return ucE_FAIL;
	}

	Width = ImageWrapper->GetWidth();
	Height = ImageWrapper->GetHeight();

	// 创建纹理
	ucHRESULT hr = CreateTexture(ppTexture, nullptr, Width, Height, 1, Usage, UC3DFMT_A8R8G8B8, Pool, 0, ucTRUE);
	if (ucFAILED(hr))
		return hr;

	// 上传数据到纹理
	UCUnreal3DTexture* pTexture = (UCUnreal3DTexture*)(*ppTexture);
	if (pTexture && pTexture->GetRHITexture().IsValid())
	{
		FTextureRHIRef TextureRHI = pTexture->GetRHITexture();
		ENQUEUE_RENDER_COMMAND(UploadTextureDataCommand)(
			[TextureRHI, Width, Height, UncompressedBGRA = MoveTemp(UncompressedBGRA)](FRHICommandListImmediate& RHICmdList)
			{
				uint32 Stride = 0;
				void* TextureData = RHICmdList.LockTexture2D(TextureRHI, 0, RLM_WriteOnly, Stride, false);
				if (TextureData)
				{
					FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
					RHICmdList.UnlockTexture2D(TextureRHI, 0, false);
				}
			});

		FlushRenderingCommands();
	}

	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DDevice::CreateDepthStencilSurface(
	UC3DModernSurface** ppSurface,
	UC3DPublicDataPtr*,
	ucUINT Width,
	ucUINT Height,
	ucDWORD Format,
	ucDWORD MultiSample,
	ucDWORD MultisampleQuality,
	ucBOOL Discard)
{
	// TODO: 实现深度模板表面创建
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::CreateVertexBuffer(
	ucUINT Length,
	ucUINT Stride,
	ucDWORD Usage,
	ucCONST ucVOID* pInitialData,
	UC3DModernVertexBuffer** ppVertexBuffer,
	UC3DPublicDataPtr*)
{
	if (ppVertexBuffer == nullptr)
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 创建 UCUnreal3DVertexBuffer 对象
	UCUnreal3DVertexBuffer* pVertexBuffer = new UCUnreal3DVertexBuffer();
	if (pVertexBuffer == nullptr)
		return ucE_FAIL;
	
	pVertexBuffer->m_pDevice = this;
	pVertexBuffer->m_Size = Length;
	pVertexBuffer->m_Stride = Stride;
	pVertexBuffer->m_Usage = Usage | UC3D_BUFFER_USAGE_VERTEX_BUFFER;
	
	// 使用 Unreal RHI 创建 Vertex Buffer
	// 在渲染线程中创建
	FBufferRHIRef VertexBufferRHI;
	
	// 如果有初始数据，需要先复制一份（因为 lambda 可能在不同线程执行）
	UCArray<ucBYTE, ucBYTE> InitialDataCopy;
	if (pInitialData != nullptr)
	{
		InitialDataCopy.SetSize(Length);
		memcpy(InitialDataCopy.GetData(), pInitialData, Length);
	}
	
	ENQUEUE_RENDER_COMMAND(CreateVertexBufferCommand)(
		[Length, Usage, &InitialDataCopy, &VertexBufferRHI](FRHICommandListImmediate& RHICmdList)
		{
			// 创建 Vertex Buffer
			FRHIResourceCreateInfo CreateInfo(TEXT("UCUnreal3DVertexBuffer"));
			
			// 根据 Usage 标志选择 Buffer 类型
			// UC3D 的 usage 目前只有 Vertex/Index/Uniform/Storage/TransferSrc/TransferDst（没有 Dynamic/Static）
			// 这里做一个简单映射：
			// - 有 TRANSFER_DST：通常是 GPU 本地资源，倾向 BUF_Static
			// - 否则：倾向 BUF_Volatile（共享大 VB 的场景也更贴近 Volatile）
			EBufferUsageFlags BufferUsage = BUF_None;
			if (Usage & UC3D_BUFFER_USAGE_TRANSFER_DST)
				BufferUsage |= BUF_Static;
			else
				BufferUsage |= BUF_Volatile;
			
			VertexBufferRHI = RHICmdList.CreateVertexBuffer(
				Length,
				BufferUsage | EBufferUsageFlags::VertexBuffer,
				CreateInfo
			);

			// 如果有初始数据，直接 Lock/Copy/Unlock 写入
			if (VertexBufferRHI.IsValid() && InitialDataCopy.GetSize() > 0)
			{
				void* Dest = RHICmdList.LockBuffer(
					VertexBufferRHI.GetReference(),
					0,
					(uint32)InitialDataCopy.GetSize(),
					RLM_WriteOnly
				);
				if (Dest != nullptr)
				{
					memcpy(Dest, InitialDataCopy.GetData(), (size_t)InitialDataCopy.GetSize());
					RHICmdList.UnlockBuffer(VertexBufferRHI.GetReference());
				}
			}
		});
	
	// 等待渲染命令完成
	FlushRenderingCommands();
	
	if (!VertexBufferRHI.IsValid())
	{
		delete pVertexBuffer;
		return ucE_FAIL;
	}
	
	// 设置 RHI Buffer
	pVertexBuffer->SetRHIBuffer(VertexBufferRHI);
	
	// 注意：如果通过 ResourceArray 传递，数据已经上传，不需要再次上传
	
	// 注册 Buffer
	RegisterBuffer(pVertexBuffer);
	*ppVertexBuffer = pVertexBuffer;
	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DDevice::CreateIndexBuffer(
	ucUINT Length,
	ucDWORD Usage,
	UC3DFORMAT Format,
	ucCONST ucVOID* pInitialData,
	UC3DModernIndexBuffer** ppIndexBuffer,
	UC3DPublicDataPtr*)
{
	// TODO: 实现索引缓冲区创建
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::CreateStagingBuffer(
	ucUINT Size,
	ucDWORD Usage,
	ucCONST ucVOID* pInitialData,
	UC3DModernBuffer** ppBuffer)
{
	if (ppBuffer == nullptr)
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 创建 UCUnreal3DStagingBuffer 对象
	UCUnreal3DStagingBuffer* pStagingBuffer = new UCUnreal3DStagingBuffer();
	if (pStagingBuffer == nullptr)
		return ucE_FAIL;
	
	pStagingBuffer->m_pDevice = this;
	pStagingBuffer->m_Size = Size;
	pStagingBuffer->m_Usage = Usage | UC3D_BUFFER_USAGE_TRANSFER_SRC;
	
	// 使用 Unreal RHI 创建 Staging Buffer
	// 在渲染线程中创建
	FBufferRHIRef StagingBufferRHI;
	ENQUEUE_RENDER_COMMAND(CreateStagingBufferCommand)(
		[Size, &StagingBufferRHI](FRHICommandListImmediate& RHICmdList)
		{
			// 创建 Staging Buffer（CPU 可写，用于上传数据到 GPU）
			FRHIResourceCreateInfo CreateInfo(TEXT("UCUnreal3DStagingBuffer"));
			
			// 使用 BUF_Volatile 标志，表示 Buffer 内容会频繁更新
			// Staging Buffer 需要 CPU 可访问，使用 VertexBuffer 创建
			// BUF_Volatile 表示 Buffer 内容会频繁更新，通常用于 CPU 可写的 Buffer
			StagingBufferRHI = RHICmdList.CreateVertexBuffer(
				Size,
				BUF_Volatile,  // BUF_Volatile 表示 CPU 可访问，内容会频繁更新
				CreateInfo
			);
		});
	
	// 等待渲染命令完成
	FlushRenderingCommands();
	
	if (!StagingBufferRHI.IsValid())
	{
		delete pStagingBuffer;
		return ucE_FAIL;
	}
	
	// 设置 RHI Buffer
	pStagingBuffer->SetRHIBuffer(StagingBufferRHI);
	
	// 如果有初始数据，上传数据
	if (pInitialData != nullptr)
	{
		ucHRESULT hr = pStagingBuffer->Update(nullptr, 0, Size, pInitialData);
		if (ucFAILED(hr))
		{
			delete pStagingBuffer;
			return hr;
		}
	}
	
	// 注册 Buffer
	RegisterBuffer(pStagingBuffer);
	*ppBuffer = pStagingBuffer;
	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DDevice::CreateUniformBuffer(
	ucUINT Size,
	ucDWORD Usage,
	ucCONST ucVOID* pInitialData,
	UC3DModernBuffer** ppBuffer)
{
	// TODO: 实现 Uniform Buffer 创建
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::CreateStorageBuffer(
	ucUINT Size,
	ucDWORD Usage,
	ucCONST ucVOID* pInitialData,
	UC3DModernBuffer** ppBuffer)
{
	// TODO: 实现 Storage Buffer 创建
	return ucE_FAIL;
}

// ========== Buffer 资源管理 ==========

ucVOID UCUnreal3DDevice::RegisterBuffer(UC3DModernBuffer* pBuffer)
{
	if (pBuffer)
		m_Buffers[pBuffer] = pBuffer;
}

ucVOID UCUnreal3DDevice::UnregisterBuffer(UC3DModernBuffer* pBuffer)
{
	if (pBuffer)
		m_Buffers.Remove(pBuffer);
}

ucVOID UCUnreal3DDevice::TrackBufferUsage(UC3DModernBuffer* pBuffer)
{
	// TODO: 实现 Buffer 使用跟踪
}

ucVOID UCUnreal3DDevice::ReleaseAllBuffers()
{
	// TODO: 释放所有 Buffer
	m_Buffers.RemoveAll();
}

// ========== Pipeline State ==========

ucHRESULT UCUnreal3DDevice::CreatePipelineState(
	ucCONST UC3DModernPipelineStateDesc& Desc,
	UC3DModernPipelineState** ppPipelineState)
{
	// TODO: 实现 Pipeline State 创建
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::CreatePipelineStateFromEffect(
	UC3DModernEffect* pEffect,
	UC3DVertexDeclaration* pVertexDecl,
	ucDWORD FVF,
	UC3DPRIMITIVETYPE PrimitiveType,
	UC3DModernPipelineState** ppPipelineState)
{
	// TODO: 实现从 Effect 创建 Pipeline State
	return ucE_FAIL;
}

// ========== UI 渲染接口 ==========

ucHRESULT UCUnreal3DDevice::DrawRectsBatch(
	ucCONST UCFRect* pRects,
	ucUINT Count,
	ucDWORD dwBack[4],
	ucCONST UCFRect& ClipRect)
{
	if (pRects == nullptr || Count == 0)
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 获取 UI Scale
	UC3DDeviceDataBase::UC3DDataMng_UI* pUIData = GetUIData();
	ucFLOAT UIScale = (pUIData != nullptr && pUIData->Scale > 0.0f) ? pUIData->Scale : 1.0f;

	// 获取窗口高度（用于 Y 坐标翻转）
	ucUINT windowHeight = GetBackBufferHeight();
	if (windowHeight == 0)
		windowHeight = GetScreenHeight();

	// 转换颜色格式：ARGB -> FColor(R,G,B,A)
	auto ToFColor = [pUIData](ucDWORD ColorARGB) -> FColor
	{
		if (pUIData != nullptr && pUIData->Alpha != 0xff)
		{
			ucBYTE a = (ucBYTE)(ColorARGB >> 24);
			a = (ucBYTE)((ucDWORD)a * (ucDWORD)pUIData->Alpha / 255);
			ColorARGB = ((ucDWORD)a << 24) | (ColorARGB & 0x00ffffff);
		}
		return FColor(
			(ColorARGB >> 16) & 0xFF,
			(ColorARGB >> 8) & 0xFF,
			(ColorARGB)&0xFF,
			(ColorARGB >> 24) & 0xFF);
	};

	FColor CornerColors[4] = {
		ToFColor(dwBack[0]), // LT
		ToFColor(dwBack[1]), // RT
		ToFColor(dwBack[2]), // LB
		ToFColor(dwBack[3])  // RB
	};

	// 计算总顶点数（每个矩形需要6个顶点：2个三角形）
	ucUINT totalVertexCount = Count * 6;
	
	// 在游戏线程先准备一份“CPU 顶点数据拷贝”，然后传给渲染线程上传到共享 VB
	UCArray<ucBYTE, ucBYTE> VertexBytes;
	const ucUINT vertexDataSize = totalVertexCount * sizeof(FUCUnrealUIRectVertex);
	VertexBytes.SetSize(vertexDataSize);
	FUCUnrealUIRectVertex* OutVerts = reinterpret_cast<FUCUnrealUIRectVertex*>(VertexBytes.GetData());
	ucUINT vertexIndex = 0;

	// 计算 NDC 转换系数
	// NDC X: [0, windowWidth] -> [-1, 1]
	// NDC Y: [0, windowHeight] -> [1, -1] (Y 轴翻转，因为屏幕 Y 向下，NDC Y 向上)
	ucFLOAT ndcScaleX = 2.0f / (ucFLOAT)m_WindowWidth;
	ucFLOAT ndcScaleY = 2.0f / (ucFLOAT)m_WindowHeight;
	ucFLOAT ndcOffsetX = -1.0f;

	for (ucUINT i = 0; i < Count; i++)
	{
		ucCONST UCFRect& rect = pRects[i];

		// 应用 Scale 到矩形坐标（屏幕空间）
		ucFLOAT scaledLeft = floorf(UIScale * rect.left);
		ucFLOAT scaledTop = floorf(UIScale * rect.top);
		ucFLOAT scaledRight = floorf(UIScale * rect.right);
		ucFLOAT scaledBottom = floorf(UIScale * rect.bottom);

		// 转换为 NDC 坐标
		// X: (screenX / windowWidth) * 2 - 1
		ucFLOAT ndcLeft = scaledLeft * ndcScaleX + ndcOffsetX;
		ucFLOAT ndcRight = scaledRight * ndcScaleX + ndcOffsetX;
		// Y: 1 - (screenY / windowHeight) * 2  (翻转 Y 轴)
		ucFLOAT ndcTop = 1.0f - scaledTop * ndcScaleY;
		ucFLOAT ndcBottom = 1.0f - scaledBottom * ndcScaleY;

		// 创建矩形的两个三角形（6个顶点），使用四个角的颜色
		// 第一个三角形：左上、右上、左下
		OutVerts[vertexIndex].Position = FVector2f(ndcLeft, ndcTop);
		OutVerts[vertexIndex].UV = FVector2f(0.0f, 0.0f);
		OutVerts[vertexIndex].Color = CornerColors[0];
		vertexIndex++;

		OutVerts[vertexIndex].Position = FVector2f(ndcRight, ndcTop);
		OutVerts[vertexIndex].UV = FVector2f(1.0f, 0.0f);
		OutVerts[vertexIndex].Color = CornerColors[1];
		vertexIndex++;

		OutVerts[vertexIndex].Position = FVector2f(ndcLeft, ndcBottom);
		OutVerts[vertexIndex].UV = FVector2f(0.0f, 1.0f);
		OutVerts[vertexIndex].Color = CornerColors[2];
		vertexIndex++;

		// 第二个三角形：右上、右下、左下
		OutVerts[vertexIndex].Position = FVector2f(ndcRight, ndcTop);
		OutVerts[vertexIndex].UV = FVector2f(1.0f, 0.0f);
		OutVerts[vertexIndex].Color = CornerColors[1];
		vertexIndex++;

		OutVerts[vertexIndex].Position = FVector2f(ndcRight, ndcBottom);
		OutVerts[vertexIndex].UV = FVector2f(1.0f, 1.0f);
		OutVerts[vertexIndex].Color = CornerColors[3];
		vertexIndex++;

		OutVerts[vertexIndex].Position = FVector2f(ndcLeft, ndcBottom);
		OutVerts[vertexIndex].UV = FVector2f(0.0f, 1.0f);
		OutVerts[vertexIndex].Color = CornerColors[2];
		vertexIndex++;
	}

	// 不在这里直接 RHI 画：把 batch 交给 Slate overlay（UI pass）去绘制，避免被场景覆盖。
	FKartGameGlobalOverlay::FUIBatch Batch;
	Batch.VertexStride = sizeof(FUCUnrealUIRectVertex);
	Batch.VertexCount = totalVertexCount;
	Batch.VertexData.SetNumUninitialized((int32)vertexDataSize);
	FMemory::Memcpy(Batch.VertexData.GetData(), VertexBytes.GetData(), vertexDataSize);
	FKartGameGlobalOverlay::AppendBatch(MoveTemp(Batch));
	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DDevice::ColorFill(ucCONST UCFRect& Rect, ucDWORD dwBack, ucCONST UCFRect& ClipRect)
{
	// Unreal 后端：这里不做清屏（清屏由 Unreal 场景/渲染管线负责）。
	// 为了兼容老 UC3D 调用链（ColorFill 可能被当作背景填充/清屏），直接返回成功。
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::DrawVerticesWithTexture(
	ucCONST UCFontVertex* pVertices,
	ucUINT VertexCount,
	UC3DModernTexture* pTexture,
	ucDWORD dwColor,
	ucFLOAT fZ)
{
	if (pVertices == nullptr || VertexCount == 0 || pTexture == nullptr)
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 转换为 UCUnreal3DTexture
	UCUnreal3DTexture* pUnrealTexture = (UCUnreal3DTexture*)(pTexture);
	if (pUnrealTexture == nullptr || !pUnrealTexture->IsValid())
		return ucE_FAIL;

	// 获取纹理格式
	UC3DFORMAT textureFormat = pTexture->GetFormat();
	ucBOOL bIsA8Format = (textureFormat == UC3DFMT_A8);

	// 获取 UI Scale
	UC3DDeviceDataBase::UC3DDataMng_UI* pUIData = GetUIData();
	ucFLOAT UIScale = (pUIData != nullptr && pUIData->Scale > 0.0f) ? pUIData->Scale : 1.0f;

	// 获取窗口高度（用于 Y 坐标翻转）
	ucUINT windowHeight = GetBackBufferHeight();
	if (windowHeight == 0)
		windowHeight = GetScreenHeight();

	// 转换 UCFontVertex 到 FUCUnrealUIRectVertex
	// UCFontVertex 应该包含 Position (x, y), UV (u, v), Color 等信息
	// 假设 UCFontVertex 结构类似：struct { float x, y; float u, v; DWORD color; }
	const ucUINT vertexDataSize = VertexCount * sizeof(FUCUnrealUIRectVertex);
	UCArray<ucBYTE, ucBYTE> VertexBytes;
	VertexBytes.SetSize(vertexDataSize);
	FUCUnrealUIRectVertex* OutVerts = reinterpret_cast<FUCUnrealUIRectVertex*>(VertexBytes.GetData());

	// 提取颜色分量
	ucBYTE a = (ucBYTE)((dwColor >> 24) & 0xFF);
	ucBYTE r = (ucBYTE)((dwColor >> 16) & 0xFF);
	ucBYTE g = (ucBYTE)((dwColor >> 8) & 0xFF);
	ucBYTE b = (ucBYTE)(dwColor & 0xFF);
	
	if (pUIData != nullptr && pUIData->Alpha != 0xff)
	{
		a = (ucBYTE)((ucDWORD)a * (ucDWORD)pUIData->Alpha / 255);
	}

	FColor VertexColor(r, g, b, a);

	// 计算 NDC 转换系数
	// NDC X: [0, windowWidth] -> [-1, 1]
	// NDC Y: [0, windowHeight] -> [1, -1] (Y 轴翻转，因为屏幕 Y 向下，NDC Y 向上)
	ucFLOAT ndcScaleX = 2.0f / (ucFLOAT)m_WindowWidth;
	ucFLOAT ndcScaleY = 2.0f / (ucFLOAT)m_WindowHeight;
	ucFLOAT ndcOffsetX = -1.0f;

	ucFLOAT ScaleY = (ucFLOAT)m_WindowHeight / 2048.0f;
	// 转换顶点数据
	// UCFontVertex 结构包含 Pos (uc3dxVector4) 和 UV (uc3dxVector2)
	for (ucUINT i = 0; i < VertexCount; ++i)
	{
		const UCFontVertex& InVert = pVertices[i];
		
		// 应用 Scale 到坐标（屏幕空间）
		ucFLOAT scaledX = InVert.Pos.x;
		ucFLOAT scaledY = InVert.Pos.y;

		// 转换为 NDC 坐标
		// X: (screenX / windowWidth) * 2 - 1
		ucFLOAT ndcX = scaledX * ndcScaleX + ndcOffsetX;
		// Y: 1 - (screenY / windowHeight) * 2  (翻转 Y 轴)
		ucFLOAT ndcY = 1.0f - scaledY * ndcScaleY;

		OutVerts[i].Position = FVector2f(ndcX, ndcY);
		OutVerts[i].UV = FVector2f(InVert.UV.x, InVert.UV.y);
		
		// UCFontVertex 没有颜色字段，使用全局颜色
		OutVerts[i].Color = VertexColor;
	}

	// 将批次添加到全局 Overlay（类似 DrawRectsBatch 的方式）
	FKartGameGlobalOverlay::FUIBatch Batch;
	Batch.VertexStride = sizeof(FUCUnrealUIRectVertex);
	Batch.VertexCount = VertexCount;
	Batch.VertexData.SetNumUninitialized((int32)vertexDataSize);
	FMemory::Memcpy(Batch.VertexData.GetData(), VertexBytes.GetData(), vertexDataSize);
	
	// 传递纹理信息到 Batch，以便在渲染时绑定纹理
	Batch.TextureRHI = pUnrealTexture->GetRHITexture();
	Batch.TextureFormat = (int32)textureFormat;  // 转换为 int32 以避免头文件依赖
	
	FKartGameGlobalOverlay::AppendBatch(MoveTemp(Batch));
	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucVOID UCUnreal3DDevice::Draw9Patch(
	ucCONST UCFRect& Rect,
	UC3DModernTexture* pTexture,
	ucLPVOID ImageData,
	ucCONST uc3dxVector2& vScaling,
	ucCONST UCFPoint& Offset,
	ucCONST UCFPoint& Center,
	ucCONST UCFRect& ClipRect,
	ucDWORD bAlpha,
	ucCONST UCIntIntMap& mapU,
	ucCONST UCIntIntMap& mapV,
	ucFLOAT fZ,
	ucBYTE iMirror)
{
	// TODO: 实现九宫格绘制
}

// ========== Pipeline State 设置 ==========

ucHRESULT UCUnreal3DDevice::SetBlendState(const UC3DModernBlendStateDesc& desc)
{
	m_CurrentBlendState = desc;
	// TODO: 应用混合状态到 Unreal
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::SetDepthStencilState(const UC3DModernDepthStencilStateDesc& desc)
{
	m_CurrentDepthStencilState = desc;
	// TODO: 应用深度模板状态到 Unreal
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::SetRasterizerState(const UC3DModernRasterizerStateDesc& desc)
{
	m_CurrentRasterizerState = desc;
	// TODO: 应用光栅化状态到 Unreal
	return ucS_OK;
}

// ========== 视口和裁剪 ==========

ucHRESULT UCUnreal3DDevice::SetViewport(const UC3DModernViewportDesc& viewport)
{
	m_CurrentViewport = viewport;
	// TODO: 应用视口到 Unreal
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::GetViewport(UC3DModernViewportDesc& viewport)
{
	viewport = m_CurrentViewport;
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::SetScissorRect(ucCONST UCFRect& rect)
{
	m_CurrentScissorRect = rect;
	// TODO: 应用裁剪矩形到 Unreal
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::GetScissorRect(UCFRect& rect)
{
	rect = m_CurrentScissorRect;
	return ucS_OK;
}

// ========== 渲染目标 ==========

ucHRESULT UCUnreal3DDevice::SetRenderTarget(ucDWORD RenderTargetIndex, UC3DModernSurface* pRenderTarget)
{
	// TODO: 实现渲染目标设置
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::GetRenderTarget(ucDWORD RenderTargetIndex, UC3DModernSurface** ppRenderTarget, UC3DPublicDataPtr* pPublicDataPtr)
{
	// TODO: 实现渲染目标获取
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::SetDepthStencilSurface(UC3DModernSurface* pNewZStencil)
{
	// TODO: 实现深度模板表面设置
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::GetDepthStencilSurface(UC3DModernSurface** ppZStencilSurface, UC3DPublicDataPtr* pPublicDataPtr)
{
	// TODO: 实现深度模板表面获取
	return ucE_FAIL;
}

// ========== 绘制命令 ==========

ucHRESULT UCUnreal3DDevice::DrawPrimitive(
	UC3DPRIMITIVETYPE PrimitiveType,
	ucUINT StartVertex,
	ucUINT PrimitiveCount)
{
	// TODO: 实现图元绘制
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::DrawIndexedPrimitive(
	UC3DPRIMITIVETYPE Type,
	ucINT BaseVertexIndex,
	ucUINT MinIndex,
	ucUINT NumVertices,
	ucUINT StartIndex,
	ucUINT PrimitiveCount)
{
	// TODO: 实现索引图元绘制
	return ucE_FAIL;
}

// ========== 场景管理 ==========

ucHRESULT UCUnreal3DDevice::BeginScene()
{
	// TODO: 开始场景渲染
	// UI overlay frame begins here (clear previous UI batches)
	FKartGameGlobalOverlay::BeginFrame();
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::EndScene()
{
	// TODO: 结束场景渲染
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::Flush()
{
	// TODO: 刷新命令缓冲区
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::Present(
	ucCONST RECT* pSourceRect,
	ucCONST RECT* pDestRect,
	ucHANDLE hDestWindowOverride,
	ucCONST ucRGNDATA* pDirtyRegion)
{
	// TODO: 实现 Present
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::Clear(
	ucDWORD Flags,
	ucDWORD Color,
	ucFLOAT Z,
	ucDWORD Stencil)
{
	// TODO: 实现清除
	return ucS_OK;
}

// ========== 设备能力 ==========

ucVOID UCUnreal3DDevice::TestDeviceCapability()
{
	// TODO: 测试设备能力
}

UCString UCUnreal3DDevice::GetErrorDescription(ucLONG error)
{
	// TODO: 返回错误描述
	return UCString();
}

// ========== 重置 ==========

ucHRESULT UCUnreal3DDevice::Reset(
	ucUINT nWindowWidth,
	ucUINT nWindowHeight,
	ucUINT nScreenWidth,
	ucUINT nScreenHeight,
	ucBOOL bFullScreen,
	ucBOOL bFSA)
{
	m_WindowWidth = nWindowWidth;
	m_WindowHeight = nWindowHeight;
	m_ScreenWidth = nScreenWidth;
	m_ScreenHeight = nScreenHeight;
	m_bFullScreen = bFullScreen;

	// TODO: 重置 Unreal 资源
	return ucS_OK;
}

// ========== 字体 ==========

ucHRESULT UCUnreal3DDevice::CreateDefFont(
	UC3DPlaneFont** ppFont,
	UC3DPublicDataPtr* pPublicDataPtr,
	ucCONST UC3DCreateFont& nFont)
{
	// 参考 UCVulkan3DDevice::CreateDefFont 的实现
	if (ppFont == nullptr)
		return ucE_FAIL;

	UCUnreal3DPlaneFont* pFont = UCUnreal3DPlaneFont::Create(this, pPublicDataPtr, nFont);
	if (pFont == nullptr)
		return ucE_FAIL;

	*ppFont = pFont;
	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::CreateFont(
	UC3DPlaneFont** ppFont,
	ucCONST UC3DCreateFont& createInfo)
{
	if (ppFont == nullptr)
		return ucE_FAIL;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	// 使用静态 Create 方法创建字体（类似 UCVulkan3DPlaneFont）
	UCUnreal3DPlaneFont* pFont = UCUnreal3DPlaneFont::Create(this, nullptr, createInfo);
	if (pFont == nullptr)
		return ucE_FAIL;

	*ppFont = pFont;
	return ucS_OK;
#else
	return ucE_FAIL;
#endif
}

ucHRESULT UCUnreal3DDevice::CreateEntityEmpty(UC3DEntity** ppEntity)
{
	// TODO: 实现空实体创建
	return ucE_FAIL;
}

ucHRESULT UCUnreal3DDevice::CreateEffectFromFile(
	ucLPCTSTR pSrcFile,
	ucCONST UC3DMACRO* pDefines,
	ucDWORD Flags,
	UC3DModernEffect** ppEffect,
	UC3DPublicDataPtr* pEffectData,
	UC3DModernBuffer** ppCompilationErrors)
{
	// TODO: 实现从文件创建 Effect
	return ucE_FAIL;
}

ucFLOAT UCUnreal3DDevice::GetCharHeight(
	UC3DPlaneFont* pFont,
	ucINT iHeight,
	ucINT iOutlineSize)
{
	if (pFont == nullptr)
		return 0.0f;

#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	UCUnreal3DPlaneFont* pUnrealFont = (UCUnreal3DPlaneFont*)(pFont);
	if (pUnrealFont == nullptr)
		return 0.0f;

	// 使用 Slate 字体测量系统获取字符高度
	const FSlateFontInfo& FontInfo = pUnrealFont->GetSlateFontInfo();
	
	// 使用实际高度参数（如果提供）
	int32 FontSize = (iHeight > 0) ? iHeight : FontInfo.Size;
	FSlateFontInfo MeasureFontInfo = FontInfo;
	MeasureFontInfo.Size = FontSize;

	// 使用 FSlateApplication 的字体测量服务
	if (FSlateApplication::IsInitialized())
	{
		TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		
		// 测量字符高度（使用大写字母A作为参考）
		FText TestText = FText::FromString(TEXT("A"));
		FVector2D TextSize = FontMeasure->Measure(TestText, MeasureFontInfo);
		
		// 如果指定了描边大小，需要额外增加高度
		float Height = (float)TextSize.Y;
		if (iOutlineSize > 0)
		{
			Height += (float)(iOutlineSize * 2);  // 上下各增加描边大小
		}

		return Height;
	}
	
	// 如果 Slate 未初始化，返回字体大小的估算值
	return (float)((iHeight > 0) ? iHeight : FontInfo.Size);
#else
	return 0.0f;
#endif
}

ucFLOAT UCUnreal3DDevice::GetCharWidth(
	UC3DPlaneFont* pFont,
	ucINT iWidth,
	ucINT iOutlineSize,
	ucTCHAR szChar)
{
	// TODO: 获取字符宽度
	return 0.0f;
}

// ========== 共享 Buffer 管理 ==========

ucHRESULT UCUnreal3DDevice::GetSharedStagingBuffer(
	ucUINT DataSize,
	ucUINT Alignment,
	UC3DModernBuffer** ppStagingBuffer,
	ucVOID** ppMappedData,
	ucUINT* pOffset)
{
	if (ppStagingBuffer == nullptr || ppMappedData == nullptr || pOffset == nullptr)
		return ucE_FAIL;

	// 计算对齐后的大小
	ucUINT alignedSize = (DataSize + Alignment - 1) & ~(Alignment - 1);

	// 获取当前帧的偏移量
	ucUINT& currentOffset = m_Frames[m_CurrentFrameIndex].StagingBufferOffset;

	// 如果 Buffer 不存在，自动创建
	if (m_pSharedStagingBuffer == nullptr || m_pSharedStagingBufferMappedData == nullptr)
	{
		// 计算需要的 Buffer 大小（至少是请求大小的 2 倍，或者使用默认大小）
		ucUINT bufferSize = (alignedSize > SHARED_STAGING_BUFFER_SIZE) ? 
			((alignedSize + SHARED_STAGING_BUFFER_SIZE - 1) / SHARED_STAGING_BUFFER_SIZE) * SHARED_STAGING_BUFFER_SIZE : 
			SHARED_STAGING_BUFFER_SIZE;

		ucHRESULT hr = CreateStagingBuffer(bufferSize, UC3D_BUFFER_USAGE_TRANSFER_SRC, nullptr, 
			&m_pSharedStagingBuffer);
		if (ucFAILED(hr))
			return ucE_FAIL;

		// 持久映射 Staging Buffer 内存（用于快速访问）
		// 注意：Unreal 的 Buffer 映射需要在渲染线程中执行
		// 对于共享 Staging Buffer，我们需要持久映射以便快速访问
		ucVOID* pMappedData = nullptr;
		hr = m_pSharedStagingBuffer->Map(nullptr, 0, 0, &pMappedData);
		if (ucFAILED(hr))
		{
			m_pSharedStagingBuffer->Release();
			m_pSharedStagingBuffer = nullptr;
			m_pSharedStagingBufferMappedData = nullptr;
			return ucE_FAIL;
		}

		m_pSharedStagingBufferMappedData = pMappedData;

		// 重置偏移量
		currentOffset = 0;
	}
	else
	{
		// 检查当前 Buffer 大小是否足够
		ucUINT currentBufferSize = m_pSharedStagingBuffer->GetSize();

		// 如果空间不够，需要扩容
		if (currentOffset + alignedSize > currentBufferSize)
		{
			// 如果单次数据就超过当前缓冲区大小，需要扩容
			if (alignedSize > currentBufferSize)
			{
				// 计算新的 Buffer 大小（至少是请求大小的 2 倍）
				ucUINT newBufferSize = ((alignedSize + currentBufferSize - 1) / currentBufferSize) * currentBufferSize * 2;

				// 释放旧的 Buffer
				if (m_pSharedStagingBufferMappedData != nullptr)
				{
					m_pSharedStagingBuffer->Unmap(nullptr);
					m_pSharedStagingBufferMappedData = nullptr;
				}
				m_pSharedStagingBuffer->Release();
				m_pSharedStagingBuffer = nullptr;

				// 创建新的更大的 Buffer
				ucHRESULT hr = CreateStagingBuffer(newBufferSize, UC3D_BUFFER_USAGE_TRANSFER_SRC, nullptr, 
					&m_pSharedStagingBuffer);
				if (ucFAILED(hr))
					return ucE_FAIL;

				// 持久映射新的 Buffer
				ucVOID* pMappedData = nullptr;
				hr = m_pSharedStagingBuffer->Map(nullptr, 0, 0, &pMappedData);
				if (ucFAILED(hr))
				{
					m_pSharedStagingBuffer->Release();
					m_pSharedStagingBuffer = nullptr;
					m_pSharedStagingBufferMappedData = nullptr;
					return ucE_FAIL;
				}
				m_pSharedStagingBufferMappedData = pMappedData;

				// 重置偏移量
				currentOffset = 0;
			}
			else
			{
				// 空间不够但数据可以放下，重置偏移量（回绕到开头）
				currentOffset = 0;
			}
		}
	}

	// 如果映射数据为空，返回错误（等待 MapBuffer 实现）
	if (m_pSharedStagingBufferMappedData == nullptr)
		return ucE_FAIL;

	// 返回共享 Buffer 和偏移量
	*ppStagingBuffer = m_pSharedStagingBuffer;
	*ppMappedData = m_pSharedStagingBufferMappedData;
	*pOffset = currentOffset;

	// 更新偏移量
	currentOffset += alignedSize;

	return ucS_OK;
}

ucHRESULT UCUnreal3DDevice::GetSharedVertexBuffer(
	ucUINT DataSize,
	ucUINT Stride,
	ucUINT Alignment,
	UC3DModernVertexBuffer** ppVertexBuffer,
	ucUINT* pOffset)
{
	if (ppVertexBuffer == nullptr || pOffset == nullptr)
		return ucE_FAIL;

	// 计算对齐后的大小
	ucUINT alignedSize = (DataSize + Alignment - 1) & ~(Alignment - 1);

	// 获取当前帧的偏移量
	ucUINT& currentOffset = m_Frames[m_CurrentFrameIndex].VertexBufferOffset;

	// 如果 Buffer 不存在，自动创建
	if (m_pSharedVertexBuffer == nullptr)
	{
		// 计算需要的 Buffer 大小（至少是请求大小的 2 倍，或者使用默认大小）
		ucUINT bufferSize = (alignedSize > SHARED_VERTEX_BUFFER_SIZE) ? 
			((alignedSize + SHARED_VERTEX_BUFFER_SIZE - 1) / SHARED_VERTEX_BUFFER_SIZE) * SHARED_VERTEX_BUFFER_SIZE : 
			SHARED_VERTEX_BUFFER_SIZE;

		ucHRESULT hr = CreateVertexBuffer(bufferSize, Stride, 0, nullptr, 
			&m_pSharedVertexBuffer, nullptr);
		if (ucFAILED(hr))
			return ucE_FAIL;

		// 重置偏移量
		currentOffset = 0;
	}
	else
	{
		// 检查当前 Buffer 大小是否足够
		ucUINT currentBufferSize = m_pSharedVertexBuffer->GetSize();

		// 如果空间不够，需要扩容
		if (currentOffset + alignedSize > currentBufferSize)
		{
			// 如果单次数据就超过当前缓冲区大小，需要扩容
			if (alignedSize > currentBufferSize)
			{
				// 计算新的 Buffer 大小（至少是请求大小的 2 倍）
				ucUINT newBufferSize = ((alignedSize + currentBufferSize - 1) / currentBufferSize) * currentBufferSize * 2;

				// 释放旧的 Buffer
				m_pSharedVertexBuffer->Release();
				m_pSharedVertexBuffer = nullptr;

				// 创建新的更大的 Buffer
				ucHRESULT hr = CreateVertexBuffer(newBufferSize, Stride, 0, nullptr, 
					&m_pSharedVertexBuffer, nullptr);
				if (ucFAILED(hr))
					return ucE_FAIL;

				// 重置偏移量
				currentOffset = 0;
			}
			else
			{
				// 空间不够但数据可以放下，重置偏移量（回绕到开头）
				currentOffset = 0;
			}
		}
	}

	// 返回共享 Buffer 和偏移量
	*ppVertexBuffer = m_pSharedVertexBuffer;
	*pOffset = currentOffset;

	// 更新偏移量
	currentOffset += alignedSize;

	return ucS_OK;
}

ucVOID UCUnreal3DDevice::ResetSharedBufferOffsets()
{
	// 重置当前帧的偏移量（在 BeginScene 或 EndScene 中调用）
	m_Frames[m_CurrentFrameIndex].StagingBufferOffset = 0;
	m_Frames[m_CurrentFrameIndex].VertexBufferOffset = 0;
}

// ========== 私有辅助函数 ==========

ucHRESULT UCUnreal3DDevice::InitializeUnrealResources(ucHANDLE hWnd)
{
	// TODO: 初始化 Unreal 相关资源
	// 例如：获取 UWorld、创建 RenderTarget 等
	
	// 初始化 UI Pipeline States（TODO: 实现）
	
	return ucS_OK;
}

ucVOID UCUnreal3DDevice::CleanupUnrealResources()
{
	// TODO: 清理 Unreal 相关资源
}

ucHRESULT UCUnreal3DDevice::ConvertFormat(UC3DFORMAT format, void* pOutFormat)
{
	// TODO: 转换格式
	return ucE_FAIL;
}

UC3DFORMAT UCUnreal3DDevice::ConvertFormat(void* pFormat)
{
	// TODO: 转换格式
	return UC3DFMT_UNKNOWN;
}

UC3D_END

// ========================================
// 导出函数实现
// ========================================

extern "C" {
	// 统一的Service创建接口，返回UC3DService基类指针
	ucHRESULT ucCreate3DService(UC3D::UC3DService** pp3D)
	{
		if (pp3D == nullptr)
			return ucE_FAIL;

		UC3D::UCUnreal3DService* pService = new UC3D::UCUnreal3DService();
		if (pService == nullptr)
			return ucE_FAIL;

		*pp3D = pService;
		return ucS_OK;
	}
}

