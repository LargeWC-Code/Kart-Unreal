#include "objects/UnrealEnvironmentExport.h"

// 引擎环境类脚本声明 - 提供脚本运行的环境上下文
SCRIPT_IMPLEMENT_BEGIN(UCE_UGameInstance, UGameInstance);
SCRIPT_IMPLEMENT_END(UCE_UGameInstance);

SCRIPT_IMPLEMENT_BEGIN(UCE_APlayerController, APlayerController);
SCRIPT_IMPLEMENT_END(UCE_APlayerController);

SCRIPT_IMPLEMENT_BEGIN(UCE_UWorld, UWorld);
	SCRIPT_RMFUNCTION_0(UCE_APlayerController*, GetFirstPlayerController);
SCRIPT_IMPLEMENT_END(UCE_UWorld);

SCRIPT_IMPLEMENT_BEGIN(UCE_UWorldProxy, UWorldProxy);
	SCRIPT_RMOPERATOR_0(UCE_UWorld*, s, ->);
SCRIPT_IMPLEMENT_END(UCE_UWorldProxy);


SCRIPT_IMPLEMENT_BEGIN(UCE_UCUnrealEventDispatcher, UCUnrealEventDispatcher);
	SCRIPT_RMFUNCTION_2(UCE_INT, Register, ucCONST UCE_UCString&, UCE_UCEvent);
	SCRIPT_RMFUNCTION_1(UCE_UCEvent*, Find, ucCONST UCE_UCString&);

	SCRIPT_RRSMFUNCTION_0(UCE_UCUnrealEventDispatcher&, GetStatic);
SCRIPT_IMPLEMENT_END(UCE_UCUnrealEventDispatcher);

UWorld* UCGetUWorld()
{
	return GWorld;
}

SCRIPT_RFUNCTION_0(UCE_UWorld*, UCGetUWorld);

ucBOOL UCUnrealEventDispatcher::Register(ucCONST UCString& Name, UCEvent Event)
{
	MapEvents.Add(Name, Event);
	return ucTRUE;
}

UCEvent* UCUnrealEventDispatcher::Find(ucCONST UCString& Name)
{
	UCAVLTreeMap<UCString, UCEvent>::Node* Node = MapEvents.Find(Name);

	if (Node == ucNULL)
		return ucNULL;

	return &Node->second;
}

UCUnrealEventDispatcher& UCUnrealEventDispatcher::GetStatic()
{
	static UCUnrealEventDispatcher StaticUnrealEventDispatcher;
	return StaticUnrealEventDispatcher;
}
