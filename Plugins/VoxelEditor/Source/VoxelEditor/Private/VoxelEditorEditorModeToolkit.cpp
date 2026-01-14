// Copyright Epic Games, Inc. All Rights Reserved.

// 对应的头文件必须作为第一个包含
#include "VoxelEditorEditorModeToolkit.h"

// 确保在包含任何可能间接包含Windows.h的头文件之前，先包含CoreMinimal
#include "CoreMinimal.h"

#include "VoxelEditorEditorMode.h"
#include "VoxelEditorEditorModeCommands.h"
#include "VoxelWorldEditorMapsWidget.h"
#include "VoxelWorldEditor.h"
#include "InteractiveToolManager.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/Guid.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateColor.h"
#include "Styling/CoreStyle.h"
#include "VoxelEditVolume.h"
#include "VoxelTerrain.h"
#include "VoxelMap.h"
#include "VoxelBlockTypes.h"
#include "Editor.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"
#include "Widgets/Images/SImage.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Slate/SlateBrushAsset.h"
#include "Styling/SlateBrush.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformFile.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "VoxelEditorEditorModeToolkit"

FVoxelEditorEditorModeToolkit::FVoxelEditorEditorModeToolkit()
{
	// 初始化状态
	CurrentMapWidth = 0;
	CurrentMapHeight = 0;
	bMapLoaded = false;
	EditToolButtonStates.Empty();
	SelectedBlockTypeIndex = VOXEL_BLOCK_TYPE_PAINT_TEXTURE;
}

void FVoxelEditorEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);

	// 保存 OwningMode，供 GetInlineContent() 使用
	OwningMode = InOwningMode;

	// 检查是否已有 VoxelWorldEditor 实例
	AVoxelWorldEditor* NewVoxelWorldEditorInstance = nullptr;
	if (UVoxelEditorEditorMode* EditorMode = Cast<UVoxelEditorEditorMode>(InOwningMode.Get()))
	{
		// 先检查是否已有实例
		NewVoxelWorldEditorInstance = EditorMode->GetVoxelWorldEditorInstance();
		
		// 如果已有实例且有效，直接使用
		if (NewVoxelWorldEditorInstance && IsValid(NewVoxelWorldEditorInstance))
		{
			UE_LOG(LogTemp, Log, TEXT("VoxelEditorToolkit: Reusing existing VoxelWorldEditor instance"));
		}
		else
		{
			// 如果没有现存的实例，创建新的
			if (UWorld* World = EditorMode->GetWorld())
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Name = FName(TEXT("VoxelWorldEditor"));
				NewVoxelWorldEditorInstance = World->SpawnActor<AVoxelWorldEditor>(SpawnParams);
				if (NewVoxelWorldEditorInstance)
				{
					// 设置文件夹路径：VoxelWorld
					NewVoxelWorldEditorInstance->SetFolderPath(FName(TEXT("VoxelWorld")));
					
					UE_LOG(LogTemp, Log, TEXT("VoxelEditorToolkit: Created VoxelWorldEditor instance in Init"));
					// 将实例保存到 EditorMode 中
					EditorMode->SetVoxelWorldEditorInstance(NewVoxelWorldEditorInstance);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("VoxelEditorToolkit: Failed to create VoxelWorldEditor instance"));
				}
			}
		}
	}

	// 保存 VoxelWorldEditor 实例，供 GetInlineContent() 使用
	VoxelWorldEditorInstance = NewVoxelWorldEditorInstance;

	// 确保只有一个 VoxelEditVolume 存在
	if (UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr)
	{
		TArray<AVoxelEditVolume*> EditVolumes;
		for (TActorIterator<AVoxelEditVolume> ActorItr(World); ActorItr; ++ActorItr)
		{
			AVoxelEditVolume* EditVolume = *ActorItr;
			if (IsValid(EditVolume))
			{
				EditVolumes.Add(EditVolume);
			}
		}

		// 如果存在多个或没有，处理
		if (EditVolumes.Num() > 1)
		{
			// 保留第一个，删除其他的
			for (int32 i = 1; i < EditVolumes.Num(); ++i)
			{
				World->DestroyActor(EditVolumes[i]);
				UE_LOG(LogTemp, Log, TEXT("VoxelEditorToolkit: Removed duplicate VoxelEditVolume"));
			}
		}
		else if (EditVolumes.Num() == 0)
		{
			// 如果没有，创建一个默认的
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("VoxelEditVolume"));
			AVoxelEditVolume* NewEditVolume = World->SpawnActor<AVoxelEditVolume>(SpawnParams);
			if (NewEditVolume)
			{
				NewEditVolume->SetFolderPath(FName(TEXT("VoxelWorld")));
				UE_LOG(LogTemp, Log, TEXT("VoxelEditorToolkit: Created default VoxelEditVolume"));
			}
		}
	}
}

TSharedPtr<SWidget> FVoxelEditorEditorModeToolkit::GetInlineContent() const
{
	// 如果还没有创建主 Widget，则创建它
	if (!MainContentWidget.IsValid())
	{
		MainContentWidget = CreateToolContentWidget();
	}
	
	return MainContentWidget;
}

TSharedPtr<SWidget> FVoxelEditorEditorModeToolkit::CreateToolContentWidget() const
{
	// 创建一个包含 WidgetSwitcher 的 Widget，根据工具状态动态切换
	ToolContentSwitcher = SNew(SWidgetSwitcher)
		.WidgetIndex_Lambda([this]() { return GetActiveToolIndex(); })
		
		// Maps Tool Widget (索引 0)
		+ SWidgetSwitcher::Slot()
		[
			GetMapsToolWidget().ToSharedRef()
		]
		
		// Edit Tool Widget (索引 1)
		+ SWidgetSwitcher::Slot()
		[
			GetEditToolWidget().ToSharedRef()
		];
	
	return ToolContentSwitcher;
}

int32 FVoxelEditorEditorModeToolkit::GetActiveToolIndex() const
{
	// 获取当前激活的工具
	if (OwningMode.IsValid())
	{
		if (UEdMode* EdMode = OwningMode.Get())
		{
			if (UInteractiveToolManager* ToolManager = EdMode->GetToolManager(EToolsContextScope::Default))
			{
				FString ActiveToolName = ToolManager->GetActiveToolName(EToolSide::Left);
				
				// 根据工具名称返回对应的索引
				if (ActiveToolName == UVoxelEditorEditorMode::MapsToolName)
				{
					return 0;  // Maps Tool
				}
				else if (ActiveToolName == UVoxelEditorEditorMode::EditToolName)
				{
					return 1;  // Edit Tool
				}
			}
		}
	}
	
	// 默认返回 Maps Tool
	return 0;
}

TSharedPtr<SWidget> FVoxelEditorEditorModeToolkit::GetMapsToolWidget() const
{
	if (VoxelWorldEditorInstance.IsValid())
	{
		return SNew(SVoxelWorldEditorMapsWidget)
			.VoxelWorldEditor(VoxelWorldEditorInstance.Get());
	}
	else
	{
		return SNew(STextBlock)
			.Text(LOCTEXT("NoVoxelWorldEditor", "Failed to create VoxelWorldEditor"));
	}
}

TSharedPtr<SWidget> FVoxelEditorEditorModeToolkit::GetEditToolWidget() const
{
	// 如果已经创建过，直接返回缓存的 Widget
	if (EditToolWidget.IsValid())
	{
		return EditToolWidget;
	}
	
	// 创建一个带滚动条的区域
	const float VisibleWidth = 800.0f;   // 可视区域宽度
	const float VisibleHeight = 600.0f;  // 可视区域高度
	
	EditToolWidget = SNew(SBorder)
		.Padding(10.0f)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			
			// 编辑体积操作区域
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(SHorizontalBox)
				
				// 清空当前Tile按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 5, 0)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("VoxelEditor", "ClearCurrentTile", "清空地块"))
					.OnClicked_Lambda([this]()
					{
						ClearCurrentTile();
						return FReply::Handled();
					})
				]
				
				// 填充编辑体积按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0, 5, 0)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("VoxelEditor", "FillEditVolume", "填充体积"))
					.OnClicked_Lambda([this]()
					{
						ApplyEditVolume();
						return FReply::Handled();
					})
				]
				
				// 清空编辑体积按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0, 0, 0)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("VoxelEditor", "ClearEditVolume", "清空体积"))
					.OnClicked_Lambda([this]()
					{
						ClearEditVolume();
						return FReply::Handled();
					})
				]
			]
			
			// 地块种类选择按钮组
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(SHorizontalBox)
				
				// Select Block 按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 5, 5, 5)
				[
					SNew(SBox)
					.MinDesiredWidth(60.0f)
					.MinDesiredHeight(30.0f)
					[
						SNew(SOverlay)
						// 背景颜色层
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SColorBlock)
							.Color(MakeAttributeLambda([this]()
							{
								return SelectedBlockTypeIndex == VOXEL_BLOCK_TYPE_SELECT ? 
									FLinearColor(0.2f, 0.5f, 0.8f, 1.0f) :  // 选中：蓝色背景
									FLinearColor(0.15f, 0.15f, 0.15f, 1.0f); // 未选中：深灰背景
							}))
						]
						// 按钮层 - 填充整个区域以扩大点击区域
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SButton)
							.Text(NSLOCTEXT("VoxelEditor", "SelectBlock", "选择"))
							.ButtonStyle(FAppStyle::Get(), "NoBorder")
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.ContentPadding(FMargin(5.0f))
							.OnClicked_Lambda([this]()
							{
								SelectedBlockTypeIndex = VOXEL_BLOCK_TYPE_SELECT;
								UE_LOG(LogTemp, Log, TEXT("Block type selected: Select Block (index: %d)"), SelectedBlockTypeIndex);
								// 刷新EditToolWidget和MainContentWidget以更新按钮状态
								EditToolWidget.Reset();
								MainContentWidget.Reset();
								return FReply::Handled();
							})
						]
					]
				]
				
				// Place Block 按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 5, 5, 5)
				[
					SNew(SBox)
					.MinDesiredWidth(60.0f)
					.MinDesiredHeight(30.0f)
					[
						SNew(SOverlay)
						// 背景颜色层
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SColorBlock)
							.Color(MakeAttributeLambda([this]()
							{
								return SelectedBlockTypeIndex == VOXEL_BLOCK_TYPE_PLACE ? 
									FLinearColor(0.2f, 0.5f, 0.8f, 1.0f) :  // 选中：蓝色背景
									FLinearColor(0.15f, 0.15f, 0.15f, 1.0f); // 未选中：深灰背景
							}))
						]
						// 按钮层 - 填充整个区域以扩大点击区域
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SButton)
							.Text(NSLOCTEXT("VoxelEditor", "PlaceBlock", "方块"))
							.ButtonStyle(FAppStyle::Get(), "NoBorder")
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.ContentPadding(FMargin(5.0f))
							.OnClicked_Lambda([this]()
							{
								SelectedBlockTypeIndex = VOXEL_BLOCK_TYPE_PLACE;
								UE_LOG(LogTemp, Log, TEXT("Block type selected: Place Block (index: %d)"), SelectedBlockTypeIndex);
								// 刷新EditToolWidget和MainContentWidget以更新按钮状态
								EditToolWidget.Reset();
								MainContentWidget.Reset();
								return FReply::Handled();
							})
						]
					]
				]
				
				// Place Square Slope 按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 5, 5, 5)
				[
					SNew(SBox)
					.MinDesiredWidth(60.0f)
					.MinDesiredHeight(30.0f)
					[
						SNew(SOverlay)
						// 背景颜色层
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SColorBlock)
							.Color(MakeAttributeLambda([this]()
							{
								return SelectedBlockTypeIndex == VOXEL_BLOCK_TYPE_PLACE_SQUARE_SLOPE ? 
									FLinearColor(0.2f, 0.5f, 0.8f, 1.0f) :  // 选中：蓝色背景
									FLinearColor(0.15f, 0.15f, 0.15f, 1.0f); // 未选中：深灰背景
							}))
						]
						// 按钮层 - 填充整个区域以扩大点击区域
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SButton)
							.Text(NSLOCTEXT("VoxelEditor", "PlaceSquareSlope", "斜楔"))
							.ButtonStyle(FAppStyle::Get(), "NoBorder")
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.ContentPadding(FMargin(5.0f))
							.OnClicked_Lambda([this]()
							{
								SelectedBlockTypeIndex = VOXEL_BLOCK_TYPE_PLACE_SQUARE_SLOPE;
								UE_LOG(LogTemp, Log, TEXT("Block type selected: Place Square Slope (index: %d)"), SelectedBlockTypeIndex);
								// 刷新EditToolWidget和MainContentWidget以更新按钮状态
								EditToolWidget.Reset();
								MainContentWidget.Reset();
								return FReply::Handled();
							})
						]
					]
				]
				
				// Place Triangular Slope 按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 5, 5, 5)
				[
					SNew(SBox)
					.MinDesiredWidth(60.0f)
					.MinDesiredHeight(30.0f)
					[
						SNew(SOverlay)
						// 背景颜色层
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SColorBlock)
							.Color(MakeAttributeLambda([this]()
							{
								return SelectedBlockTypeIndex == VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_SLOPE ? 
									FLinearColor(0.2f, 0.5f, 0.8f, 1.0f) :  // 选中：蓝色背景
									FLinearColor(0.15f, 0.15f, 0.15f, 1.0f); // 未选中：深灰背景
							}))
						]
						// 按钮层 - 填充整个区域以扩大点击区域
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SButton)
							.Text(NSLOCTEXT("VoxelEditor", "PlaceTriangularSlope", "三角锥"))
							.ButtonStyle(FAppStyle::Get(), "NoBorder")
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.ContentPadding(FMargin(5.0f))
							.OnClicked_Lambda([this]()
							{
								SelectedBlockTypeIndex = VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_SLOPE;
								UE_LOG(LogTemp, Log, TEXT("Block type selected: Place Triangular Slope (index: %d)"), SelectedBlockTypeIndex);
								// 刷新EditToolWidget和MainContentWidget以更新按钮状态
								EditToolWidget.Reset();
								MainContentWidget.Reset();
								return FReply::Handled();
							})
						]
					]
				]
				
				// Place Triangular Complement 按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 5, 5, 5)
				[
					SNew(SBox)
					.MinDesiredWidth(60.0f)
					.MinDesiredHeight(30.0f)
					[
						SNew(SOverlay)
						// 背景颜色层
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SColorBlock)
							.Color(MakeAttributeLambda([this]()
							{
								return SelectedBlockTypeIndex == VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_COMPLEMENT ? 
									FLinearColor(0.2f, 0.5f, 0.8f, 1.0f) :  // 选中：蓝色背景
									FLinearColor(0.15f, 0.15f, 0.15f, 1.0f); // 未选中：深灰背景
							}))
						]
						// 按钮层 - 填充整个区域以扩大点击区域
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SButton)
							.Text(NSLOCTEXT("VoxelEditor", "PlaceTriangularComplement", "三角锥互补"))
							.ButtonStyle(FAppStyle::Get(), "NoBorder")
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.ContentPadding(FMargin(5.0f))
							.OnClicked_Lambda([this]()
							{
								SelectedBlockTypeIndex = VOXEL_BLOCK_TYPE_PLACE_TRIANGULAR_COMPLEMENT;
								UE_LOG(LogTemp, Log, TEXT("Block type selected: Place Triangular Complement (index: %d)"), SelectedBlockTypeIndex);
								// 刷新EditToolWidget和MainContentWidget以更新按钮状态
								EditToolWidget.Reset();
								MainContentWidget.Reset();
								return FReply::Handled();
							})
						]
					]
				]
			]
			
			// 网格区域
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SBox)
				.WidthOverride(VisibleWidth)
				.HeightOverride(VisibleHeight)
				[
					SNew(SScrollBox)
						.Orientation(Orient_Horizontal)
						+ SScrollBox::Slot()
						.Padding(5.0f)
						[
							SNew(SScrollBox)
								.Orientation(Orient_Vertical)
								+ SScrollBox::Slot()
								[
									SAssignNew(EditToolGridPanel, SUniformGridPanel)
								]
						]
				]
			]
			
			// 纹理列表区域
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 10, 0, 0)
			[
				SNew(SBorder)
				.Padding(5.0f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					
					// 标题和涂抹纹理按钮
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 5)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("TextureListTitle", "纹理列表"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(5, 0, 0, 0)
						[
							SNew(SBox)
							.MinDesiredWidth(80.0f)
							.MinDesiredHeight(30.0f)
							[
								SNew(SOverlay)
								// 背景颜色层
								+ SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SColorBlock)
									.Color(MakeAttributeLambda([this]()
									{
										return SelectedBlockTypeIndex == VOXEL_BLOCK_TYPE_PAINT_TEXTURE ? 
											FLinearColor(0.2f, 0.5f, 0.8f, 1.0f) :  // 选中：蓝色背景
											FLinearColor(0.15f, 0.15f, 0.15f, 1.0f); // 未选中：深灰背景
									}))
								]
								// 按钮层 - 填充整个区域以扩大点击区域
								+ SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SButton)
									.Text(NSLOCTEXT("VoxelEditor", "PaintTexture", "涂抹纹理"))
									.ButtonStyle(FAppStyle::Get(), "NoBorder")
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.ContentPadding(FMargin(5.0f))
									.OnClicked_Lambda([this]()
									{
										SelectedBlockTypeIndex = VOXEL_BLOCK_TYPE_PAINT_TEXTURE;
										UE_LOG(LogTemp, Log, TEXT("Block type selected: Paint Texture (index: %d)"), SelectedBlockTypeIndex);
										// 刷新EditToolWidget和MainContentWidget以更新按钮状态
										EditToolWidget.Reset();
										MainContentWidget.Reset();
										return FReply::Handled();
									})
								]
							]
						]
					]
					
					// 添加按钮
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 5)
					[
						SNew(SButton)
						.Text(LOCTEXT("AddTexture", "添加纹理"))
						.OnClicked_Lambda([this]()
						{
							if (VoxelWorldEditorInstance.IsValid())
							{
								AVoxelWorldEditor* WorldEditor = VoxelWorldEditorInstance.Get();
								if (WorldEditor)
								{
									IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
									if (DesktopPlatform)
									{
										TArray<FString> OutFiles;
										const FString Title = TEXT("选择纹理图片");
										const FString DefaultPath = AVoxelWorldEditor::GetVoxelWorldDirectory();
										const FString FileTypes = TEXT("Image Files (*.tga, *.png, *.jpg, *.jpeg)|*.tga;*.png;*.jpg;*.jpeg|All Files (*.*)|*.*");
										
										if (DesktopPlatform->OpenFileDialog(
											FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
											Title,
											DefaultPath,
											TEXT(""),
											FileTypes,
											EFileDialogFlags::None,
											OutFiles
										))
										{
											if (OutFiles.Num() > 0)
											{
												FString SelectedFile = OutFiles[0];
												FString VoxelWorldDir = AVoxelWorldEditor::GetVoxelWorldDirectory();
												
												// 计算相对路径
												FString ResourceDir = VoxelWorldDir / TEXT("Resource");
												FString RelativePath;
												
												// 尝试将路径转换为相对于 Resource 目录的路径
												if (FPaths::MakePathRelativeTo(SelectedFile, *ResourceDir))
												{
													RelativePath = SelectedFile;
												}
												else
												{
													// 如果不在资源目录内，复制到资源目录的 Textures 子目录
													FString FileName = FPaths::GetCleanFilename(SelectedFile);
													FString TargetPath = ResourceDir / TEXT("Textures") / FileName;
													
													// 确保 Textures 目录存在
													IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
													FString TexturesDir = ResourceDir / TEXT("Textures");
													if (!PlatformFile.DirectoryExists(*TexturesDir))
													{
														PlatformFile.CreateDirectoryTree(*TexturesDir);
													}
													
													if (IFileManager::Get().Copy(*TargetPath, *SelectedFile, true, true))
													{
														RelativePath = TEXT("Textures") / FileName;
													}
													else
													{
														RelativePath = FileName;
													}
												}
												
												AddTexture(RelativePath);
												// AddTexture 内部已经处理了刷新
											}
										}
									}
								}
							}
							return FReply::Handled();
						})
					]
					
					// 纹理列表
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						BuildTextureListWidget().ToSharedRef()
						]
				]
			]
		];
	
	// 默认显示空状态（没有加载地图）
	UpdateEditToolGridFromMap(0, 0);
	
	return EditToolWidget;
}

void FVoxelEditorEditorModeToolkit::UpdateEditToolGridFromMap(int32 MapWidth, int32 MapHeight) const
{
	// 更新当前地图尺寸
	CurrentMapWidth = MapWidth;
	CurrentMapHeight = MapHeight;
	bMapLoaded = (MapWidth > 0 && MapHeight > 0);
	
	// 如果网格面板无效，直接返回
	if (!EditToolGridPanel.IsValid())
	{
		return;
	}
	
	// 清空现有内容
	EditToolGridPanel->ClearChildren();
	
	// 如果没有加载地图，显示提示信息
	if (!bMapLoaded)
	{
		EditToolGridPanel->AddSlot(0, 0)
			[
				SNew(SBox)
				.MinDesiredWidth(400.0f)
				.MinDesiredHeight(100.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NoMapLoaded", "请双击地图列表中的地图以加载并显示编辑界面"))
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
				]
			];
		return;
	}
	
	// 根据地图尺寸创建网格
	// 确保状态数组大小正确
	const int32 TotalButtons = MapWidth * MapHeight;
	if (EditToolButtonStates.Num() != TotalButtons)
	{
		EditToolButtonStates.SetNum(TotalButtons, false);
	}
	
	// 创建按钮
	const float ButtonSize = 50.0f;
	
	for (int32 Row = 0; Row < MapHeight; ++Row)
	{
		for (int32 Col = 0; Col < MapWidth; ++Col)
		{
			int32 ButtonIndex = Row * MapWidth + Col;
			// 计算坐标：中心为(0,0)
			// X坐标：从-(MapWidth-1)/2 到 (MapWidth-1)/2
			// Y坐标：从-(MapHeight-1)/2 到 (MapHeight-1)/2（Y轴反转，顶部为正）
			int32 X = Col - (MapWidth - 1) / 2;
			int32 Y = (MapHeight - 1) / 2 - Row; // Y轴反转，使顶部为正
			
			// 创建坐标文本
			FString CoordText = FString::Printf(TEXT("%d,%d"), X, Y);
			
			EditToolGridPanel->AddSlot(Col, Row)
				[
					SNew(SBox)
					.MinDesiredWidth(ButtonSize)
					.MinDesiredHeight(ButtonSize)
					.WidthOverride(ButtonSize)
					.HeightOverride(ButtonSize)
					[
						SNew(SButton)
						.ContentPadding(2.0f)
						.ButtonStyle(FAppStyle::Get(), "NoBorder")
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.OnClicked_Lambda([this, ButtonIndex, MapWidth, MapHeight, X, Y]()
						{
							if (ButtonIndex >= 0 && ButtonIndex < EditToolButtonStates.Num())
							{
								EditToolButtonStates[ButtonIndex] = !EditToolButtonStates[ButtonIndex];
								UE_LOG(LogTemp, Log, TEXT("Edit Tool Button [%d,%d] clicked, Active: %s"), 
									X, Y,
									EditToolButtonStates[ButtonIndex] ? TEXT("true") : TEXT("false"));
								
								// 激活/取消激活对应的地块
								if (VoxelWorldEditorInstance.IsValid())
								{
									AVoxelWorldEditor* WorldEditor = VoxelWorldEditorInstance.Get();
									if (WorldEditor)
									{
										UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
										if (!Terrain)
										{
											Terrain = WorldEditor->CreateTerrain();
										}
										
										if (Terrain && WorldEditor->GetWorld())
										{
											// 获取地图数据（如果已加载）
											const UCVoxelMapData* MapData = nullptr;
											UCVoxelMapManager& MapManager = WorldEditor->GetMapManager();
											if (MapManager.Curr)
											{
												MapData = MapManager.Curr;
											}
											
											// 设置地块激活状态（会自动创建或销毁）
											Terrain->SetTileActive(X, Y, WorldEditor->GetWorld(), EditToolButtonStates[ButtonIndex]);
											UE_LOG(LogTemp, Log, TEXT("Tile [%d,%d] set to %s"), 
												X, Y,
												EditToolButtonStates[ButtonIndex] ? TEXT("Active") : TEXT("Inactive"));
											
											// 如果激活了Tile，更新VoxelEditVolume的位置和大小
											if (EditToolButtonStates[ButtonIndex])
											{
												// 查找VoxelEditVolume
												AVoxelEditVolume* EditVolume = nullptr;
												for (TActorIterator<AVoxelEditVolume> ActorItr(WorldEditor->GetWorld()); ActorItr; ++ActorItr)
												{
													AVoxelEditVolume* Volume = *ActorItr;
													if (IsValid(Volume))
													{
														EditVolume = Volume;
														break;
													}
												}
												
												// 如果找到了EditVolume，更新其位置和大小
												if (EditVolume && EditVolume->BoxComponent)
												{
													// 计算Tile的世界位置（Tile中心）
													// Tile (0,0)的中心在(0, 0, 0)，每个Tile是32*32*64个体素
													const float TileSizeX = (float)VOXEL_TILE_SIZE_X;
													const float TileSizeY = (float)VOXEL_TILE_SIZE_Y;
													const float TileSizeZ = (float)VOXEL_TILE_SIZE_Z;
													const float VoxelSize = Terrain->VoxelSize;
													const float TileWorldSize = TileSizeX * VoxelSize; // 3200厘米
													
													// Tile中心位置：根据之前的代码修改，Tile (0,0)的中心在(0, 0, 0)
													// X和Y方向：从-16*100到+16*100，中心在0
													// Z方向：从0到64*100，中心在32*100=3200
													float TileCenterX = Y * TileWorldSize;
													float TileCenterY = X * TileWorldSize;
													float TileCenterZ = (TileSizeZ * VoxelSize) * 0.5f; // 32 * 100 = 3200
													
													// 设置Volume位置为Tile中心
													FVector VolumePosition(TileCenterX, TileCenterY, TileCenterZ);
													EditVolume->SetActorLocation(VolumePosition);
													
													// 设置Volume大小：3200x3200x6400（厘米），SetBoxExtent使用半尺寸
													FVector BoxExtent(
														(float)(VOXEL_TILE_SIZE_X * VoxelSize) / 2.0f,
														(float)(VOXEL_TILE_SIZE_Y * VoxelSize) / 2.0f,
														(float)(VOXEL_TILE_SIZE_Z * VoxelSize) / 2.0f
													); // 半尺寸：VOXEL_TILE_SIZE_X*VoxelSize/2, VOXEL_TILE_SIZE_Y*VoxelSize/2, VOXEL_TILE_SIZE_Z*VoxelSize/2
													EditVolume->BoxComponent->SetBoxExtent(BoxExtent);
													
													UE_LOG(LogTemp, Log, TEXT("Updated VoxelEditVolume position to Tile [%d,%d] center: (%.2f, %.2f, %.2f), size: (%.2f, %.2f, %.2f)"), 
														X, Y, VolumePosition.X, VolumePosition.Y, VolumePosition.Z, 
														BoxExtent.X * 2.0f, BoxExtent.Y * 2.0f, BoxExtent.Z * 2.0f);
												}
											}
										}
									}
								}
								
								// 触发整个网格面板的刷新，以更新所有按钮的状态显示
								if (EditToolGridPanel.IsValid())
								{
									EditToolGridPanel->Invalidate(EInvalidateWidget::Paint);
								}
							}
							return FReply::Handled();
						})
						[
							SNew(SOverlay)
							// 背景颜色层 - 使用 SColorBlock 显示纯色背景
							+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(SColorBlock)
								.Color(MakeAttributeLambda([this, ButtonIndex]()
								{
									bool bIsActive = (ButtonIndex >= 0 && ButtonIndex < EditToolButtonStates.Num()) 
										? EditToolButtonStates[ButtonIndex] : false;
									// 激活状态：亮绿色，非激活状态：浅灰色
									return bIsActive ? 
										FLinearColor(0.0f, 0.4f, 0.0f, 1.0f) : // 纯亮绿色 RGB(0,255,0)
										FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
								}))
							]
							// 文本层
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(2.0f)
							[
								SNew(STextBlock)
								.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
								.Text(FText::FromString(CoordText))
								.Justification(ETextJustify::Center)
								.AutoWrapText(false)
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
							]
						]
					]
				];
		}
	}
	
	// 触发刷新
	if (EditToolGridPanel.IsValid())
	{
		EditToolGridPanel->Invalidate(EInvalidateWidget::Layout);
	}
}

void FVoxelEditorEditorModeToolkit::ApplyEditVolume() const
{
	// 获取当前世界
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot get editor world"));
		return;
	}

	// 查找第一个有效的编辑体积
	AVoxelEditVolume* SelectedVolume = nullptr;
	for (TActorIterator<AVoxelEditVolume> ActorItr(World); ActorItr; ++ActorItr)
	{
		AVoxelEditVolume* EditVolume = *ActorItr;
		if (IsValid(EditVolume))
		{
			SelectedVolume = EditVolume;
			break;
		}
	}

	if (!SelectedVolume)
	{
		UE_LOG(LogTemp, Warning, TEXT("No edit volume found in the world"));
		return;
	}

	// 获取 VoxelWorldEditor 和 Terrain
	if (!VoxelWorldEditorInstance.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("VoxelWorldEditor instance is not valid"));
		return;
	}

	AVoxelWorldEditor* WorldEditor = VoxelWorldEditorInstance.Get();
	if (!WorldEditor)
	{
		return;
	}

	UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
	if (!Terrain)
	{
		Terrain = WorldEditor->CreateTerrain();
	}

	if (!Terrain)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot get or create Terrain"));
		return;
	}

	// 获取编辑体积的边界框
	FBox VolumeBounds = SelectedVolume->GetVolumeBounds();
	FVector Min = VolumeBounds.Min;
	FVector Max = VolumeBounds.Max;
	
	FVector Dis = VolumeBounds.GetExtent();

	int ExtentX = FMath::RoundToInt(Dis.X / 100.0f);
	int ExtentY = FMath::RoundToInt(Dis.Y / 100.0f);
	int ExtentZ = FMath::RoundToInt(Dis.Z / 100.0f);

	//if (ExtentX % 2 == 1)
	{
		Min.X += 50.0f;
		Max.X -= 50.0f;
	}

	//if (ExtentY % 2 == 1)
	{
		Min.Y += 50.0f;
		Max.Y -= 50.0f;
	}

	//if (ExtentZ % 2 == 1)
	{
		Min.Z += 50.0f;
		Max.Z -= 50.0f;
	}
	// 调用 Terrain 的填充方法
	Terrain->FillRegion(Min, Max, SelectedVolume->VoxelType, SelectedVolume->VoxelLayer, World);
}

void FVoxelEditorEditorModeToolkit::ClearEditVolume() const
{
	// 获取当前世界
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot get editor world"));
		return;
	}

	// 查找第一个有效的编辑体积
	AVoxelEditVolume* SelectedVolume = nullptr;
	for (TActorIterator<AVoxelEditVolume> ActorItr(World); ActorItr; ++ActorItr)
	{
		AVoxelEditVolume* EditVolume = *ActorItr;
		if (IsValid(EditVolume))
		{
			SelectedVolume = EditVolume;
			break;
		}
	}

	if (!SelectedVolume)
	{
		UE_LOG(LogTemp, Warning, TEXT("No edit volume found in the world"));
		return;
	}

	// 获取 VoxelWorldEditor 和 Terrain
	if (!VoxelWorldEditorInstance.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("VoxelWorldEditor instance is not valid"));
		return;
	}

	AVoxelWorldEditor* WorldEditor = VoxelWorldEditorInstance.Get();
	if (!WorldEditor)
	{
		return;
	}

	UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
	if (!Terrain)
	{
		Terrain = WorldEditor->CreateTerrain();
	}

	if (!Terrain)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot get or create Terrain"));
		return;
	}

	// 获取编辑体积的边界框
	FBox VolumeBounds = SelectedVolume->GetVolumeBounds();
	FVector Min = VolumeBounds.Min;
	FVector Max = VolumeBounds.Max;
	
	FVector Dis = VolumeBounds.GetExtent();

	int ExtentX = FMath::RoundToInt(Dis.X / 100.0f);
	int ExtentY = FMath::RoundToInt(Dis.Y / 100.0f);
	int ExtentZ = FMath::RoundToInt(Dis.Z / 100.0f);

	//if (ExtentX % 2 == 1)
	{
		Min.X += 50.0f;
		Max.X -= 50.0f;
	}

	//if (ExtentY % 2 == 1)
	{
		Min.Y += 50.0f;
		Max.Y -= 50.0f;
	}

	//if (ExtentZ % 2 == 1)
	{
		Min.Z += 50.0f;
		Max.Z -= 50.0f;
	}

	// 调用 Terrain 的填充方法，类型为0表示清空
	Terrain->FillRegion(Min, Max, 0, 0, World);
	
	UE_LOG(LogTemp, Log, TEXT("Cleared edit volume region from (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)"), 
		Min.X, Min.Y, Min.Z, Max.X, Max.Y, Max.Z);
}

void FVoxelEditorEditorModeToolkit::ClearCurrentTile() const
{
	// 获取当前世界
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot get editor world"));
		return;
	}

	// 查找第一个有效的编辑体积
	AVoxelEditVolume* SelectedVolume = nullptr;
	for (TActorIterator<AVoxelEditVolume> ActorItr(World); ActorItr; ++ActorItr)
	{
		AVoxelEditVolume* EditVolume = *ActorItr;
		if (IsValid(EditVolume))
		{
			SelectedVolume = EditVolume;
			break;
		}
	}

	if (!SelectedVolume)
	{
		UE_LOG(LogTemp, Warning, TEXT("No edit volume found in the world"));
		return;
	}

	// 获取 VoxelWorldEditor 和 Terrain
	if (!VoxelWorldEditorInstance.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("VoxelWorldEditor instance is not valid"));
		return;
	}

	AVoxelWorldEditor* WorldEditor = VoxelWorldEditorInstance.Get();
	if (!WorldEditor)
	{
		return;
	}

	UVoxelTerrain* Terrain = WorldEditor->GetTerrain();
	if (!Terrain)
	{
		Terrain = WorldEditor->CreateTerrain();
	}

	if (!Terrain)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot get or create Terrain"));
		return;
	}

	// 获取编辑体积的中心位置
	FVector VolumeCenter = SelectedVolume->GetActorLocation();
	float VoxelSize = Terrain->VoxelSize;
	
	// 计算体素坐标
	int32 VoxelX = FMath::FloorToInt(VolumeCenter.X / VoxelSize);
	int32 VoxelY = FMath::FloorToInt(VolumeCenter.Y / VoxelSize);
	
	// 计算Tile坐标
	const int32 TileSizeX = VOXEL_TILE_SIZE_X;
	const int32 TileSizeY = VOXEL_TILE_SIZE_Y;
	const int32 HalfTileSizeX = TileSizeX / 2; // 16
	const int32 HalfTileSizeY = TileSizeY / 2; // 16

	auto FloorDiv = [](int32 Dividend, int32 Divisor) -> int32 {
		if (Dividend >= 0)
			return Dividend / Divisor;
		else
			return (Dividend - Divisor + 1) / Divisor;
	};

	int32 TileX = FloorDiv(VoxelX + HalfTileSizeX, TileSizeX);
	int32 TileY = FloorDiv(VoxelY + HalfTileSizeY, TileSizeY);

	// 计算Tile的世界位置（Tile中心）
	const float TileWorldSize = TileSizeX * VoxelSize; // 3200厘米
	float TileCenterX = TileX * TileWorldSize;
	float TileCenterY = TileY * TileWorldSize;
	
	// Tile的边界：从-16*100到+16*100 (X和Y), 从0到64*100 (Z)
	// Tile (0,0)的中心在(0, 0, 0)，边界从(-1600, -1600, 0)到(+1600, +1600, 6400)
	const float TileSizeZ = (float)VOXEL_TILE_SIZE_Z;
	const float HalfTileWorldSize = HalfTileSizeX * VoxelSize; // 1600厘米
	FVector TileMin = FVector(TileCenterX - HalfTileWorldSize, TileCenterY - HalfTileWorldSize, 0.0f);
	FVector TileMax = FVector(TileCenterX + HalfTileWorldSize, TileCenterY + HalfTileWorldSize, TileSizeZ * VoxelSize);
	
	// 清空整个Tile区域
	Terrain->FillRegion(TileMin, TileMax, 0, 0, World);
	
	UE_LOG(LogTemp, Log, TEXT("Cleared tile [%d, %d] from (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)"), 
		TileX, TileY, TileMin.X, TileMin.Y, TileMin.Z, TileMax.X, TileMax.Y, TileMax.Z);
}

void FVoxelEditorEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}

FName FVoxelEditorEditorModeToolkit::GetToolkitFName() const
{
	return FName("VoxelEditorEditorMode");
}

FText FVoxelEditorEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "VoxelEditorEditorMode Toolkit");
}

int32 FVoxelEditorEditorModeToolkit::GetSelectedBlockType() const
{
	return SelectedBlockTypeIndex;
}

void FVoxelEditorEditorModeToolkit::AddTexture(const FString& TexturePath) const
{
	if (!VoxelWorldEditorInstance.IsValid())
	{
		return;
	}

	AVoxelWorldEditor* WorldEditor = VoxelWorldEditorInstance.Get();
	if (!WorldEditor)
	{
		return;
	}

	UCVoxelMapManager& MapManager = WorldEditor->GetMapManager();
	UCString UCTexturePath = WorldEditor->FStringToUCString(TexturePath);
	
	// 添加到纹理列表
	MapManager.TextureConfig.AryTexturePaths.Add(UCTexturePath);
	
	// 保存配置
	FString VoxelWorldDir = AVoxelWorldEditor::GetVoxelWorldDirectory();
	UCString UCVoxelWorldDir = WorldEditor->FStringToUCString(VoxelWorldDir);
	MapManager.SaveResources(UCVoxelWorldDir);
	
	UE_LOG(LogTemp, Log, TEXT("Added texture: %s"), *TexturePath);
	
	// 强制刷新UI - 重置Widget以便下次调用时重新创建
	EditToolWidget.Reset();
	MainContentWidget.Reset();
	ToolContentSwitcher.Reset();
	
	// 如果当前正在显示编辑工具，立即重新创建Widget
	if (GetActiveToolIndex() == 1) // Edit Tool
	{
		MainContentWidget = CreateToolContentWidget();
		// 强制刷新 WidgetSwitcher
		if (ToolContentSwitcher.IsValid())
		{
			ToolContentSwitcher->Invalidate(EInvalidateWidget::LayoutAndVolatility);
		}
		// 强制刷新整个工具包内容
		if (ToolkitWidget.IsValid())
		{
			ToolkitWidget->Invalidate(EInvalidateWidget::LayoutAndVolatility);
		}
	}
}

void FVoxelEditorEditorModeToolkit::RemoveTexture(int32 Index) const
{
	if (!VoxelWorldEditorInstance.IsValid())
	{
		return;
	}

	AVoxelWorldEditor* WorldEditor = VoxelWorldEditorInstance.Get();
	if (!WorldEditor)
	{
		return;
	}

	UCVoxelMapManager& MapManager = WorldEditor->GetMapManager();
	
	if (Index >= 0 && Index < MapManager.TextureConfig.AryTexturePaths.GetSize())
	{
		MapManager.TextureConfig.AryTexturePaths.RemoveAt(Index);
		
		// 保存配置
		FString VoxelWorldDir = AVoxelWorldEditor::GetVoxelWorldDirectory();
		UCString UCVoxelWorldDir = WorldEditor->FStringToUCString(VoxelWorldDir);
		MapManager.SaveResources(UCVoxelWorldDir);
		
		UE_LOG(LogTemp, Log, TEXT("Removed texture at index: %d"), Index);
		
		// 强制刷新UI - 重置Widget以便下次调用时重新创建
		EditToolWidget.Reset();
		MainContentWidget.Reset();
		ToolContentSwitcher.Reset();
		
		// 如果当前正在显示编辑工具，立即重新创建Widget
		if (GetActiveToolIndex() == 1) // Edit Tool
		{
			MainContentWidget = CreateToolContentWidget();
			// 强制刷新 WidgetSwitcher
			if (ToolContentSwitcher.IsValid())
			{
				ToolContentSwitcher->Invalidate(EInvalidateWidget::LayoutAndVolatility);
			}
			// 强制刷新整个工具包内容
			if (ToolkitWidget.IsValid())
			{
				ToolkitWidget->Invalidate(EInvalidateWidget::LayoutAndVolatility);
			}
		}
	}
}

TSharedPtr<SWidget> FVoxelEditorEditorModeToolkit::BuildTextureListWidget() const
{
	TSharedPtr<SVerticalBox> VerticalBox = SNew(SVerticalBox);

	if (VoxelWorldEditorInstance.IsValid())
	{
		AVoxelWorldEditor* WorldEditor = VoxelWorldEditorInstance.Get();
		if (WorldEditor)
		{
			UCVoxelMapManager& MapManager = WorldEditor->GetMapManager();
			int32 TextureCount = MapManager.TextureConfig.AryTexturePaths.GetSize();
			
			// 为每个纹理创建一个 Widget
			for (int32 i = 0; i < TextureCount; ++i)
			{
				// 创建纹理项 Widget
				int32 TextureIndex = i; // 捕获索引
				
				// 为每个纹理创建画笔（不使用静态缓存，每次都重新加载以确保刷新）
				const FSlateBrush* Brush = nullptr;
				
				// 加载纹理
				UCString TexturePathUC = MapManager.TextureConfig.AryTexturePaths[TextureIndex];
				FString TextureRelativePath = WorldEditor->UCStringToFString(TexturePathUC);
				if (!TextureRelativePath.IsEmpty())
				{
					FString VoxelWorldDir = AVoxelWorldEditor::GetVoxelWorldDirectory();
					FString FullTexturePath = VoxelWorldDir / TextureRelativePath;
					
					IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
					if (PlatformFile.FileExists(*FullTexturePath))
					{
						TArray<uint8> ImageData;
						if (FFileHelper::LoadFileToArray(ImageData, *FullTexturePath))
						{
							UTexture2D* Texture = FImageUtils::ImportBufferAsTexture2D(ImageData);
							if (Texture)
							{
								// 获取纹理尺寸
								int32 TextureWidth = Texture->GetSizeX();
								int32 TextureHeight = Texture->GetSizeY();
								float AspectRatio = (float)TextureWidth / (float)TextureHeight;
								
								// 根据长宽比计算UV区域
								FBox2f UVRegion;
								if (AspectRatio >= 1.9f) // 长度是宽度的两倍或更多（2:1或更大）
								{
									// 显示左上角：宽度1/8，高度1/4
									UVRegion = FBox2f(
										FVector2f(0.0f, 0.0f),      // Min: 左上角
										FVector2f(0.125f, 0.25f)    // Max: 宽度1/8，高度1/4
									);
								}
								else // 长宽相等或接近（1:1）
								{
									// 显示左上角1/4
									UVRegion = FBox2f(
										FVector2f(0.0f, 0.0f),      // Min: 左上角
										FVector2f(0.5f, 0.5f)       // Max: 宽度1/2，高度1/2
									);
								}
								
								// 创建一个自定义的Brush类来设置UV区域
								// 由于FSlateBrush的UVRegion是protected，我们创建一个继承类
								struct FCustomSlateBrush : public FSlateBrush
								{
									FCustomSlateBrush(UTexture2D* InTexture, const FBox2f& InUVRegion)
									{
										SetResourceObject(InTexture);
										ImageSize = FVector2D(InTexture->GetSizeX(), InTexture->GetSizeY());
										DrawAs = ESlateBrushDrawType::Image;
										Tiling = ESlateBrushTileType::NoTile;
										UVRegion = InUVRegion; // 在构造函数中可以访问protected成员
									}
								};
								
								// 创建自定义画笔
								TSharedPtr<FCustomSlateBrush> CustomBrush = MakeShareable(
									new FCustomSlateBrush(Texture, UVRegion)
								);
								
								// 将画笔保存到静态缓存中（使用路径作为键）
								static TMap<FString, TSharedPtr<FSlateBrush>> TextureBrushCache;
								TextureBrushCache.Add(FullTexturePath, CustomBrush);
								Brush = CustomBrush.Get();
							}
						}
					}
				}
				
				// 如果没有加载成功，使用默认的灰色画笔
				if (!Brush)
				{
					Brush = FAppStyle::GetBrush("WhiteBrush");
				}
				
				VerticalBox->AddSlot()
					.AutoHeight()
					.Padding(0, 5, 0, 0)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "NoBorder")
						.ContentPadding(0.0f)
						.OnClicked_Lambda([this, TextureIndex]()
						{
							SelectTexture(TextureIndex);
							return FReply::Handled();
						})
						[
							SNew(SBorder)
							// 根据是否选中来改变边框和背景
							.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
							.BorderBackgroundColor_Lambda([this, TextureIndex]()
							{
								if (SelectedTextureIndex == TextureIndex)
								{
									// 选中状态：蓝色背景
									return FLinearColor(0.2f, 0.5f, 0.8f, 0.6f);
								}
								else
								{
									// 未选中状态：默认背景
									return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
								}
							})
							.Padding(8.0f)
							[
								SNew(SHorizontalBox)
								// 左侧：纹理预览图
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(0, 0, 10, 0)
								[
									SNew(SBox)
									.WidthOverride(60.0f)
									.HeightOverride(60.0f)
									[
										SNew(SImage)
										.Image(Brush)
									]
								]
								// 右侧：文字和删除按钮
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text_Lambda([WorldEditor, TextureIndex]()
										{
											UCVoxelMapManager& MapManager = WorldEditor->GetMapManager();
											if (TextureIndex >= 0 && TextureIndex < MapManager.TextureConfig.AryTexturePaths.GetSize())
											{
												UCString TexturePathUC = MapManager.TextureConfig.AryTexturePaths[TextureIndex];
												FString TexturePath = WorldEditor->UCStringToFString(TexturePathUC);
												return FText::FromString(FPaths::GetCleanFilename(TexturePath));
											}
											return FText::GetEmpty();
										})
										.Justification(ETextJustify::Left)
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
										.ColorAndOpacity_Lambda([this, TextureIndex]()
										{
											if (SelectedTextureIndex == TextureIndex)
											{
												// 选中状态：白色文字
												return FSlateColor(FLinearColor::White);
											}
											else
											{
												// 未选中状态：默认颜色
												return FSlateColor::UseForeground();
											}
										})
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 8, 0, 0)
									[
										SNew(SButton)
										.Text(LOCTEXT("DeleteTexture", "删除"))
										.HAlign(HAlign_Left)
										.OnClicked_Lambda([this, TextureIndex]()
										{
											RemoveTexture(TextureIndex);
											// RemoveTexture 内部已经处理了刷新
											// 返回 Handled 以阻止事件冒泡到父按钮
											return FReply::Handled();
										})
									]
								]
							]
						]
					];
			}
		}
	}

	return SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		+ SScrollBox::Slot()
		.Padding(5.0f)
		[
			VerticalBox.ToSharedRef()
		];
}

void FVoxelEditorEditorModeToolkit::SelectTexture(int32 Index) const
{
	// 更新选中的纹理索引
	SelectedTextureIndex = Index;
	
	UE_LOG(LogTemp, Log, TEXT("Selected texture index: %d"), Index);
	
	// 由于GetEditToolWidget会缓存Widget，需要重置缓存才能触发重新构建
	// 但为了避免在回调中立即销毁Widget导致崩溃，我们使用FTSTicker延迟执行
	if (GetActiveToolIndex() == 1) // Edit Tool
	{
		// 使用FTSTicker在下一帧执行刷新，避免在回调中立即操作Widget
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float DeltaTime)
		{
			// 重置缓存，强制下次调用时重新创建
			EditToolWidget.Reset();
			MainContentWidget.Reset();
			
			// 强制刷新 WidgetSwitcher
			if (ToolContentSwitcher.IsValid())
			{
				ToolContentSwitcher->Invalidate(EInvalidateWidget::LayoutAndVolatility);
			}
			
			// 强制刷新整个工具包内容
			if (ToolkitWidget.IsValid())
			{
				ToolkitWidget->Invalidate(EInvalidateWidget::LayoutAndVolatility);
			}
			
			// 只执行一次
			return false;
		}));
	}
}

#undef LOCTEXT_NAMESPACE
