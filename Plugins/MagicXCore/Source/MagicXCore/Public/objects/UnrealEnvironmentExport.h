#ifndef _UnrealEnvironmentExport_
#define _UnrealEnvironmentExport_

#include "MagicXCore.h"
#include "UnrealBaseExport.h"

// 引擎环境类导出 - 提供脚本运行的环境上下文
SCRIPT_INHERIT_DECLARE(, UCE_UGameInstance, UGameInstance, UCE_UObject, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_APlayerController, APlayerController, UCE_UObject, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UWorld, UWorld, UCE_UObject, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UWorldProxy, UWorldProxy, UCE_UObject, ucFALSE);

class MAGICXCORE_API UCUnrealEventDispatcher
{
public:
	ucBOOL		Register(ucCONST UCString&, UCEvent);

	UCEvent*	Find(ucCONST UCString&);

	static UCUnrealEventDispatcher& GetStatic();
private:
	UCAVLTreeMap<UCString, UCEvent>		MapEvents;
};

SCRIPT_DECLARE(, UCE_UCUnrealEventDispatcher, UCUnrealEventDispatcher, ucFALSE);

#endif
