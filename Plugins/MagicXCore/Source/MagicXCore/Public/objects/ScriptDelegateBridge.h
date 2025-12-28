#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/Button.h"
#include "ScriptDelegateBridge.generated.h"

/**
 * 脚本委托桥接类
 * 允许非UObject脚本代码响应UMG动态委托事件
 * 支持多种参数类型的TFunction回调
 */
class UCUnrealGCExtra;
UCLASS(BlueprintType, Blueprintable)
class MAGICXCORE_API UScriptDelegateBridge : public UObject
{
    GENERATED_BODY()
public:
    UCUnrealGCExtra*       GCExtra;
public:
    UScriptDelegateBridge();
    virtual ~UScriptDelegateBridge();

    // 委托类型枚举
    enum class EDelegateType : uint8
    {
        None,
        Void,
        Int32,
        Int32String,
        Int32StringFloat,
        Int32StringFloatBool,
        Int32StringFloatBoolVector,
        Int32StringFloatBoolVectorColor
    };

    /**
     * 设置无参数回调函数
     * @param Callback 无参数的lambda或函数对象
     */
    void Set(TFunction<void()> Callback);

    /**
     * 设置单int32参数回调函数
     * @param Callback 接受int32参数的lambda或函数对象
     */
    void Set(TFunction<void(int32)> Callback);

    /**
     * 设置int32和FString参数回调函数
     * @param Callback 接受int32和FString参数的lambda或函数对象
     */
    void Set(TFunction<void(int32, FString)> Callback);

    /**
     * 设置三参数回调函数
     * @param Callback 接受int32、FString和float参数的lambda或函数对象
     */
    void Set(TFunction<void(int32, FString, float)> Callback);

    /**
     * 设置四参数回调函数
     * @param Callback 接受int32、FString、float和bool参数的lambda或函数对象
     */
    void Set(TFunction<void(int32, FString, float, bool)> Callback);

    /**
     * 设置五参数回调函数
     * @param Callback 接受int32、FString、float、bool和FVector参数的lambda或函数对象
     */
    void Set(TFunction<void(int32, FString, float, bool, FVector)> Callback);

    /**
     * 设置六参数回调函数
     * @param Callback 接受int32、FString、float、bool、FVector和FColor参数的lambda或函数对象
     */
    void Set(TFunction<void(int32, FString, float, bool, FVector, FColor)> Callback);

    /**
     * 清除当前设置的回调函数
     */
    void Clear();

    // UFUNCTION调用入口 - 供动态委托绑定使用

    /**
     * 调用无参数回调
     */
    UFUNCTION()
    void Invoke();

    /**
     * 调用单int32参数回调
     * @param Value int32参数值
     */
    UFUNCTION()
    void InvokeInt(int32 Value);

    /**
     * 调用int32和FString参数回调
     * @param IntValue int32参数值
     * @param StringValue FString参数值
     */
    UFUNCTION()
    void InvokeIntString(int32 IntValue, FString StringValue);

    /**
     * 调用三参数回调
     * @param IntValue int32参数值
     * @param StringValue FString参数值
     * @param FloatValue float参数值
     */
    UFUNCTION()
    void InvokeIntStringFloat(int32 IntValue, FString StringValue, float FloatValue);

    /**
     * 调用四参数回调
     * @param IntValue int32参数值
     * @param StringValue FString参数值
     * @param FloatValue float参数值
     * @param BoolValue bool参数值
     */
    UFUNCTION()
    void InvokeIntStringFloatBool(int32 IntValue, FString StringValue, float FloatValue, bool BoolValue);

    /**
     * 调用五参数回调
     * @param IntValue int32参数值
     * @param StringValue FString参数值
     * @param FloatValue float参数值
     * @param BoolValue bool参数值
     * @param VectorValue FVector参数值
     */
    UFUNCTION()
    void InvokeIntStringFloatBoolVector(int32 IntValue, FString StringValue, float FloatValue, bool BoolValue, FVector VectorValue);

    /**
     * 调用六参数回调
     * @param IntValue int32参数值
     * @param StringValue FString参数值
     * @param FloatValue float参数值
     * @param BoolValue bool参数值
     * @param VectorValue FVector参数值
     * @param ColorValue FColor参数值
     */
    UFUNCTION()
    void InvokeIntStringFloatBoolVectorColor(int32 IntValue, FString StringValue, float FloatValue, bool BoolValue, FVector VectorValue, FColor ColorValue);

protected:
    virtual void BeginDestroy() override;

private:
    // 委托类型
    EDelegateType DelegateType;

    // 使用union存储不同类型的TFunction，避免为每个类型创建单独的桥接类
    union FDelegateStorage
    {
        TFunction<void()> VoidCallback;
        TFunction<void(int32)> Int32Callback;
        TFunction<void(int32, FString)> Int32StringCallback;
        TFunction<void(int32, FString, float)> Int32StringFloatCallback;
        TFunction<void(int32, FString, float, bool)> Int32StringFloatBoolCallback;
        TFunction<void(int32, FString, float, bool, FVector)> Int32StringFloatBoolVectorCallback;
        TFunction<void(int32, FString, float, bool, FVector, FColor)> Int32StringFloatBoolVectorColorCallback;

        FDelegateStorage() {}
        ~FDelegateStorage() {}
    } DelegateStorage;

    /**
     * 安全析构当前存储的委托
     */
    void DestroyCurrentDelegate();
};
