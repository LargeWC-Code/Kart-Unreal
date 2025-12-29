/********************************************************************
created:	2024/12/XX
filename: 	VoxelWorldEditorMapsWidget.cpp
author:		Auto Generated

purpose:	VoxelWorldEditor Maps Widget - 地图管理窗口实现
*********************************************************************/

#include "VoxelWorldEditorMapsWidget.h"
#include "VoxelWorldEditor.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"

void SVoxelWorldEditorMapsWidget::Construct(const FArguments& InArgs)
{
	VoxelWorldEditor = InArgs._VoxelWorldEditor;

	TSharedPtr<STreeView<TSharedPtr<FVoxelMapTreeNode>>> TreeViewRef;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(10.0f)
		[
			SNew(SVerticalBox)

			// 按钮栏
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(SHorizontalBox)

				// 新建按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 5, 0)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("VoxelEditor", "CreateMap", "新建"))
					.OnClicked(this, &SVoxelWorldEditorMapsWidget::OnCreateMapClicked)
				]

				// 删除按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(NSLOCTEXT("VoxelEditor", "DeleteMap", "删除"))
					.OnClicked(this, &SVoxelWorldEditorMapsWidget::OnDeleteMapClicked)
				]
			]

			// 地图列表树形控件
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(TreeViewRef, STreeView<TSharedPtr<FVoxelMapTreeNode>>)
				.TreeItemsSource(&MapListItems)
				.OnGenerateRow(this, &SVoxelWorldEditorMapsWidget::OnGenerateRow)
				.OnGetChildren(this, &SVoxelWorldEditorMapsWidget::OnGetChildren)
				.OnSelectionChanged(this, &SVoxelWorldEditorMapsWidget::OnMapSelectionChanged)
				.OnMouseButtonDoubleClick(this, &SVoxelWorldEditorMapsWidget::OnMapDoubleClicked)
			]
		]
	];

	// 保存 TreeView 引用
	MapTreeView = TreeViewRef;

	// 刷新地图列表（会在 RefreshMapList 中处理默认选中 Root）
	RefreshMapList();
}

TSharedRef<ITableRow> SVoxelWorldEditorMapsWidget::OnGenerateRow(TSharedPtr<FVoxelMapTreeNode> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FVoxelMapTreeNode>>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item.IsValid() ? Item->DisplayName : FString()))
		];
}

void SVoxelWorldEditorMapsWidget::OnGetChildren(TSharedPtr<FVoxelMapTreeNode> Item, TArray<TSharedPtr<FVoxelMapTreeNode>>& OutChildren)
{
	if (!Item.IsValid() || !Item->NodeData)
	{
		return;
	}

	// 如果是 Root 节点，返回所有直接子节点
	if (Item->bIsRoot)
	{
		OutChildren = Item->Children;
		return;
	}

	// 如果是文件夹节点，返回其子节点
	if (Item->NodeData->Type == UCVoxelMapNodeData_Folder)
	{
		OutChildren = Item->Children;
	}
	else
	{
		OutChildren.Empty();
	}
}

void SVoxelWorldEditorMapsWidget::BuildTreeNodeRecursive(UCVoxelMapNodeData* NodeData, TSharedPtr<FVoxelMapTreeNode> TreeNode)
{
	if (!NodeData)
	{
		return;
	}

	// 构建当前节点的子节点
	ucINT ChildCount = NodeData->_AryNodes.GetSize();
	for (ucINT i = 0; i < ChildCount; i++)
	{
		UCVoxelMapNodeData& ChildNodeData = (UCVoxelMapNodeData&)NodeData->_AryNodes[i];
		if (ChildNodeData.Name.GetLength() > 0)
		{
			const wchar_t* NameBuffer = (const wchar_t*)ChildNodeData.Name;
			FString ChildName = FString(ChildNodeData.Name.GetLength(), NameBuffer);
			
			TSharedPtr<FVoxelMapTreeNode> ChildNode = MakeShareable(new FVoxelMapTreeNode(ChildName, &ChildNodeData));
			TreeNode->Children.Add(ChildNode);
			
			// 递归构建子节点的子节点
			BuildTreeNodeRecursive(&ChildNodeData, ChildNode);
		}
	}
}

TSharedPtr<FVoxelMapTreeNode> SVoxelWorldEditorMapsWidget::FindTreeNodeByNodeData(TSharedPtr<FVoxelMapTreeNode> TreeNode, UCVoxelMapNodeData* NodeData)
{
	if (!TreeNode.IsValid() || !NodeData)
	{
		return nullptr;
	}

	if (TreeNode->NodeData == NodeData)
	{
		return TreeNode;
	}

	// 递归查找子节点
	for (auto& Child : TreeNode->Children)
	{
		TSharedPtr<FVoxelMapTreeNode> Found = FindTreeNodeByNodeData(Child, NodeData);
		if (Found.IsValid())
		{
			return Found;
		}
	}

	return nullptr;
}

void SVoxelWorldEditorMapsWidget::RefreshMapList()
{
	MapListItems.Empty();
	RootNode.Reset();

	if (VoxelWorldEditor.IsValid())
	{
		AVoxelWorldEditor* WorldEditor = VoxelWorldEditor.Get();
		if (WorldEditor)
		{
			// 创建 Root 节点
			UCVoxelMapManager& MapManager = WorldEditor->GetMapManager();
			RootNode = MakeShareable(new FVoxelMapTreeNode(TEXT("Root"), &MapManager.Root, true));
			
			// 递归构建树结构
			BuildTreeNodeRecursive(&MapManager.Root, RootNode);
			
			// Root 节点作为列表的第一项
			MapListItems.Add(RootNode);
		}
	}

	MapTreeView->RequestTreeRefresh();
	
	// 如果 RootNode 有效且之前没有选中，则默认选中 Root
	// 只在初始构建时执行，避免在刷新后立即设置选择导致访问违例
	if (RootNode.IsValid() && MapTreeView.IsValid() && !SelectedNode.IsValid())
	{
		MapTreeView->SetItemExpansion(RootNode, true);
		MapTreeView->SetSelection(RootNode);
		SelectedNode = RootNode;
	}
}

FReply SVoxelWorldEditorMapsWidget::OnCreateMapClicked()
{
	if (!VoxelWorldEditor.IsValid())
	{
		return FReply::Handled();
	}

	// 检查选中的是否是文件夹或 Root
	bool bCanAddChild = SelectedNode->bIsRoot ||
		(SelectedNode->NodeData && SelectedNode->NodeData->Type == UCVoxelMapNodeData_Folder);

	if (!bCanAddChild)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			NSLOCTEXT("VoxelEditor", "NotFolderError", "只能在文件夹中创建新节点！"));
		return FReply::Handled();
	}

	// 创建输入对话框
	TSharedPtr<SEditableTextBox> MapNameTextBox;
	TSharedPtr<SCheckBox> TypeCheckBox;
	TSharedPtr<TArray<TSharedPtr<int32>>> WidthOptions;
	TSharedPtr<TArray<TSharedPtr<int32>>> HeightOptions;
	TSharedPtr<int32> SelectedWidth;
	TSharedPtr<int32> SelectedHeight;
	TSharedPtr<SComboBox<TSharedPtr<int32>>> WidthComboBox;
	TSharedPtr<SComboBox<TSharedPtr<int32>>> HeightComboBox;
	
	// 初始化下拉框选项（1、2、4、8、16）
	WidthOptions = MakeShareable(new TArray<TSharedPtr<int32>>);
	HeightOptions = MakeShareable(new TArray<TSharedPtr<int32>>);
	for (int32 Size : {1, 2, 4, 8, 16})
	{
		WidthOptions->Add(MakeShareable(new int32(Size)));
		HeightOptions->Add(MakeShareable(new int32(Size)));
	}
	SelectedWidth = WidthOptions->Last(); // 默认16
	SelectedHeight = HeightOptions->Last(); // 默认16
	
	// 先创建窗口指针，然后可以在 Lambda 中捕获
	TSharedPtr<SWindow> WindowPtr;
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(NSLOCTEXT("VoxelEditor", "CreateMapDialogTitle", "新建"))
		.ClientSize(FVector2D(400, 320))
		.SizingRule(ESizingRule::UserSized)
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			SNew(SVerticalBox)
			
			// 提示文本
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10, 10, 10, 5)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("VoxelEditor", "MapNameLabel", "名称:"))
			]
			
			// 输入框
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10, 0, 10, 5)
			[
				SAssignNew(MapNameTextBox, SEditableTextBox)
				.HintText(NSLOCTEXT("VoxelEditor", "MapNameHint", "请输入名称"))
			]
			
			// 类型复选框
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10, 5, 10, 5)
			[
				SAssignNew(TypeCheckBox, SCheckBox)
				.IsChecked(false)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("VoxelEditor", "IsFolderLabel", "创建为文件夹"))
				]
			]
			
			// 地图尺寸（宽度）- 只在创建地图时显示
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10, 5, 10, 5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 10, 0)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("VoxelEditor", "MapWidthLabel", "宽度:"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(WidthComboBox, SComboBox<TSharedPtr<int32>>)
					.OptionsSource(WidthOptions.Get())
					.OnGenerateWidget_Lambda([](TSharedPtr<int32> InOption)
					{
						return SNew(STextBlock).Text(FText::AsNumber(*InOption));
					})
					.OnSelectionChanged_Lambda([&SelectedWidth](TSharedPtr<int32> NewSelection, ESelectInfo::Type)
					{
						if (NewSelection.IsValid())
						{
							SelectedWidth = NewSelection;
						}
					})
					[
						SNew(STextBlock)
						.Text_Lambda([&SelectedWidth]()
						{
							return FText::AsNumber(SelectedWidth.IsValid() ? *SelectedWidth : 16);
						})
					]
				]
			]
			
			// 地图尺寸（高度）- 只在创建地图时显示
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10, 5, 10, 10)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 10, 0)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("VoxelEditor", "MapHeightLabel", "高度:"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(HeightComboBox, SComboBox<TSharedPtr<int32>>)
					.OptionsSource(HeightOptions.Get())
					.OnGenerateWidget_Lambda([](TSharedPtr<int32> InOption)
					{
						return SNew(STextBlock).Text(FText::AsNumber(*InOption));
					})
					.OnSelectionChanged_Lambda([&SelectedHeight](TSharedPtr<int32> NewSelection, ESelectInfo::Type)
					{
						if (NewSelection.IsValid())
						{
							SelectedHeight = NewSelection;
						}
					})
					[
						SNew(STextBlock)
						.Text_Lambda([&SelectedHeight]()
						{
							return FText::AsNumber(SelectedHeight.IsValid() ? *SelectedHeight : 16);
						})
					]
				]
			]
			
			// 按钮栏
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10, 0, 10, 10)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				
				// 确定按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 5, 0)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("VoxelEditor", "OK", "确定"))
					.OnClicked_Lambda([&WindowPtr, MapNameTextBox, TypeCheckBox, &SelectedWidth, &SelectedHeight, WidthComboBox, HeightComboBox, this]()
					{
						FString MapName = MapNameTextBox->GetText().ToString().TrimStartAndEnd();
						
						if (MapName.IsEmpty())
						{
							FMessageDialog::Open(EAppMsgType::Ok, 
								NSLOCTEXT("VoxelEditor", "EmptyMapNameError", "名称不能为空！"));
							return FReply::Handled();
						}
						
						// 检查是否选中了节点
						if (!SelectedNode.IsValid() || !SelectedNode->NodeData)
						{
							FMessageDialog::Open(EAppMsgType::Ok, 
								NSLOCTEXT("VoxelEditor", "NoNodeSelectedError", "请先选择一个节点！"));
							return FReply::Handled();
						}
												
						// 获取类型
						bool bIsFolder = TypeCheckBox->IsChecked();
						ucINT NodeType = bIsFolder ? UCVoxelMapNodeData_Folder : UCVoxelMapNodeData_Map;
						
						// 获取地图尺寸（只在创建地图时使用）
						int32 MapWidth = bIsFolder ? 0 : (SelectedWidth.IsValid() ? *SelectedWidth : 16);
						int32 MapHeight = bIsFolder ? 0 : (SelectedHeight.IsValid() ? *SelectedHeight : 16);
						
						// 检查名称是否已存在
						if (VoxelWorldEditor.IsValid())
						{
							AVoxelWorldEditor* WorldEditor = VoxelWorldEditor.Get();
							if (WorldEditor)
							{
								UCVoxelMapNodeData* ParentNode = SelectedNode->NodeData;
								
								// 检查同级节点名称是否重复
								ucINT NodeCount = ParentNode->_AryNodes.GetSize();
								for (ucINT i = 0; i < NodeCount; i++)
								{
									UCVoxelMapNodeData& NodeData = (UCVoxelMapNodeData&)ParentNode->_AryNodes[i];
									if (NodeData.Name.GetLength() > 0)
									{
										const wchar_t* NameBuffer = (const wchar_t*)NodeData.Name;
										FString ExistingName = FString(NodeData.Name.GetLength(), NameBuffer);
										if (ExistingName == MapName)
										{
											FMessageDialog::Open(EAppMsgType::Ok, 
												NSLOCTEXT("VoxelEditor", "DuplicateMapNameError", "名称已存在！"));
											return FReply::Handled();
										}
									}
								}
								
								// 创建新节点数据
								UCVoxelMapNodeData NewNodeData;
								NewNodeData.Type = NodeType;
								NewNodeData.Name = UCString(*MapName);
								
								// 添加到父节点的数组（使用 union 中的 AryNodes）
								ParentNode->_AryNodes.Add((ucCONST _UCEArray::TValue&)NewNodeData);
								
								// 保存到文件
								FString MapsDir = FPaths::ProjectContentDir() / TEXT("VoxelWorld/World");
								FString MapFile = MapsDir / TEXT("AllMaps.wjson");
								UCString UCMapFile = UCString(*MapFile);
								WorldEditor->GetMapManager().SaveToFile(UCMapFile);

								// 刷新列表（不要在这里设置选择，避免访问违例）
								RefreshMapList();
							}
						}
						
						// 使用 FSlateApplication 安全地关闭窗口
						if (WindowPtr.IsValid())
						{
							FSlateApplication::Get().RequestDestroyWindow(WindowPtr.ToSharedRef());
						}
						return FReply::Handled();
					})
				]
				
				// 取消按钮
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(NSLOCTEXT("VoxelEditor", "Cancel", "取消"))
					.OnClicked_Lambda([&WindowPtr]()
					{
						// 使用 FSlateApplication 安全地关闭窗口
						if (WindowPtr.IsValid())
						{
							FSlateApplication::Get().RequestDestroyWindow(WindowPtr.ToSharedRef());
						}
						return FReply::Handled();
					})
				]
			]
		];
	
	// 将 Window 赋值给 WindowPtr，以便 Lambda 中可以使用
	WindowPtr = Window;
	
	// 显示对话框
	FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().FindBestParentWindowForDialogs(nullptr));
	
	// 设置焦点到输入框
	if (MapNameTextBox.IsValid())
	{
		FSlateApplication::Get().SetKeyboardFocus(MapNameTextBox.ToSharedRef());
	}
	
	return FReply::Handled();
}

FReply SVoxelWorldEditorMapsWidget::OnDeleteMapClicked()
{
	if (!VoxelWorldEditor.IsValid() || !SelectedNode.IsValid() || !SelectedNode->NodeData)
	{
		FMessageDialog::Open(EAppMsgType::Ok, 
			NSLOCTEXT("VoxelEditor", "NoMapSelectedError", "请先选择一个节点！"));
		return FReply::Handled();
	}

	// 不能删除 Root
	if (SelectedNode->bIsRoot)
	{
		FMessageDialog::Open(EAppMsgType::Ok, 
			NSLOCTEXT("VoxelEditor", "CannotDeleteRootError", "不能删除 Root 节点！"));
		return FReply::Handled();
	}

	// 获取要删除的节点名称
	FString NodeNameToDelete = SelectedNode->DisplayName;
	
	// 确认删除
	EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo,
		FText::Format(NSLOCTEXT("VoxelEditor", "ConfirmDeleteMap", "确定要删除节点 \"{0}\" 吗？"), FText::FromString(NodeNameToDelete)));
	
	if (Result != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	AVoxelWorldEditor* WorldEditor = VoxelWorldEditor.Get();
	if (WorldEditor)
	{
		UCVoxelMapManager& MapManager = WorldEditor->GetMapManager();
		
		// 递归查找并删除节点的辅助函数
		struct FNodeDeleter
		{
			static bool FindAndDeleteNode(UCVoxelMapNodeData* Parent, UCVoxelMapNodeData* Target)
			{
				if (!Parent || !Target)
				{
					return false;
				}

				ucINT NodeCount = Parent->_AryNodes.GetSize();
				for (ucINT i = 0; i < NodeCount; i++)
				{
					UCVoxelMapNodeData& NodeData = (UCVoxelMapNodeData&)Parent->_AryNodes[i];
					if (&NodeData == Target)
					{
						// 找到匹配的节点，删除它（使用 union 中的 AryNodes）
						Parent->_AryNodes.RemoveAt(i);
						return true;
					}
					
					// 递归查找子节点
					if (FindAndDeleteNode(&NodeData, Target))
					{
						return true;
					}
				}
				return false;
			}
		};
		
		// 从 Root 开始查找
		bool bFound = FNodeDeleter::FindAndDeleteNode(&MapManager.Root, SelectedNode->NodeData);
		if (bFound)
		{
			bFound = true;
			
			// 保存到文件
			FString MapsDir = FPaths::ProjectContentDir() / TEXT("VoxelWorld/World");
			FString MapFile = MapsDir / TEXT("AllMaps.wjson");
			UCString UCMapFile = UCString(*MapFile);
			MapManager.SaveToFile(UCMapFile);
			
			UE_LOG(LogTemp, Log, TEXT("VoxelWorldEditor: Deleted node: %s"), *NodeNameToDelete);
			
			// 刷新列表（不要在这里设置选择，避免访问违例）
			RefreshMapList();
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, 
				NSLOCTEXT("VoxelEditor", "MapNotFoundError", "未找到要删除的节点！"));
		}
	}
	
	return FReply::Handled();
}

void SVoxelWorldEditorMapsWidget::OnMapSelectionChanged(TSharedPtr<FVoxelMapTreeNode> SelectedItem, ESelectInfo::Type SelectInfo)
{
	SelectedNode = SelectedItem;
	if (SelectedItem.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("VoxelWorldEditor: Node selected: %s"), *SelectedItem->DisplayName);
	}
}

void SVoxelWorldEditorMapsWidget::OnMapDoubleClicked(TSharedPtr<FVoxelMapTreeNode> Item)
{
	if (!Item.IsValid() || !Item->NodeData || !VoxelWorldEditor.IsValid())
	{
		return;
	}
	
	// 只处理地图节点（不是文件夹）
	if (Item->bIsRoot || Item->NodeData->Type == UCVoxelMapNodeData_Folder)
	{
		return;
	}
	
	AVoxelWorldEditor* WorldEditor = VoxelWorldEditor.Get();
	if (!WorldEditor)
	{
		return;
	}
	
	// 获取节点路径
	FString NodePath = GetNodePath(Item);
	FString MapsDir = FPaths::ProjectContentDir() / TEXT("VoxelWorld/World") / NodePath;
	
	// 获取地图名称
	const wchar_t* NameBuffer = (const wchar_t*)Item->NodeData->Name;
	FString MapName = FString(Item->NodeData->Name.GetLength(), NameBuffer);
	FString MapFile = MapsDir / (MapName + TEXT(".mjson"));
	
	// 检查文件是否存在
	if (FPaths::FileExists(MapFile))
	{
		// 文件存在，加载地图
		UCString UCMapFile = UCString(*MapFile);
		if (WorldEditor->GetMapManager().LoadMap(UCMapFile))
		{
			UE_LOG(LogTemp, Log, TEXT("VoxelWorldEditor: Loaded map: %s"), *MapFile);
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok,
				NSLOCTEXT("VoxelEditor", "LoadMapError", "加载地图失败！"));
		}
	}
	else
	{
		// 文件不存在，创建目录并保存新地图
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*MapsDir);
		
		UCString UCMapFile = UCString(*MapFile);
		WorldEditor->GetMapManager().SaveMap(UCMapFile);
		
		UE_LOG(LogTemp, Log, TEXT("VoxelWorldEditor: Created new map file: %s"), *MapFile);
	}
}

FString SVoxelWorldEditorMapsWidget::GetNodePath(TSharedPtr<FVoxelMapTreeNode> Node)
{
	if (!Node.IsValid() || Node->bIsRoot)
	{
		return FString();
	}
	
	// 递归获取从Root到当前节点的路径
	TArray<FString> PathParts;
	TSharedPtr<FVoxelMapTreeNode> CurrentNode = Node;
	
	// 从当前节点向上查找直到Root
	while (CurrentNode.IsValid() && !CurrentNode->bIsRoot)
	{
		PathParts.Insert(CurrentNode->DisplayName, 0);
		
		// 查找父节点（在RootNode中查找）
		if (RootNode.IsValid())
		{
			// 在RootNode的子树中查找包含CurrentNode的父节点
			TSharedPtr<FVoxelMapTreeNode> ParentNode = nullptr;
			TFunction<void(TSharedPtr<FVoxelMapTreeNode>)> FindParent = [&](TSharedPtr<FVoxelMapTreeNode> TreeNode)
			{
				if (!TreeNode.IsValid())
				{
					return;
				}
				
				for (auto& Child : TreeNode->Children)
				{
					if (Child == CurrentNode)
					{
						ParentNode = TreeNode;
						return;
					}
					FindParent(Child);
					if (ParentNode.IsValid())
					{
						return;
					}
				}
			};
			
			FindParent(RootNode);
			CurrentNode = ParentNode;
		}
		else
		{
			break;
		}
	}
	
	// 组合路径（跳过Root）
	FString Result;
	for (int32 i = 0; i < PathParts.Num() - 1; i++) // 最后一个元素是当前节点名称，不包含在路径中
	{
		if (i > 0)
		{
			Result += TEXT("/");
		}
		Result += PathParts[i];
	}
	
	return Result;
}

