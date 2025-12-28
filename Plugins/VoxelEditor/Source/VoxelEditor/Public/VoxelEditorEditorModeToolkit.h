// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Toolkits/BaseToolkit.h"
#include "VoxelEditorEditorMode.h"

/**
 * This FModeToolkit just creates a basic UI panel that allows various InteractiveTools to
 * be initialized, and a DetailsView used to show properties of the active Tool.
 */
class FVoxelEditorEditorModeToolkit : public FModeToolkit
{
public:
	FVoxelEditorEditorModeToolkit();

	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;
	virtual TSharedPtr<SWidget> GetInlineContent() const override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;

private:
	/** 获取 Maps 工具的 Widget */
	TSharedPtr<SWidget> GetMapsToolWidget() const;

	/** 获取 Edit 工具的 Widget */
	TSharedPtr<SWidget> GetEditToolWidget() const;

	/** 创建包含 WidgetSwitcher 的主 Widget */
	TSharedPtr<SWidget> CreateToolContentWidget() const;

	/** 获取当前激活的工具索引 */
	int32 GetActiveToolIndex() const;

private:
	/** VoxelWorldEditor 实例 */
	TWeakObjectPtr<class AVoxelWorldEditor> VoxelWorldEditorInstance;

	/** Owning Editor Mode */
	TWeakObjectPtr<UEdMode> OwningMode;

	/** 主内容 Widget（包含 WidgetSwitcher） */
	mutable TSharedPtr<SWidget> MainContentWidget;

	/** Edit 工具按钮的激活状态数组（32×32 = 1024个按钮） */
	mutable TArray<bool> EditToolButtonStates;

	/** Edit 工具 Widget（缓存以避免重复创建） */
	mutable TSharedPtr<SWidget> EditToolWidget;

	/** Edit 工具按钮网格面板（用于触发刷新） */
	mutable TSharedPtr<class SUniformGridPanel> EditToolGridPanel;
};
