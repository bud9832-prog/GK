// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKCharacter.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

AGKCharacter::AGKCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CharacterAkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("CharacterAkComponent"));
	CharacterAkComponent->SetupAttachment(GetCapsuleComponent());

	ApplyCharacterTuning();
}

void AGKCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyCharacterTuning();
}

void AGKCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyCharacterTuning();
	Stamina = MaxStamina;

	if (!DefaultMappingContext) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (IsLocallyControlled())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AGKCharacter::SetupWwiseDistanceProbe);
	}
}

void AGKCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsLocallyControlled())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AGKCharacter::SetupWwiseDistanceProbe);
	}
}

void AGKCharacter::UnPossessed()
{
	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		if (UAkComponent* SpatialListener = AudioDevice->GetSpatialAudioListener())
		{
			AudioDevice->SetDistanceProbe(SpatialListener, nullptr);
		}
	}

	Super::UnPossessed();
}

void AGKCharacter::SetupWwiseDistanceProbe()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice || !CharacterAkComponent)
	{
		return;
	}

	UAkComponent* SpatialListener = AudioDevice->GetSpatialAudioListener();
	if (!SpatialListener)
	{
		return;
	}

	AudioDevice->SetDistanceProbe(SpatialListener, CharacterAkComponent);
}

void AGKCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return;

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGKCharacter::HandleMove);
	}

	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGKCharacter::HandleLook);
	}

	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
}

void AGKCharacter::HandleMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (!Controller) return;

	const FRotator Yaw(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

void AGKCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AGKCharacter::ApplyCharacterTuning()
{
	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, RotationRate, 0.f);
	Movement->JumpZVelocity = JumpZVelocity;
	Movement->AirControl = AirControl;
	Movement->MaxWalkSpeed = MaxWalkSpeed;
	Movement->MinAnalogWalkSpeed = MinAnalogWalkSpeed;
	Movement->BrakingDecelerationWalking = BrakingDecelerationWalking;
	Movement->BrakingDecelerationFalling = BrakingDecelerationFalling;

	CameraBoom->TargetArmLength = CameraArmLength;
}
