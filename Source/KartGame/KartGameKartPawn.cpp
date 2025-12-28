// Copyright Epic Games, Inc. All Rights Reserved.

#include "KartGameKartPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include <PhysicsEngine/BodyInstance.h>

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "KartGame.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

AKartGameKartPawn::AKartGameKartPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SteeringSpeed = 0.0f;
	CenterSteeringValue = 0.0f;

	// 创建车身物理碰撞盒
	Collision_Center = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision_Center"));
	RootComponent = Collision_Center;

	// 设置胶囊体尺寸
	Collision_Center->SetCapsuleHalfHeight(168.0f);	// 车长的一半
	Collision_Center->SetCapsuleRadius(80.0f);		// 车宽的一半

	// 车身物理设置
	Collision_Center->SetSimulatePhysics(false); // ✅ Kinematic模式
	Collision_Center->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 保持碰撞检测
	Collision_Center->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);
	Collision_Center->SetCollisionResponseToAllChannels(ECR_Block);

	// (x, y, z)
	Collision_Center->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

	Collision_Center->SetNotifyRigidBodyCollision(true);
	Collision_Center->SetCollisionObjectType(ECollisionChannel::ECC_Vehicle);

	Collision_Center->OnComponentHit.AddDynamic(this, &AKartGameKartPawn::Center_OnHit);

	// 2. 创建车身组件（StaticMesh，负责视觉表现）
	Collision_Body = CreateDefaultSubobject<USceneComponent>(TEXT("Collision_Body"));
	Collision_Body->SetupAttachment(Collision_Center);
	
	Collision_Body->SetRelativeRotation(FRotator(0.0f, -90.0f, +90.0f)); // 设置旋转


	FVector WheelPositions[4] =
	{
		FVector(112.0f, -48.0f, -48.0f),	// 左前
		FVector(112.0f, +48.0f, -48.0f),	// 右前
		FVector(-92.0f, -48.0f, -48.0f),	// 左后
		FVector(-92.0f, +48.0f, -48.0f)		// 右后
	};
	FString Collision_WheelNames[4] =
	{
		FString(TEXT("Collision_Wheel_FL")),
		FString(TEXT("Collision_Wheel_FR")),
		FString(TEXT("Collision_Wheel_BL")),
		FString(TEXT("Collision_Wheel_BR"))
	};
	TObjectPtr<USphereComponent>* Collision_Wheels[4] =
	{
		&Collision_Wheel_FL,
		&Collision_Wheel_FR,
		&Collision_Wheel_BL,
		&Collision_Wheel_BR
	};

	FString Scene_WheelNames[4] =
	{
		FString(TEXT("Scene_Wheel_FL")),
		FString(TEXT("Scene_Wheel_FR")),
		FString(TEXT("Scene_Wheel_BL")),
		FString(TEXT("Scene_Wheel_BR"))
	};
	TObjectPtr<USceneComponent>* Scene_Wheels[4] =
	{
		&Scene_Wheel_FL,
		&Scene_Wheel_FR,
		&Scene_Wheel_BL,
		&Scene_Wheel_BR
	};
	FVector Scene_Offset[4] =
	{
		FVector(0.0f, -30.0f, 0.0f),
		FVector(0.0f, +10.0f, 0.0f),
		FVector(0.0f, -30.0f, 0.0f),
		FVector(0.0f, +10.0f, 0.0f),
	};
	for (int32 i = 0; i < 4; i++)
	{
		FString Collision_WheelName = Collision_WheelNames[i];

		TObjectPtr<USphereComponent>& Collision_Wheel = *Collision_Wheels[i];
		Collision_Wheel = CreateDefaultSubobject<USphereComponent>(*Collision_WheelName);
		Collision_Wheel->SetupAttachment(Collision_Body);
		Collision_Wheel->SetRelativeLocation(WheelPositions[i]);
		Collision_Wheel->SetSphereRadius(WheelRadius); // 轮子半径

		// 关键：只检测，不阻挡，不影响移动
		Collision_Wheel->SetSimulatePhysics(false);
		Collision_Wheel->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Collision_Wheel->SetCollisionObjectType(ECC_WorldDynamic);
		Collision_Wheel->SetCollisionResponseToAllChannels(ECR_Ignore);
		Collision_Wheel->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);

		FString Scene_WheelName = Scene_WheelNames[i];

		TObjectPtr<USceneComponent>& Scene_Wheel = *Scene_Wheels[i];
		Scene_Wheel = CreateDefaultSubobject<USceneComponent>(*Scene_WheelName);
		Scene_Wheel->SetupAttachment(Collision_Wheel);
		Scene_Wheel->SetRelativeLocation(Scene_Offset[i]);

		//Mesh_Wheels[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// 初始化状态
		WheelGroundContact[i] = false;
	}

	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(Collision_Body);
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(Collision_Body);
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
// 	GetMesh()->SetSimulatePhysics(true);
// 	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// 创建临时物理材质
	SmoothMaterial = NewObject<UPhysicalMaterial>();
	SmoothMaterial->Friction = 0.0f;
	SmoothMaterial->StaticFriction = 0.0f;
	SmoothMaterial->Restitution = 0.1f; 
	// 创建临时物理材质
	RoughMaterial = NewObject<UPhysicalMaterial>();
	RoughMaterial->Friction = 1.0f;
	RoughMaterial->StaticFriction = 1.0f;
	RoughMaterial->Restitution = 0.1f;
}

void AKartGameKartPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		PlayerInputComponent->BindAxis("MoveForward", this, &AKartGameKartPawn::OnMoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &AKartGameKartPawn::OnMoveRight);

		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AKartGameKartPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AKartGameKartPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AKartGameKartPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AKartGameKartPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AKartGameKartPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AKartGameKartPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AKartGameKartPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AKartGameKartPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AKartGameKartPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AKartGameKartPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &AKartGameKartPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AKartGameKartPawn::ResetVehicle);
	}
	else
	{
		UE_LOG(LogKartGame, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AKartGameKartPawn::Tick(float Delta)
{
	Super::Tick(Delta);

// 	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
// 	Mesh->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	if (bIsOnGround)
	{
		FVector UpVector = Collision_Body->GetUpVector();

		// 获取当前车身方向
		FVector CurrentUpVector = UpVector;
		FVector TargetUpVector = CenterGroundNormal; // 目标是与地面法线对齐

		// 计算需要的旋转
		FQuat CurrentRotation = GetActorQuat();
		FQuat TargetRotation = FQuat::FindBetweenNormals(CurrentUpVector, TargetUpVector) * CurrentRotation;

		// 使用球面插值平滑过渡
		float InterpSpeed = 0.2f * Delta;
		FQuat NewRotation = FQuat::Slerp(CurrentRotation, TargetRotation, InterpSpeed);

		// 应用新的旋转
		SetActorRotation(NewRotation);
	}

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));

	bIsOnGround = false;
}

void AKartGameKartPawn::Center_OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	FVector FVUp(0.0f, 0.0f, 1.0f);
	float fDot = Hit.Normal | FVUp;

	// 大概某个角度以上认为是地面
	if (fDot > 0.6f)
	{
		bIsOnGround = true;
		CenterGroundNormal = Hit.Normal;
	}
}

void AKartGameKartPawn::OnMoveForward(float Value)
{
	// 先用日志测试是否被调用
	UE_LOG(LogTemp, Warning, TEXT("MoveForward: %f"), Value);
}

void AKartGameKartPawn::OnMoveRight(float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveRight: %f"), Value);
}

void AKartGameKartPawn::Steering(const FInputActionValue& Value)
{
	// route the input
	// DoSteering(Value.Get<float>());
}

void AKartGameKartPawn::Throttle(const FInputActionValue& Value)
{
	// route the input
	// DoThrottle(Value.Get<float>());
}

void AKartGameKartPawn::Brake(const FInputActionValue& Value)
{
	// route the input
	DoBrake(Value.Get<float>());
}

void AKartGameKartPawn::StartBrake(const FInputActionValue& Value)
{
	// route the input
	DoBrakeStart();
}

void AKartGameKartPawn::StopBrake(const FInputActionValue& Value)
{
	// route the input
	DoBrakeStop();
}

void AKartGameKartPawn::StartHandbrake(const FInputActionValue& Value)
{
	// route the input
	DoHandbrakeStart();
}

void AKartGameKartPawn::StopHandbrake(const FInputActionValue& Value)
{
	// route the input
	DoHandbrakeStop();
}

void AKartGameKartPawn::LookAround(const FInputActionValue& Value)
{
	// route the input
	DoLookAround(Value.Get<float>());
}

void AKartGameKartPawn::ToggleCamera(const FInputActionValue& Value)
{
	// route the input
	DoToggleCamera();
}

void AKartGameKartPawn::ResetVehicle(const FInputActionValue& Value)
{
	// route the input
	DoResetVehicle();
}

void AKartGameKartPawn::DoSteering(float SteeringValue)
{
	CenterSteeringValue = SteeringValue;
}

void AKartGameKartPawn::DoThrottle(float ThrottleValue)
{
	FVector CurrentVelocity = Collision_Center->GetPhysicsLinearVelocity();
	FVector ForwardVector = Collision_Body->GetForwardVector();
	FVector UpVector = Collision_Body->GetUpVector();

	if (bIsOnGround)
	{
		float MaxSpeed = 2000.0f;

		CurrentVelocity = CurrentVelocity + ForwardVector * (ThrottleValue * 80.0f);
		float CurrSpeed = CurrentVelocity.Length();

		if (CurrSpeed > MaxSpeed)
		{
			CurrentVelocity.Normalize();
			CurrentVelocity = CurrentVelocity * MaxSpeed;
		}
	}

	float ForwardSpeed = FVector::DotProduct(CurrentVelocity, ForwardVector);;
	{
		float MaxAngleSpeed = 1.2f;
		float AccAngleSpeed = 0.02f;

		float CurrCenterSteeringValue = CenterSteeringValue;

		{
			if (ForwardSpeed < 0)
				CurrCenterSteeringValue = -CurrCenterSteeringValue;
		}

		//if (bIsOnGround)
		{
			if (abs(CurrCenterSteeringValue) > 0.0f)
			{
				if (CurrCenterSteeringValue > 0.0f)
					SteeringSpeed += AccAngleSpeed;
				else
					SteeringSpeed -= AccAngleSpeed;

				if (SteeringSpeed > +MaxAngleSpeed)
					SteeringSpeed = +MaxAngleSpeed;
				if (SteeringSpeed < -MaxAngleSpeed)
					SteeringSpeed = -MaxAngleSpeed;
			}
			else
			{
				float AngleColddown = 0.01f;

				if (SteeringSpeed > 0.0f)
				{
					SteeringSpeed -= AngleColddown;
					if (SteeringSpeed <= 0.0f)
						SteeringSpeed = 0.0f;
				}
				if (SteeringSpeed < 0.0f)
				{
					SteeringSpeed += AngleColddown;
					if (SteeringSpeed >= 0.0f)
						SteeringSpeed = 0.0f;
				}
			}
		}
	}

	{
		FRotator CurrentRotation = GetActorRotation(); // 这是Center的旋转
		FRotator NewRotation = FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw + SteeringSpeed, CurrentRotation.Roll); // 只改Yaw

		SetActorRotation(NewRotation);

		USceneComponent* Scene_Wheels[4];
		Scene_Wheels[0] = Scene_Wheel_FL;
		Scene_Wheels[1] = Scene_Wheel_FR;
		Scene_Wheels[2] = Scene_Wheel_BL;
		Scene_Wheels[3] = Scene_Wheel_BR;

		WheelPosition += ForwardSpeed / WheelRadius / 50.0f;

		for (int i = 0; i < 4; i++)
		{
			if (i < 2)
				Scene_Wheels[i]->SetRelativeRotation(FRotator(FMath::RadiansToDegrees(-WheelPosition), FMath::RadiansToDegrees(SteeringSpeed), 0.0f));
			else
				Scene_Wheels[i]->SetRelativeRotation(FRotator(FMath::RadiansToDegrees(-WheelPosition), 0.0f, 0.0f));
		}
	}

	// 将速度永远为0，不提供翻滚
	Collision_Center->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);

	if (abs(ThrottleValue) > 0.001f)
	{
		Collision_Center->SetPhysicsLinearVelocity(CurrentVelocity);

		Collision_Center->SetPhysMaterialOverride(SmoothMaterial);
	}
	else
	{
		Collision_Center->SetPhysMaterialOverride(RoughMaterial);
	}
}

void AKartGameKartPawn::DoBrake(float BrakeValue)
{
	if (BrakeValue != 0.0f)
		DoThrottle(BrakeValue);
}

void AKartGameKartPawn::DoBrakeStart()
{
	// call the Blueprint hook for the brake lights
	//BrakeLights(true);
}

void AKartGameKartPawn::DoBrakeStop()
{
	// call the Blueprint hook for the brake lights
	//BrakeLights(false);

	// reset brake input to zero
	//ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void AKartGameKartPawn::DoHandbrakeStart()
{
	// add the input
	//ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	//BrakeLights(true);
}

void AKartGameKartPawn::DoHandbrakeStop()
{
	// add the input
	//ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	//BrakeLights(false);
}

void AKartGameKartPawn::DoLookAround(float YawDelta)
{
	// rotate the spring arm
	BackSpringArm->AddLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
}

void AKartGameKartPawn::DoToggleCamera()
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void AKartGameKartPawn::DoResetVehicle()
{
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;

	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	//GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	//GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

#undef LOCTEXT_NAMESPACE