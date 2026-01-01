// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelEditorEditorMode.h"
#include "VoxelEditorEditorModeToolkit.h"
#include "InteractiveToolManager.h"
#include "VoxelEditorEditorModeCommands.h"
#include "Modules/ModuleManager.h"
#include "VoxelWorldEditor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#include "EditorModeTools.h"
#include "EditorModes.h"
#include "LevelEditor.h"
#include "EditorModeManager.h"
#endif


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/VoxelEditorMapsTool.h"
#include "Tools/VoxelEditorEditTool.h"

// step 2: register a ToolBuilder in FVoxelEditorEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "VoxelEditorEditorMode"

const FEditorModeID UVoxelEditorEditorMode::EM_VoxelEditorEditorModeId = TEXT("EM_VoxelEditorEditorMode");

FString UVoxelEditorEditorMode::MapsToolName = TEXT("VoxelEditor_MapsTool");
FString UVoxelEditorEditorMode::EditToolName = TEXT("VoxelEditor_EditTool");
bool UVoxelEditorEditorMode::bMapLoaded = false;
UVoxelEditorEditorMode* UVoxelEditorEditorMode::ActiveInstance = nullptr;


UVoxelEditorEditorMode::UVoxelEditorEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UVoxelEditorEditorMode::EM_VoxelEditorEditorModeId,
		LOCTEXT("ModeName", "VoxelEditor"),
		FSlateIcon(),
		true);
}


UVoxelEditorEditorMode::~UVoxelEditorEditorMode()
{
}


void UVoxelEditorEditorMode::ActorSelectionChangeNotify()
{
}

void UVoxelEditorEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FVoxelEditorEditorModeCommands& SampleToolCommands = FVoxelEditorEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.MapsTool, MapsToolName, NewObject<UVoxelEditorMapsToolBuilder>(this));
	RegisterTool(SampleToolCommands.EditTool, EditToolName, NewObject<UVoxelEditorEditToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, MapsToolName);
	
	// Initially, no map is loaded, so Edit tool should be disabled
	bMapLoaded = false;

	// 保存当前实例
	ActiveInstance = this;
}

void UVoxelEditorEditorMode::Exit()
{
#if WITH_EDITOR
	// 退出编辑器模式时，不清除 VoxelWorldEditor 实例（保留以便下次使用）
	// 清除当前实例引用
	if (ActiveInstance == this)
	{
		ActiveInstance = nullptr;
	}
#endif

	UEdMode::Exit();
}

void UVoxelEditorEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FVoxelEditorEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UVoxelEditorEditorMode::GetModeCommands() const
{
	return FVoxelEditorEditorModeCommands::Get().GetCommands();
}

TSharedPtr<FVoxelEditorEditorModeToolkit> UVoxelEditorEditorMode::GetToolkit() const
{
	return StaticCastSharedPtr<FVoxelEditorEditorModeToolkit>(Toolkit);
}

UVoxelEditorEditorMode* UVoxelEditorEditorMode::GetActiveEditorMode()
{
	return ActiveInstance;
}

#undef LOCTEXT_NAMESPACE
