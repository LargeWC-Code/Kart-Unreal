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
}

void UVoxelEditorEditorMode::Exit()
{
#if WITH_EDITOR
	// 退出编辑器模式时，提示保存并销毁 VoxelWorldEditor
	if (VoxelWorldEditorInstance && IsValid(VoxelWorldEditorInstance))
	{
		// 检查是否有未保存的更改（这里可以根据实际需求实现更复杂的检查逻辑）
		bool bHasUnsavedChanges = false; // TODO: 实现未保存更改检查

		if (bHasUnsavedChanges)
		{
			// 显示保存提示对话框
			FText Title = LOCTEXT("SaveChangesTitle", "Save Changes?");
			FText Message = LOCTEXT("SaveChangesMessage", "You have unsaved changes in VoxelWorldEditor. Do you want to save before exiting?");
			
			EAppReturnType::Type Result = FMessageDialog::Open(
				EAppMsgType::YesNoCancel,
				Message,
				Title
			);

			if (Result == EAppReturnType::Yes)
			{
				// 保存地图（这里可以调用 VoxelWorldEditorInstance->SaveMap()）
				// TODO: 实现保存逻辑
				UE_LOG(LogTemp, Log, TEXT("VoxelEditor: Saving VoxelWorldEditor before exit"));
			}
			else if (Result == EAppReturnType::Cancel)
			{
				// 取消退出（这里需要阻止退出，但 UEdMode::Exit() 没有返回值，所以只能记录日志）
				UE_LOG(LogTemp, Log, TEXT("VoxelEditor: Exit cancelled by user"));
			}
		}

		// 销毁 VoxelWorldEditor 实例
		if (VoxelWorldEditorInstance)
		{
			UWorld* World = VoxelWorldEditorInstance->GetWorld();
			if (World)
			{
				World->DestroyActor(VoxelWorldEditorInstance);
				UE_LOG(LogTemp, Log, TEXT("VoxelEditor: Destroyed VoxelWorldEditor instance"));
			}
			VoxelWorldEditorInstance = nullptr;
		}
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

#undef LOCTEXT_NAMESPACE
