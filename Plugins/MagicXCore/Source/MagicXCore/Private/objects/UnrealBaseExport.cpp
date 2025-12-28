#include "objects/UnrealBaseExport.h"
#include "objects/UnrealUIExport.h"
#include "objects/UnrealEnvironmentExport.h"
#include <functional>
#include <unordered_map>

// 包含所有需要的UI组件头文件
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanel.h"
#include "Components/GridPanel.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/Border.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

SCRIPT_TOKEN_DEFINE(UCNewUObject, _UCNewUObject);
SCRIPT_TOKEN_DEFINE(UCCreateWidget, _UCCreateWidget);

SCRIPT_TOKEN_DEFINE(UCLoadClass, _UCLoadClass);
SCRIPT_TOKEN_DEFINE(UCTSubclassOf, _UCTSubclassOf);

SCRIPT_TOKEN_DEFINE(UCTObjectPtr, _UCTObjectPtr);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCTSubclassOf, _UCTSubclassOf);
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCEVariableInfoCC&);
	SCRIPT_CONSTRUCT_2(ucCONST UCE_UCEVariableInfoCC&, UCE_UCLoadClass&);
	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCTSubclassOf);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCLoadClass, _UCLoadClass);
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCEVariableInfoCC&);
	SCRIPT_NCONSTRUCT_2(_20, ucCONST UCE_UCEVariableInfoCC&, UCE_FString&);
	SCRIPT_NCONSTRUCT_2(_21, ucCONST UCE_UCEVariableInfoCC&, ucCONST UCE_TCHAR*);

	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCLoadClass);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCNewUObject, _UCNewUObject);
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCEVariableInfoCC&);

	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCNewUObject);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCTObjectPtr, _UCTObjectPtr);
	SCRIPT_CONSTRUCT_1(ucCONST UCE_UCEVariableInfoCC&);
	SCRIPT_NCONSTRUCT_2(_21, ucCONST UCE_UCEVariableInfoCC&, UCE_UCNewUObject&);
	SCRIPT_NCONSTRUCT_2(_22, ucCONST UCE_UCEVariableInfoCC&, UCE_UCLoadClass&);
	SCRIPT_NCONSTRUCT_2(_23, ucCONST UCE_UCEVariableInfoCC&, UCE_UCCreateWidget&);
	SCRIPT_NCONSTRUCT_2(_24, ucCONST UCE_UCEVariableInfoCC&, UCE_UObject*);

	SCRIPT_RMOPERATOR_0(UCE_UCETemplateArg0*, s, ->);

	SCRIPT_PROPERTY(UCE_UCETemplateArg0*, Object);
	SCRIPT_RMFUNCTION_0(UCE_UCETemplateArg0*, Get);

	SCRIPT_MOPERATOR_1(eq1, =, UCE_UCNewUObject&);
	SCRIPT_MOPERATOR_1(eq2, =, UCE_UCLoadClass&);
	SCRIPT_MOPERATOR_1(eq3, =, UCE_UCCreateWidget&);
	SCRIPT_MOPERATOR_1(eq4, =, UCE_UObject*);


	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCTObjectPtr);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCCreateWidget, _UCCreateWidget);
	SCRIPT_CONSTRUCT_3(ucCONST UCE_UCEVariableInfoCC&, UCE_UObject*, UCE_UClass*);

	SCRIPT_DECONSTRUCT();
SCRIPT_IMPLEMENT_END(UCE_UCCreateWidget);


// 静态创建函数
namespace UCObjectCreators
{
	// UI控件创建函数
	UObject* CreateButton() { return NewObject<UButton>(); }
	UObject* CreateTextBlock() { return NewObject<UTextBlock>(); }
	UObject* CreateEditableTextBox() { return NewObject<UEditableTextBox>(); }
	UObject* CreateMultiLineEditableTextBox() { return NewObject<UMultiLineEditableTextBox>(); }
	UObject* CreateImage() { return NewObject<UImage>(); }
	UObject* CreateProgressBar() { return NewObject<UProgressBar>(); }
	UObject* CreateCheckBox() { return NewObject<UCheckBox>(); }
	UObject* CreateSlider() { return NewObject<USlider>(); }
	UObject* CreateComboBoxString() { return NewObject<UComboBoxString>(); }
	UObject* CreateBorder() { return NewObject<UBorder>(); }
	
	// 面板容器创建函数
	UObject* CreateVerticalBox() { return NewObject<UVerticalBox>(); }
	UObject* CreateHorizontalBox() { return NewObject<UHorizontalBox>(); }
	UObject* CreateCanvasPanel() { return NewObject<UCanvasPanel>(); }
	UObject* CreateGridPanel() { return NewObject<UGridPanel>(); }
	UObject* CreateScrollBox() { return NewObject<UScrollBox>(); }
	
	// 用户控件创建函数
	UObject* CreateUserWidget() { return NewObject<UUserWidget>(); }
	
	// 基础类创建函数
	UObject* CreateClass() { return NewObject<UClass>(); }
	UObject* CreateGameInstance() { return NewObject<UGameInstance>(); }
	UObject* CreatePlayerController() { return NewObject<APlayerController>(); }
	UObject* CreateWorld() { return NewObject<UWorld>(); }
	
	// Slot类创建函数
	UObject* CreateVerticalBoxSlot() { return NewObject<UVerticalBoxSlot>(); }
	UObject* CreateHorizontalBoxSlot() { return NewObject<UHorizontalBoxSlot>(); }
	UObject* CreateCanvasPanelSlot() { return NewObject<UCanvasPanelSlot>(); }
	UObject* CreateGridSlot() { return NewObject<UGridSlot>(); }
}

// 静态LoadClass函数
namespace UCLoadClassFunctions
{
	// UI控件LoadClass函数
	UClass* LoadButton(const FString& Path) { return LoadClass<UButton>(nullptr, *Path); }
	UClass* LoadTextBlock(const FString& Path) { return LoadClass<UTextBlock>(nullptr, *Path); }
	UClass* LoadEditableTextBox(const FString& Path) { return LoadClass<UEditableTextBox>(nullptr, *Path); }
	UClass* LoadMultiLineEditableTextBox(const FString& Path) { return LoadClass<UMultiLineEditableTextBox>(nullptr, *Path); }
	UClass* LoadImage(const FString& Path) { return LoadClass<UImage>(nullptr, *Path); }
	UClass* LoadProgressBar(const FString& Path) { return LoadClass<UProgressBar>(nullptr, *Path); }
	UClass* LoadCheckBox(const FString& Path) { return LoadClass<UCheckBox>(nullptr, *Path); }
	UClass* LoadSlider(const FString& Path) { return LoadClass<USlider>(nullptr, *Path); }
	UClass* LoadComboBoxString(const FString& Path) { return LoadClass<UComboBoxString>(nullptr, *Path); }
	UClass* LoadBorder(const FString& Path) { return LoadClass<UBorder>(nullptr, *Path); }
	
	// 面板容器LoadClass函数
	UClass* LoadVerticalBox(const FString& Path) { return LoadClass<UVerticalBox>(nullptr, *Path); }
	UClass* LoadHorizontalBox(const FString& Path) { return LoadClass<UHorizontalBox>(nullptr, *Path); }
	UClass* LoadCanvasPanel(const FString& Path) { return LoadClass<UCanvasPanel>(nullptr, *Path); }
	UClass* LoadGridPanel(const FString& Path) { return LoadClass<UGridPanel>(nullptr, *Path); }
	UClass* LoadScrollBox(const FString& Path) { return LoadClass<UScrollBox>(nullptr, *Path); }
	
	// 用户控件LoadClass函数
	UClass* LoadUserWidget(const FString& Path) { return LoadClass<UUserWidget>(nullptr, *Path); }
	
	// 基础类LoadClass函数
	UClass* LoadUClass(const FString& Path) { return ::LoadClass<UClass>(nullptr, *Path); }
	UClass* LoadGameInstance(const FString& Path) { return ::LoadClass<UGameInstance>(nullptr, *Path); }
	UClass* LoadPlayerController(const FString& Path) { return ::LoadClass<APlayerController>(nullptr, *Path); }
	UClass* LoadWorld(const FString& Path) { return ::LoadClass<UWorld>(nullptr, *Path); }
	
	// Slot类LoadClass函数
	UClass* LoadVerticalBoxSlot(const FString& Path) { return LoadClass<UVerticalBoxSlot>(nullptr, *Path); }
	UClass* LoadHorizontalBoxSlot(const FString& Path) { return LoadClass<UHorizontalBoxSlot>(nullptr, *Path); }
	UClass* LoadCanvasPanelSlot(const FString& Path) { return LoadClass<UCanvasPanelSlot>(nullptr, *Path); }
	UClass* LoadGridSlot(const FString& Path) { return LoadClass<UGridSlot>(nullptr, *Path); }
}

class UCUnrealFactory
{
public:
	// 定义创建函数指针类型
	using _CREATEFUNCTION = UObject*(*)();
	// 定义LoadClass函数指针类型
	using _LOADCLASSFUNCTION = UClass*(*)(const FString&);
	
	// 映射表：脚本类指针 -> 创建函数指针
	UCSimpleMap<ucCONST UCEClassInfo*, _CREATEFUNCTION>		ObjectCreatorMap;
	// 映射表：脚本类指针 -> LoadClass函数指针
	UCSimpleMap<ucCONST UCEClassInfo*, _LOADCLASSFUNCTION>	LoadClassMap;

	UCUnrealFactory()
	{
		InitializeCreatorMap();
		InitializeLoadClassMap();
	}

private:
	void InitializeCreatorMap()
	{
		// UI 控件类映射
		ObjectCreatorMap[&UCE_UButton::I] = UCObjectCreators::CreateButton;
		ObjectCreatorMap[&UCE_UTextBlock::I] = UCObjectCreators::CreateTextBlock;
		ObjectCreatorMap[&UCE_UEditableTextBox::I] = UCObjectCreators::CreateEditableTextBox;
		ObjectCreatorMap[&UCE_UMultiLineEditableTextBox::I] = UCObjectCreators::CreateMultiLineEditableTextBox;
		ObjectCreatorMap[&UCE_UImage::I] = UCObjectCreators::CreateImage;
		ObjectCreatorMap[&UCE_UProgressBar::I] = UCObjectCreators::CreateProgressBar;
		ObjectCreatorMap[&UCE_UCheckBox::I] = UCObjectCreators::CreateCheckBox;
		ObjectCreatorMap[&UCE_USlider::I] = UCObjectCreators::CreateSlider;
		ObjectCreatorMap[&UCE_UComboBoxString::I] = UCObjectCreators::CreateComboBoxString;
		ObjectCreatorMap[&UCE_UBorder::I] = UCObjectCreators::CreateBorder;
		
		// 面板容器类映射
		ObjectCreatorMap[&UCE_UVerticalBox::I] = UCObjectCreators::CreateVerticalBox;
		ObjectCreatorMap[&UCE_UHorizontalBox::I] = UCObjectCreators::CreateHorizontalBox;
		ObjectCreatorMap[&UCE_UCanvasPanel::I] = UCObjectCreators::CreateCanvasPanel;
		ObjectCreatorMap[&UCE_UGridPanel::I] = UCObjectCreators::CreateGridPanel;
		ObjectCreatorMap[&UCE_UScrollBox::I] = UCObjectCreators::CreateScrollBox;
		
		// 用户控件类映射
		ObjectCreatorMap[&UCE_UUserWidget::I] = UCObjectCreators::CreateUserWidget;
		
		// 基础类映射
		ObjectCreatorMap[&UCE_UClass::I] = UCObjectCreators::CreateClass;
		ObjectCreatorMap[&UCE_UGameInstance::I] = UCObjectCreators::CreateGameInstance;
		ObjectCreatorMap[&UCE_APlayerController::I] = UCObjectCreators::CreatePlayerController;
		ObjectCreatorMap[&UCE_UWorld::I] = UCObjectCreators::CreateWorld;
		
		// Slot类映射
		ObjectCreatorMap[&UCE_UVerticalBoxSlot::I] = UCObjectCreators::CreateVerticalBoxSlot;
		ObjectCreatorMap[&UCE_UHorizontalBoxSlot::I] = UCObjectCreators::CreateHorizontalBoxSlot;
		ObjectCreatorMap[&UCE_UCanvasPanelSlot::I] = UCObjectCreators::CreateCanvasPanelSlot;
		ObjectCreatorMap[&UCE_UGridSlot::I] = UCObjectCreators::CreateGridSlot;
	}

	void InitializeLoadClassMap()
	{
		// UI 控件类LoadClass映射
		LoadClassMap[&UCE_UButton::I] = UCLoadClassFunctions::LoadButton;
		LoadClassMap[&UCE_UTextBlock::I] = UCLoadClassFunctions::LoadTextBlock;
		LoadClassMap[&UCE_UEditableTextBox::I] = UCLoadClassFunctions::LoadEditableTextBox;
		LoadClassMap[&UCE_UMultiLineEditableTextBox::I] = UCLoadClassFunctions::LoadMultiLineEditableTextBox;
		LoadClassMap[&UCE_UImage::I] = UCLoadClassFunctions::LoadImage;
		LoadClassMap[&UCE_UProgressBar::I] = UCLoadClassFunctions::LoadProgressBar;
		LoadClassMap[&UCE_UCheckBox::I] = UCLoadClassFunctions::LoadCheckBox;
		LoadClassMap[&UCE_USlider::I] = UCLoadClassFunctions::LoadSlider;
		LoadClassMap[&UCE_UComboBoxString::I] = UCLoadClassFunctions::LoadComboBoxString;
		LoadClassMap[&UCE_UBorder::I] = UCLoadClassFunctions::LoadBorder;
		
		// 面板容器类LoadClass映射
		LoadClassMap[&UCE_UVerticalBox::I] = UCLoadClassFunctions::LoadVerticalBox;
		LoadClassMap[&UCE_UHorizontalBox::I] = UCLoadClassFunctions::LoadHorizontalBox;
		LoadClassMap[&UCE_UCanvasPanel::I] = UCLoadClassFunctions::LoadCanvasPanel;
		LoadClassMap[&UCE_UGridPanel::I] = UCLoadClassFunctions::LoadGridPanel;
		LoadClassMap[&UCE_UScrollBox::I] = UCLoadClassFunctions::LoadScrollBox;
		
		// 用户控件类LoadClass映射
		LoadClassMap[&UCE_UUserWidget::I] = UCLoadClassFunctions::LoadUserWidget;
		
		// 基础类LoadClass映射
		LoadClassMap[&UCE_UClass::I] = UCLoadClassFunctions::LoadUClass;
		LoadClassMap[&UCE_UGameInstance::I] = UCLoadClassFunctions::LoadGameInstance;
		LoadClassMap[&UCE_APlayerController::I] = UCLoadClassFunctions::LoadPlayerController;
		LoadClassMap[&UCE_UWorld::I] = UCLoadClassFunctions::LoadWorld;
		
		// Slot类LoadClass映射
		LoadClassMap[&UCE_UVerticalBoxSlot::I] = UCLoadClassFunctions::LoadVerticalBoxSlot;
		LoadClassMap[&UCE_UHorizontalBoxSlot::I] = UCLoadClassFunctions::LoadHorizontalBoxSlot;
		LoadClassMap[&UCE_UCanvasPanelSlot::I] = UCLoadClassFunctions::LoadCanvasPanelSlot;
		LoadClassMap[&UCE_UGridSlot::I] = UCLoadClassFunctions::LoadGridSlot;
	}
public:
	// 根据脚本类创建对象
	UObject* CreateObject(ucCONST UCEClassInfo* ClassInfo)
	{
		ucINT Pos = ObjectCreatorMap.FindKey(ClassInfo);
		if (Pos >= 0)
			return ObjectCreatorMap.GetValueAt(Pos)();
		return ucNULL;
	}

	// 根据脚本类和路径加载类对象
	UClass* LoadClassObject(ucCONST UCEClassInfo* ClassInfo, const FString& Path)
	{
		ucINT Pos = LoadClassMap.FindKey(ClassInfo);
		if (Pos >= 0)
		{
			_LOADCLASSFUNCTION LoadFunc = LoadClassMap.GetValueAt(Pos);
			return LoadFunc(Path);
		}
		return ucNULL;
	}
};

UCUnrealFactory* GetFactory()
{
	static UCUnrealFactory Manager;
	return &Manager;
}

_UCTSubclassOf::_UCTSubclassOf(ucCONST UCEVariableInfoCC& Info)
{
	Class = nullptr;
	this->Info = Info;

	if (Info.Object == ucNULL)
	{
		throw;
		return;
	}

	if (Info.Object->_IsClass(&UCE_UClass::I))
	{
		// 使用映射表加载类对象
		UCUnrealFactory* Manager = GetFactory();
		UObject* LoadedObject = Manager->LoadClassObject(Info.Object, TEXT(""));

		if (LoadedObject)
		{
			Class = Cast<UClass>(LoadedObject);
		}
	}
}

_UCTSubclassOf::_UCTSubclassOf(ucCONST UCEVariableInfoCC& Info, _UCLoadClass& InLoadClass)
{
	Class = nullptr;
	this->Info = Info;

	if (Info.Object == ucNULL)
	{
		throw;
		return;
	}

	if (Info.Object->_IsClass(&UCE_UClass::I))
		Class = InLoadClass.Class;
}

_UCLoadClass::_UCLoadClass(ucCONST UCEVariableInfoCC& Info)
{
	Class = nullptr;
	Path = TEXT("");
	this->Info = Info;

	if (Info.Object == ucNULL)
	{
		throw;
		return;
	}
}

_UCLoadClass::_UCLoadClass(ucCONST UCEVariableInfoCC& Info, const FString& InPath)
{
	Class = nullptr;
	Path = InPath;
	this->Info = Info;

	if (Info.Object == ucNULL)
	{
		throw;
		return;
	}

	if (Path.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadClass path is empty"));
		return;
	}

	FString FullPath = Path;
	// 确保路径里带 _C
	if (!Path.EndsWith(TEXT("_C")))
		FullPath += TEXT(".") + FPaths::GetBaseFilename(Path) + TEXT("_C");

	// 使用映射表加载类对象
	UCUnrealFactory* Manager = GetFactory();
	Class = Manager->LoadClassObject(Info.Object, FullPath);

	if (!Class)
	{
		// 如果映射表中没有找到，抛出异常或记录错误
		UE_LOG(LogTemp, Warning, TEXT("Unknown object type for LoadClass: %s"), *FullPath);
		throw;
		return;
	}
}

_UCNewUObject::_UCNewUObject(ucCONST UCEVariableInfoCC& Info)
{
	Object = nullptr;
	
	if (Info.Object == ucNULL)
	{
		throw;
		return;
	}
	
	if (Info.Object->_IsClass(&UCE_UObject::I))
	{
		// 使用映射表创建对象
		UCUnrealFactory* Manager = GetFactory();
		Object = Manager->CreateObject(Info.Object);
		
		if (!Object)
		{
			// 如果映射表中没有找到，抛出异常或记录错误
			UE_LOG(LogTemp, Warning, TEXT("Unknown object type for creation"));
			throw;
			return;
		}
	}
}

_UCTObjectPtr::_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info)
{
	this->Info = Info;
	FGCObject = ucNULL;
}

_UCTObjectPtr::_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info, _UCNewUObject& NewUObject)
{
	this->Info = Info;
	FGCObject = ucNULL;

	if (NewUObject.Info.Object && NewUObject.Info.Object == Info.Object && NewUObject.Object)
	{
		this->Object = NewUObject.Object;
		FGCObject = new UCUnrealGCObject(Info.Object, NewUObject.Object);
	}
	else
	{
		throw;
		return;
	}
}

_UCTObjectPtr::_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info, _UCLoadClass& LoadClass)
{
	this->Info = Info;
	FGCObject = ucNULL;

	if (LoadClass.Info.Object && Info.Object == &UCE_UClass::I && LoadClass.Class)
	{
		this->Object = LoadClass.Class;
		FGCObject = new UCUnrealGCObject(Info.Object, this->Object);
	}
	else
	{
		throw;
		return;
	}
}

_UCTObjectPtr::_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info, _UCCreateWidget& CreateWidget)
{
	this->Info = Info;
	FGCObject = ucNULL;

	if (CreateWidget.Info.Object && Info.Object == &UCE_UUserWidget::I && CreateWidget.Object)
	{
		this->Object = CreateWidget.Object;
		FGCObject = new UCUnrealGCObject(Info.Object, this->Object);
	}
	else
	{
		throw;
		return;
	}
}

_UCTObjectPtr::_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info, UObject* Object)
{
	this->Info = Info;
	FGCObject = ucNULL;

	if (Info.Object && Object)
	{
		this->Object = Object;
		FGCObject = new UCUnrealGCObject(Info.Object, this->Object);
	}
	else
	{
		throw;
		return;
	}
}

_UCTObjectPtr::~_UCTObjectPtr()
{
	if (FGCObject)
		delete FGCObject;
}

UObject* _UCTObjectPtr::operator ->()
{
	return (UObject*)Object;
}

_UCTObjectPtr& _UCTObjectPtr::operator =(_UCNewUObject& NewUObject)
{
	if (Info.Object && NewUObject.Object)
	{
		this->Object = NewUObject.Object;
		FGCObject = new UCUnrealGCObject(Info.Object, NewUObject.Object);
	}
	else
		throw;
	return *this;
}

_UCTObjectPtr& _UCTObjectPtr::operator =(_UCLoadClass& LoadClass)
{
	if (Info.Object && LoadClass.Class)
	{
		this->Object = LoadClass.Class;
		FGCObject = new UCUnrealGCObject(Info.Object, LoadClass.Class);
	}
	else
		throw;
	return *this;
}

_UCTObjectPtr& _UCTObjectPtr::operator =(_UCCreateWidget& CreateWidget)
{
	if (Info.Object && CreateWidget.Object)
	{
		this->Object = CreateWidget.Object;
		FGCObject = new UCUnrealGCObject(Info.Object, CreateWidget.Object);
	}
	else
		throw;
	return *this;
}
_UCTObjectPtr& _UCTObjectPtr::operator =(UObject* InObject)
{
	if (Info.Object && InObject)
	{
		this->Object = InObject;
		FGCObject = new UCUnrealGCObject(Info.Object, InObject);
	}
	else
		throw;
	return *this;
}

_UCTObjectPtr::TValue* _UCTObjectPtr::Get()
{
	return (TValue*)Object;
}

_UCCreateWidget::_UCCreateWidget(ucCONST UCEVariableInfoCC& Info, UObject* WorldContext, UClass* WidgetClass)
{
	Object = nullptr;

	if (Info.Object == ucNULL)
	{
		throw;
		return;
	}

	if (Info.Object->_IsClass(&UCE_UObject::I))
	{
		if (!WorldContext || !WidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateWidget: WorldContext or WidgetClass is null"));
			return;
		}


		// 优先尝试 PlayerController
		if (APlayerController* PC = Cast<APlayerController>(WorldContext))
		{
			Object = CreateWidget<UUserWidget>(PC, WidgetClass);
		}

		// 尝试 GameInstance
		if (UGameInstance* GI = Cast<UGameInstance>(WorldContext))
		{
			Object = CreateWidget<UUserWidget>(GI, WidgetClass);
		}

		// 尝试 UWorld
		if (UWorld* World = Cast<UWorld>(WorldContext))
		{
			Object = CreateWidget<UUserWidget>(World, WidgetClass);
		}

		// 尝试 Widge
		if (UWidget* Widge = Cast<UWidget>(WorldContext))
		{
			Object = CreateWidget<UUserWidget>(Widge, WidgetClass);
		}

		// 尝试 WidgetTree
		if (UWidgetTree* WidgetTree = Cast<UWidgetTree>(WorldContext))
		{
			Object = CreateWidget<UUserWidget>(WidgetTree, WidgetClass);
		}

		if (!Object)
		{
			// 如果映射表中没有找到，抛出异常或记录错误
			UE_LOG(LogTemp, Warning, TEXT("Unknown object type for CreateWidget"));
			throw;
			return;
		}
	}
}
