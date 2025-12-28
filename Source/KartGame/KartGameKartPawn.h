// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ucnetbase.h"
#include "uccompile.h"

#include <GameFramework/Pawn.h>
#include <Components/BoxComponent.h>
#include <Components/SphereComponent.h>
#include <Components/CapsuleComponent.h>
#include <Components/StaticMeshComponent.h>
#include <PhysicsEngine/PhysicsConstraintComponent.h>
#include <GameFramework/PawnMovementComponent.h>
#include "CoreMinimal.h"
#include "KartGameKartPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
struct FInputActionValue;

/**
 *  Vehicle Pawn class
 *  Handles common functionality for all vehicle types,
 *  including input handling and camera management.
 *  
 *  Specific vehicle configurations are handled in subclasses.
 */
UCLASS(abstract, config = Game, BlueprintType)
class AKartGameKartPawn : public APawn
{
	GENERATED_BODY()

	// 物理中心 - 胶囊体负责移动和主碰撞
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Collision_Center;

	// =================================================================================================
	// 车身组件 - StaticMesh负责视觉表现
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent>	Collision_Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Collision_Wheel_FL;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Collision_Wheel_FR;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Collision_Wheel_BL;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Scene_Wheel_FL;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Scene_Wheel_FR;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Scene_Wheel_BL;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Scene_Wheel_BR;

	// =================================================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Collision_Wheel_BR;

	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category ="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent*	FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category ="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent*		FrontCamera;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category ="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent*	BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category ="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent*		BackCamera;
protected:

	/** Steering Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SteeringAction;

	/** Throttle Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ThrottleAction;

	/** Brake Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* BrakeAction;

	/** Handbrake Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* HandbrakeAction;

	/** Look Around Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAroundAction;

	/** Toggle Camera Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ToggleCameraAction;

	/** Reset Vehicle Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ResetVehicleAction;

	UPROPERTY()
	TObjectPtr<UPhysicalMaterial> SmoothMaterial;
	UPROPERTY()
	TObjectPtr<UPhysicalMaterial> RoughMaterial;

	/** Keeps track of which camera is active */
	bool bFrontCameraActive = false;
	bool bIsOnGround = false;
	FVector	CenterGroundNormal;

	float	CenterSteeringValue;
	float	SteeringSpeed = 0.0f;
	float	WheelPosition = 0.0f;
	float	WheelRadius = 48.0f;

	bool	WheelGroundContact[4];
// 	float	WheelHeightOffset[4];
// 	FVector	WheelGroundNormal[4];
// 	bool	PreviousWheelContact[4];
public:
	AKartGameKartPawn();

	// Begin Pawn interface

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	// End Pawn interface

	// Begin Actor interface

	virtual void Tick(float Delta) override;

	// End Actor interface

protected:
	UFUNCTION()
	void OnMoveForward(float Value);
	UFUNCTION()
	void OnMoveRight(float Value);

	UFUNCTION()
	void Center_OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
	/** Handles steering input */
	void Steering(const FInputActionValue& Value);

	/** Handles throttle input */
	void Throttle(const FInputActionValue& Value);

	/** Handles brake input */
	void Brake(const FInputActionValue& Value);

	/** Handles brake start/stop inputs */
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	/** Handles handbrake start/stop inputs */
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	/** Handles look around input */
	void LookAround(const FInputActionValue& Value);

	/** Handles toggle camera input */
	void ToggleCamera(const FInputActionValue& Value);

	/** Handles reset vehicle input */
	void ResetVehicle(const FInputActionValue& Value);
public:
	/** Handle steering input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSteering(float SteeringValue);

	/** Handle throttle input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoThrottle(float ThrottleValue);

	/** Handle brake input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrake(float BrakeValue);

	/** Handle brake start input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrakeStart();

	/** Handle brake stop input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoBrakeStop();

	/** Handle handbrake start input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHandbrakeStart();

	/** Handle handbrake stop input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoHandbrakeStop();

	/** Handle look input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoLookAround(float YawDelta);

	/** Handle toggle camera input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoToggleCamera();

	/** Handle reset vehicle input by input actions or mobile interface */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoResetVehicle();

protected:

	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

public:
	/** Returns the front spring arm subobject */
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	/** Returns the front camera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	/** Returns the back spring arm subobject */
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	/** Returns the back camera subobject */
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
private:
	void UpdateWheelPositionsToBodyTransform(); 
};
