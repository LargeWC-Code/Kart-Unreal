// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelEditorEditorModeToolkit.h"
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

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "VoxelEditorEditorModeToolkit"

FVoxelEditorEditorModeToolkit::FVoxelEditorEditorModeToolkit()
{
	// 初始化 32×32 = 1024 个按钮状态，默认为非激活
	EditToolButtonStates.SetNum(32 * 32, false);
}

void FVoxelEditorEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);

	// 保存 OwningMode，供 GetInlineContent() 使用
	OwningMode = InOwningMode;

	// 创建 VoxelWorldEditor 实例
	AVoxelWorldEditor* NewVoxelWorldEditorInstance = nullptr;
	if (UVoxelEditorEditorMode* EditorMode = Cast<UVoxelEditorEditorMode>(InOwningMode.Get()))
	{
		if (UWorld* World = EditorMode->GetWorld())
		{
			// 生成唯一的 GUID 并组合到名称中
			FGuid UniqueID = FGuid::NewGuid();
			FString UniqueName = FString::Printf(TEXT("VoxelWorldEditor_%s"), *UniqueID.ToString(EGuidFormats::Short));
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(*UniqueName);
			NewVoxelWorldEditorInstance = World->SpawnActor<AVoxelWorldEditor>(SpawnParams);
			if (NewVoxelWorldEditorInstance)
			{
				UE_LOG(LogTemp, Log, TEXT("VoxelEditorToolkit: Created VoxelWorldEditor instance in Init with name: %s"), *UniqueName);
				// 将实例保存到 EditorMode 中，以便 Exit 时可以销毁
				EditorMode->SetVoxelWorldEditorInstance(NewVoxelWorldEditorInstance);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("VoxelEditorToolkit: Failed to create VoxelWorldEditor instance"));
			}
		}
	}

	// 保存 VoxelWorldEditor 实例，供 GetInlineContent() 使用
	VoxelWorldEditorInstance = NewVoxelWorldEditorInstance;
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
	return SNew(SWidgetSwitcher)
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
	
	// 确保状态数组已初始化（32×32 = 1024个按钮）
	const int32 GridSize = 16;
	if (EditToolButtonStates.Num() != GridSize * GridSize)
	{
		EditToolButtonStates.SetNum(GridSize * GridSize, false);
	}
	
	// 创建一个带滚动条的区域，包含 32×32 个按钮
	// 使用嵌套的滚动框来支持水平和垂直滚动
	// 设置固定的小尺寸可视区域，方便使用滚动条
	const float VisibleWidth = 800.0f;   // 可视区域宽度
	const float VisibleHeight = 600.0f;  // 可视区域高度
	
	EditToolWidget = SNew(SBorder)
		.Padding(10.0f)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
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
		];
	
	// 创建 32×32 个按钮（总共 1024 个）
	// 坐标范围：从 -16 到 15（中心是 0,0）
	const float ButtonSize = 50.0f; // 增大按钮以显示坐标文本
	
	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; ++Col)
		{
			int32 ButtonIndex = Row * GridSize + Col;
			// 计算坐标：X和Y都从-16到15（中心是0,0）
			// 对于32×32网格，中心位置是(15.5, 15.5)，我们使用(15,15)作为(0,0)的参考点
			int32 X = Col - (GridSize / 2);  // -16 到 15
			int32 Y = (GridSize / 2 - 1) - Row; // Y轴反转，使(0,0)在中心附近：15 到 -16
			
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
						.OnClicked_Lambda([this, ButtonIndex, GridSize, X, Y]()
						{
							if (ButtonIndex >= 0 && ButtonIndex < EditToolButtonStates.Num())
							{
								EditToolButtonStates[ButtonIndex] = !EditToolButtonStates[ButtonIndex];
								UE_LOG(LogTemp, Log, TEXT("Edit Tool Button [%d,%d] clicked, Active: %s"), 
									X, Y,
									EditToolButtonStates[ButtonIndex] ? TEXT("true") : TEXT("false"));
								
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
	
	return EditToolWidget;
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

#undef LOCTEXT_NAMESPACE
