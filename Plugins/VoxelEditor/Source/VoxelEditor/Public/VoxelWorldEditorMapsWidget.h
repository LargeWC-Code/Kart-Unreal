/********************************************************************
created:	2024/12/XX
filename: 	VoxelWorldEditorMapsWidget.h
author:		Auto Generated

purpose:	VoxelWorldEditor Maps Widget - 地图管理窗口
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "VoxelMap.h"

struct UCVoxelMapNodeData;
class SWindow;
// 树节点数据结构
struct FVoxelMapTreeNode
{
	FString DisplayName;
	UCVoxelMapNodeData* NodeData;  // 指向实际节点的指针
	bool bIsRoot;  // 是否为 Root 节点
	TArray<TSharedPtr<FVoxelMapTreeNode>> Children;  // 子节点列表

	FVoxelMapTreeNode() : NodeData(nullptr), bIsRoot(false) {}
	FVoxelMapTreeNode(const FString& InName, UCVoxelMapNodeData* InNodeData, bool InIsRoot = false)
		: DisplayName(InName), NodeData(InNodeData), bIsRoot(InIsRoot) {}
};

class SVoxelWorldEditorMapsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVoxelWorldEditorMapsWidget)
	{}
	SLATE_ARGUMENT(class AVoxelWorldEditor*, VoxelWorldEditor)
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** 生成树形控件行 */
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FVoxelMapTreeNode> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** 获取子节点（用于树形结构） */
	void OnGetChildren(TSharedPtr<FVoxelMapTreeNode> Item, TArray<TSharedPtr<FVoxelMapTreeNode>>& OutChildren);

private:
	/** 刷新地图列表 */
	void RefreshMapList();

	/** 递归构建树节点 */
	void BuildTreeNodeRecursive(UCVoxelMapNodeData* NodeData, TSharedPtr<FVoxelMapTreeNode> TreeNode);

	/** 根据节点指针查找树节点 */
	TSharedPtr<FVoxelMapTreeNode> FindTreeNodeByNodeData(TSharedPtr<FVoxelMapTreeNode> TreeNode, UCVoxelMapNodeData* NodeData);

	/** 新建地图 */
	FReply OnCreateMapClicked();

	/** 删除地图 */
	FReply OnDeleteMapClicked();

	/** 保存按钮 */
	FReply OnSaveMapClicked();

	/** 关闭按钮 */
	FReply OnCloseClicked();

	/** 树形控件选择改变 */
	void OnMapSelectionChanged(TSharedPtr<FVoxelMapTreeNode> SelectedItem, ESelectInfo::Type SelectInfo);

	/** 树形控件双击事件 */
	void OnMapDoubleClicked(TSharedPtr<FVoxelMapTreeNode> Item);

	/** 递归获取节点路径（从Root到当前节点） */
	FString GetNodePath(TSharedPtr<FVoxelMapTreeNode> Node);

private:
	/** VoxelWorldEditor 引用 */
	TWeakObjectPtr<class AVoxelWorldEditor> VoxelWorldEditor;

	/** 父窗口引用 */
	TWeakPtr<SWindow> ParentWindow;

	/** 地图列表树形控件 */
	TSharedPtr<class STreeView<TSharedPtr<FVoxelMapTreeNode>>> MapTreeView;

	/** 地图列表数据源 */
	TArray<TSharedPtr<FVoxelMapTreeNode>> MapListItems;

	/** Root 节点 */
	TSharedPtr<FVoxelMapTreeNode> RootNode;

	/** 当前选中的节点 */
	TSharedPtr<FVoxelMapTreeNode> SelectedNode;

	/** 已展开的节点集合（使用 NodeData 指针作为标识） */
	TSet<UCVoxelMapNodeData*> ExpandedNodeDataSet;

	/** 节点展开/折叠状态改变的回调 */
	void OnExpansionChanged(TSharedPtr<FVoxelMapTreeNode> Item, bool bExpanded);

	/** 获取当前加载的地图文件路径 */
	FString GetCurrentMapFilePath() const;
};


