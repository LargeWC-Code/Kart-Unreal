/********************************************************************
created:	2024/12/XX
filename: 	VoxelWorldEditor.cpp
author:		Auto Generated

purpose:	AVoxelWorldEditor - 编辑器专用体素世界类实现
*********************************************************************/

#include "VoxelWorldEditor.h"
#include "VoxelWorldEditorMapsWidget.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformFile.h"
#include "Misc/Paths.h"
#if WITH_EDITOR
#include "Misc/FileHelper.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Modules/ModuleManager.h"
#endif

AVoxelWorldEditor::AVoxelWorldEditor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	// 使用 ProjectContentDir 作为基础路径（编辑器模式下可读写）
	FString MapsDir = FPaths::ProjectContentDir() / TEXT("VoxelWorld/World");
	FString MapFile = MapsDir / TEXT("AllMaps.wjson");
	
	// 确保目录存在
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*MapsDir))
	{
		PlatformFile.CreateDirectoryTree(*MapsDir);
	}
	
	// 转换路径为 UCString
	UCString UCMapFile = UCString(*MapFile);
	
	// 如果文件不存在，创建一个空文件
	if (!MapManager.LoadFromFile(UCMapFile))
	{
		MapManager.SaveToFile(UCMapFile);
	}
#endif
}

#if WITH_EDITOR
void AVoxelWorldEditor::ShowMapsWindow()
{
	// 创建窗口
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(NSLOCTEXT("VoxelEditor", "MapsWindowTitle", "地图管理"))
		.ClientSize(FVector2D(400, 500))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	// 创建内容
	TSharedRef<SVoxelWorldEditorMapsWidget> MapsWidget = SNew(SVoxelWorldEditorMapsWidget)
		.VoxelWorldEditor(this)
		.ParentWindow(Window);

	Window->SetContent(MapsWidget);

	// 显示窗口
	FSlateApplication::Get().AddWindow(Window);
}
#endif

void AVoxelWorldEditor::BeginPlay()
{
	Super::BeginPlay();

	// 注意：在编辑器模式下，窗口在 Enter() 时通过 VoxelEditorEditorMode 显示
	// BeginPlay 只在 PIE（Play In Editor）时调用
}

void AVoxelWorldEditor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#if WITH_EDITOR
void AVoxelWorldEditor::EditorVisualizeDebug()
{
	UE_LOG(LogTemp, Log, TEXT("VoxelWorldEditor: EditorVisualizeDebug called"));
	// TODO: 实现编辑器可视化调试功能
}

void AVoxelWorldEditor::EditorQuickTest()
{
	UE_LOG(LogTemp, Log, TEXT("VoxelWorldEditor: EditorQuickTest called"));
	// TODO: 实现编辑器快速测试功能
}
#endif

