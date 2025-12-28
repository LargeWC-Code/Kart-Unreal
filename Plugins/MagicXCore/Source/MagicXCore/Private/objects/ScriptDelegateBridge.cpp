#include "objects/ScriptDelegateBridge.h"
#include "Engine/Engine.h"

#include "uccompile.h"
#include "ucnetbase.h"

#include "objects/UnrealBaseExport.h"

UScriptDelegateBridge::UScriptDelegateBridge()
    : DelegateType(EDelegateType::None)
{
    GCExtra = new UCUnrealGCExtra();
    GCExtra->Add(this);
    // 初始化union，但不构造任何TFunction
    new(&DelegateStorage) FDelegateStorage();
}

UScriptDelegateBridge::~UScriptDelegateBridge()
{
    DestroyCurrentDelegate();
    GCExtra->Release();
}

void UScriptDelegateBridge::Set(TFunction<void()> Callback)
{
    // 先销毁当前委托
    DestroyCurrentDelegate();
    
    // 设置新的委托类型
    DelegateType = EDelegateType::Void;
    
    // 使用placement new在union中构造TFunction
    new(&DelegateStorage.VoidCallback) TFunction<void()>(MoveTemp(Callback));
}

void UScriptDelegateBridge::Set(TFunction<void(int32)> Callback)
{
    // 先销毁当前委托
    DestroyCurrentDelegate();
    
    // 设置新的委托类型
    DelegateType = EDelegateType::Int32;
    
    // 使用placement new在union中构造TFunction
    new(&DelegateStorage.Int32Callback) TFunction<void(int32)>(MoveTemp(Callback));
}

void UScriptDelegateBridge::Set(TFunction<void(int32, FString)> Callback)
{
    // 先销毁当前委托
    DestroyCurrentDelegate();
    
    // 设置新的委托类型
    DelegateType = EDelegateType::Int32String;
    
    // 使用placement new在union中构造TFunction
    new(&DelegateStorage.Int32StringCallback) TFunction<void(int32, FString)>(MoveTemp(Callback));
}

void UScriptDelegateBridge::Set(TFunction<void(int32, FString, float)> Callback)
{
    // 先销毁当前委托
    DestroyCurrentDelegate();
    
    // 设置新的委托类型
    DelegateType = EDelegateType::Int32StringFloat;
    
    // 使用placement new在union中构造TFunction
    new(&DelegateStorage.Int32StringFloatCallback) TFunction<void(int32, FString, float)>(MoveTemp(Callback));
}

void UScriptDelegateBridge::Set(TFunction<void(int32, FString, float, bool)> Callback)
{
    // 先销毁当前委托
    DestroyCurrentDelegate();
    
    // 设置新的委托类型
    DelegateType = EDelegateType::Int32StringFloatBool;
    
    // 使用placement new在union中构造TFunction
    new(&DelegateStorage.Int32StringFloatBoolCallback) TFunction<void(int32, FString, float, bool)>(MoveTemp(Callback));
}

void UScriptDelegateBridge::Set(TFunction<void(int32, FString, float, bool, FVector)> Callback)
{
    // 先销毁当前委托
    DestroyCurrentDelegate();
    
    // 设置新的委托类型
    DelegateType = EDelegateType::Int32StringFloatBoolVector;
    
    // 使用placement new在union中构造TFunction
    new(&DelegateStorage.Int32StringFloatBoolVectorCallback) TFunction<void(int32, FString, float, bool, FVector)>(MoveTemp(Callback));
}

void UScriptDelegateBridge::Set(TFunction<void(int32, FString, float, bool, FVector, FColor)> Callback)
{
    // 先销毁当前委托
    DestroyCurrentDelegate();
    
    // 设置新的委托类型
    DelegateType = EDelegateType::Int32StringFloatBoolVectorColor;
    
    // 使用placement new在union中构造TFunction
    new(&DelegateStorage.Int32StringFloatBoolVectorColorCallback) TFunction<void(int32, FString, float, bool, FVector, FColor)>(MoveTemp(Callback));
}

void UScriptDelegateBridge::Clear()
{
    DestroyCurrentDelegate();
    DelegateType = EDelegateType::None;
}

void UScriptDelegateBridge::Invoke()
{
    if (DelegateType == EDelegateType::Void && DelegateStorage.VoidCallback)
    {
        DelegateStorage.VoidCallback();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UScriptDelegateBridge::Invoke() called but no void() callback is set"));
    }
}

void UScriptDelegateBridge::InvokeInt(int32 Value)
{
    if (DelegateType == EDelegateType::Int32 && DelegateStorage.Int32Callback)
    {
        DelegateStorage.Int32Callback(Value);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UScriptDelegateBridge::InvokeInt() called but no void(int32) callback is set"));
    }
}

void UScriptDelegateBridge::InvokeIntString(int32 IntValue, FString StringValue)
{
    if (DelegateType == EDelegateType::Int32String && DelegateStorage.Int32StringCallback)
    {
        DelegateStorage.Int32StringCallback(IntValue, StringValue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UScriptDelegateBridge::InvokeIntString() called but no void(int32, FString) callback is set"));
    }
}

void UScriptDelegateBridge::InvokeIntStringFloat(int32 IntValue, FString StringValue, float FloatValue)
{
    if (DelegateType == EDelegateType::Int32StringFloat && DelegateStorage.Int32StringFloatCallback)
    {
        DelegateStorage.Int32StringFloatCallback(IntValue, StringValue, FloatValue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UScriptDelegateBridge::InvokeIntStringFloat() called but no void(int32, FString, float) callback is set"));
    }
}

void UScriptDelegateBridge::InvokeIntStringFloatBool(int32 IntValue, FString StringValue, float FloatValue, bool BoolValue)
{
    if (DelegateType == EDelegateType::Int32StringFloatBool && DelegateStorage.Int32StringFloatBoolCallback)
    {
        DelegateStorage.Int32StringFloatBoolCallback(IntValue, StringValue, FloatValue, BoolValue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UScriptDelegateBridge::InvokeIntStringFloatBool() called but no void(int32, FString, float, bool) callback is set"));
    }
}

void UScriptDelegateBridge::InvokeIntStringFloatBoolVector(int32 IntValue, FString StringValue, float FloatValue, bool BoolValue, FVector VectorValue)
{
    if (DelegateType == EDelegateType::Int32StringFloatBoolVector && DelegateStorage.Int32StringFloatBoolVectorCallback)
    {
        DelegateStorage.Int32StringFloatBoolVectorCallback(IntValue, StringValue, FloatValue, BoolValue, VectorValue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UScriptDelegateBridge::InvokeIntStringFloatBoolVector() called but no void(int32, FString, float, bool, FVector) callback is set"));
    }
}

void UScriptDelegateBridge::InvokeIntStringFloatBoolVectorColor(int32 IntValue, FString StringValue, float FloatValue, bool BoolValue, FVector VectorValue, FColor ColorValue)
{
    if (DelegateType == EDelegateType::Int32StringFloatBoolVectorColor && DelegateStorage.Int32StringFloatBoolVectorColorCallback)
    {
        DelegateStorage.Int32StringFloatBoolVectorColorCallback(IntValue, StringValue, FloatValue, BoolValue, VectorValue, ColorValue);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UScriptDelegateBridge::InvokeIntStringFloatBoolVectorColor() called but no void(int32, FString, float, bool, FVector, FColor) callback is set"));
    }
}

void UScriptDelegateBridge::BeginDestroy()
{
    DestroyCurrentDelegate();
    Super::BeginDestroy();
}

void UScriptDelegateBridge::DestroyCurrentDelegate()
{
    switch (DelegateType)
    {
    case EDelegateType::Void:
        DelegateStorage.VoidCallback.~TFunction<void()>();
        break;
    case EDelegateType::Int32:
        DelegateStorage.Int32Callback.~TFunction<void(int32)>();
        break;
    case EDelegateType::Int32String:
        DelegateStorage.Int32StringCallback.~TFunction<void(int32, FString)>();
        break;
    case EDelegateType::Int32StringFloat:
        DelegateStorage.Int32StringFloatCallback.~TFunction<void(int32, FString, float)>();
        break;
    case EDelegateType::Int32StringFloatBool:
        DelegateStorage.Int32StringFloatBoolCallback.~TFunction<void(int32, FString, float, bool)>();
        break;
    case EDelegateType::Int32StringFloatBoolVector:
        DelegateStorage.Int32StringFloatBoolVectorCallback.~TFunction<void(int32, FString, float, bool, FVector)>();
        break;
    case EDelegateType::Int32StringFloatBoolVectorColor:
        DelegateStorage.Int32StringFloatBoolVectorColorCallback.~TFunction<void(int32, FString, float, bool, FVector, FColor)>();
        break;
    case EDelegateType::None:
    default:
        // 没有需要销毁的委托
        break;
    }
    
    DelegateType = EDelegateType::None;
}

