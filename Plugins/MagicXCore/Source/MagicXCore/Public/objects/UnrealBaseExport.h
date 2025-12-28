#ifndef _UnrealBaseExport_
#define _UnrealBaseExport_

#include "uccompile.h"
#include "ucnetbase.h"

#include "Components/Visual.h"

class UCUnrealGCObject : public FGCObject
{
public:
	UCSimpleMap<UObject*, ucCONST UCEClassInfo*>	MapGCObjects;

	UCUnrealGCObject()
	{

	}
	UCUnrealGCObject(ucCONST UCEClassInfo* ClassInfo, UObject* Object)
	{
		MapGCObjects.Add(Object, ClassInfo);
	}
public:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		for (ucINT i=0;i< MapGCObjects.GetSize();i++)
			Collector.AddReferencedObject(MapGCObjects.GetKeyAt(i));
	}

	virtual FString GetReferencerName() const override
	{
		return TEXT("UCUnrealObject");
	}
};

class UCUnrealGCExtra : public UCGCExtra
{
public:
	UCUnrealGCObject		GCObject;
public:
	UCUnrealGCExtra(){}
	virtual ~UCUnrealGCExtra() {}

	virtual ucVOID Release()
	{
		delete this;
	}

	ucVOID Add(UObject* Object)
	{
		GCObject.MapGCObjects.Add(Object, ucNULL);
	}
};

SCRIPT_DECLARE(, UCE_UObject, UObject, ucFALSE);
SCRIPT_DECLARE(, UCE_UClass, UClass, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UVisual, UVisual, UCE_UObject, ucFALSE);

template<class TYPE>
class UCLoadClass
{
	UCEVariableInfo			Info;
public:
	UCLoadClass(FString) {}
	ucCONST UCEVariableInfo& GetType() ucCONST { return Info; }

	UObject* operator ()() { return ucNULL; }
};

class _UCTSubclassOf;
class _UCLoadClass : public UCETemplate
{
public:
	_UCLoadClass(ucCONST UCEVariableInfoCC& Info);
	_UCLoadClass(ucCONST UCEVariableInfoCC& Info, const FString& Path);
private:
	friend class _UCTObjectPtr;
	friend class _UCTSubclassOf;
	UCEVariableInfo		Info;
	FString				Path;
	UClass*				Class;
};

SCRIPT_INHERIT_DECLARE(, UCE_UCLoadClass, _UCLoadClass, UCE_UCETemplate, ucFALSE);

// UCTSubclassOf 模板类
template<class TYPE>
class UCTSubclassOf
{
	UCEVariableInfo		Info;
public:
	ucCONST UCEVariableInfo& GetType() ucCONST { return Info; }

	// 获取 UClass* 指针
	UClass* Get() { return (UClass*)Info.Object; }

	// 转换为 UClass*
	operator UClass* () { return Get(); }

	UObject* operator ()() { return Get(); }
};

// UCTSubclassOf 的具体实现类
class _UCTSubclassOf : public UCETemplate
{
public:
	_UCTSubclassOf(ucCONST UCEVariableInfoCC& Info);
	_UCTSubclassOf(ucCONST UCEVariableInfoCC& Info, _UCLoadClass& InLoadClass);
private:
	friend class _UCTObjectPtr;
	UCEVariableInfo		Info;
	UClass*				Class;
};

SCRIPT_INHERIT_DECLARE(, UCE_UCTSubclassOf, _UCTSubclassOf, UCE_UCETemplate, ucFALSE);

template<class TYPE>
class UCNewUObject
{
	UCEVariableInfo			Info;
public:
	ucCONST UCEVariableInfo& GetType() ucCONST { return Info; }

	UObject* operator ()() { return ucNULL; }
};

class _UCNewUObject : public UCETemplate
{
public:
	_UCNewUObject(ucCONST UCEVariableInfoCC& Info);
private:
	friend class _UCTObjectPtr;
	UCEVariableInfo		Info;
	UObject*			Object;
};

SCRIPT_INHERIT_DECLARE(, UCE_UCNewUObject, _UCNewUObject, UCE_UCETemplate, ucFALSE);

class _UCCreateWidget : public UCETemplate
{
public:
	_UCCreateWidget(ucCONST UCEVariableInfoCC& Info, UObject* WorldContext, UClass* WidgetClass);
private:
	friend class _UCTObjectPtr;
	UCEVariableInfo		Info;
	UObject* Object;
};

SCRIPT_INHERIT_DECLARE(, UCE_UCCreateWidget, _UCCreateWidget, UCE_UCETemplate, ucFALSE);

template<class TYPE>
class UCCreateWidget
{
	UCEVariableInfo			Info;
public:
	ucCONST UCEVariableInfo& GetType() ucCONST { return Info; }

	UObject* operator ()() { return ucNULL; }
};

class _UCTObjectPtr : public UCETemplate
{
public:
	typedef UCETemplateArg0	TValue;

	UObject*			Object;
public:

	_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info);
	_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info, _UCNewUObject& NewUObject);
	_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info, _UCLoadClass& LoadClass);
	_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info, _UCCreateWidget& CreateWidget);
	_UCTObjectPtr(ucCONST UCEVariableInfoCC& Info, UObject* Object);

	~_UCTObjectPtr();

	UObject* operator->();

	_UCTObjectPtr& operator =(_UCNewUObject& NewUObject);
	_UCTObjectPtr& operator =(_UCLoadClass& LoadClass);
	_UCTObjectPtr& operator =(_UCCreateWidget& CreateWidget);
	_UCTObjectPtr& operator =(UObject* Object);

	TValue* Get();
private:
	UCEVariableInfo		Info;
	UCUnrealGCObject*	FGCObject;
};

SCRIPT_INHERIT_DECLARE(, UCE_UCTObjectPtr, _UCTObjectPtr, UCE_UCETemplate, ucFALSE);

template<class TYPE>
class UCTObjectPtr
{
	UCEVariableInfo			Info;
public:
	ucCONST UCEVariableInfo& GetType() ucCONST { return Info; }

	UObject* operator ()() { return ucNULL; }
};

// =========================================================================================
// 委托事件宏定义
// =========================================================================================

// 无参数委托事件宏
#define SCRIPT_UNREAL_FEVENT_0(SCRIPT_TYPE, UNREAL_TYPE, RETURN_TYPE) \
SCRIPT_IMPLEMENT_BEGIN(SCRIPT_TYPE, UNREAL_TYPE); \
SCRIPT_CONSTRUCT_0(); \
SCRIPT_CONSTRUCT_1(ucCONST SCRIPT_TYPE&); \
class meq0 { \
public: \
	ucInline meq0() { \
		UCEVariableInfo vt(&UCE_void::I, 0); \
		UCEVariableInfo v0 = UCEVariableInfo::Build<UCObject*>(); \
		UCEVariableInfo v1 = UCEVariableInfo::Build<ucVOID(*)()>(); \
		ThisType::I._AppendMethod((UCEMETHODINFO_CALL)&meq0::AddScript, 0, "AddScript", 0, &vt, 2, &v0, &v1); \
		ThisType::I._AppendMethod((UCEMETHODINFO_CALL)&meq0::Call, 0, "Call", 0, &vt, 0); \
	} \
	ucVOID AddScript(ucHANDLE obj, UCStack aryArgs, ucHANDLE& Return) \
	{ \
		UCObject* This = *(UCObject**)aryArgs[0]; \
		UCEMethodInfo* Method = *(UCEMethodInfo**)aryArgs[1]; \
		UScriptDelegateBridge* Bridge = NewObject<UScriptDelegateBridge>(); \
		Bridge->Set([This, Method]() { \
			if (This && Method) { \
				ucHANDLE Stack[1]; \
				ucHANDLE Return = 0; \
				((ucGenericClass*)Method->*(Method->FCall))(This, Stack, Return); \
			} \
		}); \
		((UNREAL_TYPE*)obj)->AddDynamic(Bridge, &UScriptDelegateBridge::Invoke); \
	} \
	ucVOID Call(ucHANDLE obj, UCStack aryArgs, ucHANDLE& Return) \
	{ \
		((ExternType*)(obj))->Broadcast(); \
	} \
}meq0; \
SCRIPT_MOPERATOR_1(eq, =, ucCONST SCRIPT_TYPE&); \
SCRIPT_DECONSTRUCT(); \
SCRIPT_IMPLEMENT_END(SCRIPT_TYPE);

// 单参数委托事件宏
#define SCRIPT_UNREAL_FEVENT_1(SCRIPT_TYPE, UNREAL_TYPE, RETURN_TYPE, PARAM1_TYPE) \
SCRIPT_IMPLEMENT_BEGIN(SCRIPT_TYPE, UNREAL_TYPE); \
SCRIPT_CONSTRUCT_0(); \
SCRIPT_CONSTRUCT_1(ucCONST SCRIPT_TYPE&); \
class meq1 \
{ \
public: \
	ucInline meq1() { \
		UCEVariableInfo vt(&UCE_void::I, 0); \
		UCEVariableInfo v0 = UCEVariableInfo::Build<UCObject*>(); \
		UCEVariableInfo v1 = UCEVariableInfo::Build<ucVOID(*)()>(); \
		ThisType::I._AppendMethod((UCEMETHODINFO_CALL)&meq1::AddScript, 0, "AddScript", 0, &vt, 2, &v0, &v1); \
		UCEVariableInfo& a_v0 = v1.AryArgs[0]; \
		ThisType::I._AppendMethod((UCEMETHODINFO_CALL)&meq1::Call, 0, "Call", 0, &vt, 0); \
	} \
	ucVOID AddScript(ucHANDLE obj, UCStack aryArgs, ucHANDLE& Return) \
	{ \
		UCObject* This = *(UCObject**)aryArgs[0]; \
		UCEMethodInfo* Method = *(UCEMethodInfo**)aryArgs[1]; \
		UScriptDelegateBridge* Bridge = NewObject<UScriptDelegateBridge>(); \
		Bridge->Set([This, Method]() { \
			if (This && Method) { \
				ucHANDLE Stack[1]; \
				ucHANDLE Return = 0; \
				((ucGenericClass*)Method->*(Method->FCall))(This, Stack, Return); \
			} \
		}); \
		((UNREAL_TYPE*)obj)->AddDynamic(Bridge, &UScriptDelegateBridge::Invoke); \
	} \
	ucVOID Call(ucHANDLE obj, UCStack aryArgs, ucHANDLE& Return) \
	{ \
		((ExternType*)(obj))->Broadcast(); \
	} \
}eq1; \
SCRIPT_MOPERATOR_1(eq, =, ucCONST SCRIPT_TYPE&); \
SCRIPT_DECONSTRUCT(); \
SCRIPT_IMPLEMENT_END(SCRIPT_TYPE);

#endif