// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKCharacter.h"
#include "GKEnemyCharacter.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace GKCharacterLog
{
	static void DebugPrint(const AGKCharacter* Character, const FString& Message, const FColor Color = FColor::Cyan)
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
		if (Character && GEngine && Character->IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
		}
	}
}

const UGKCombatConfig* AGKCharacter::GetCombatConfig() const
{
	return CombatConfig ? CombatConfig.Get() : GetDefault<UGKCombatConfig>();
}

const UGKPlayerStatsConfig* AGKCharacter::GetPlayerStatsConfig() const
{
	return PlayerStatsConfig ? PlayerStatsConfig.Get() : GetDefault<UGKPlayerStatsConfig>();
}

UDataTable* AGKCharacter::GetComboTable()
{
	if (CombatConfig && CombatConfig->ComboAttackTable)
	{
		return CombatConfig->ComboAttackTable;
	}

	if (!RuntimeComboTable)
	{
		RuntimeComboTable = UGKCombatConfig::CreateDefaultComboAttackTable(this);
	}

	return RuntimeComboTable;
}

const FGKComboAttackRow* AGKCharacter::GetComboRow(int32 ComboIndex) const
{
	if (CombatConfig && CombatConfig->ComboAttackTable)
	{
		return GetCombatConfig()->GetComboRowByIndex(ComboIndex);
	}

	if (RuntimeComboTable)
	{
		for (const TPair<FName, uint8*>& RowPair : RuntimeComboTable->GetRowMap())
		{
			const FGKComboAttackRow* Row = reinterpret_cast<const FGKComboAttackRow*>(RowPair.Value);
			if (Row && Row->ComboIndex == ComboIndex)
			{
				return Row;
			}
		}
	}

	return nullptr;
}

AGKCharacter::AGKCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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

void AGKCharacter::EnsureRuntimeConfigs()
{
	if (!CombatConfig)
	{
		CombatConfig = NewObject<UGKCombatConfig>(this, TEXT("RuntimeCombatConfig"));
	}

	if (!PlayerStatsConfig)
	{
		PlayerStatsConfig = NewObject<UGKPlayerStatsConfig>(this, TEXT("RuntimePlayerStatsConfig"));
	}

	if (CombatConfig && !CombatConfig->ComboAttackTable)
	{
		GetComboTable();
	}
}

void AGKCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyCharacterTuning();
}

void AGKCharacter::BeginPlay()
{
	Super::BeginPlay();

	EnsureRuntimeConfigs();

	const UGKCombatConfig* Config = GetCombatConfig();
	const UGKPlayerStatsConfig* Stats = GetPlayerStatsConfig();

	CurrentHP = Stats->MaxHP;
	CurrentStamina = Config->MaxStamina;
	HealItemRemaining = Stats->HealItem_StartCount;
	CombatState = EGKCombatState::Idle;

	ApplyCharacterTuning();

	if (!DefaultMappingContext)
	{
		return;
	}

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

void AGKCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CombatState == EGKCombatState::Death)
	{
		return;
	}

	if (CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::JumpAttack)
	{
		ActiveMotionElapsed += DeltaSeconds;
	}

	UpdateLockOn(DeltaSeconds);
	UpdateSprintStaminaDrain(DeltaSeconds);
	UpdateStaminaRegen(DeltaSeconds);
	UpdateLocomotionStateFromInput();
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
	if (!EIC)
	{
		return;
	}

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGKCharacter::HandleMove);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &AGKCharacter::HandleMove);
	}

	if (LookAction)
	{
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGKCharacter::HandleLook);
	}

	if (SprintAction)
	{
		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AGKCharacter::HandleSprintStarted);
		EIC->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AGKCharacter::HandleSprintTriggered);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AGKCharacter::HandleSprintCompleted);
	}

	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AGKCharacter::HandleJumpStarted);
	}

	if (EvadeAction)
	{
		EIC->BindAction(EvadeAction, ETriggerEvent::Started, this, &AGKCharacter::HandleEvadeStarted);
	}

	if (AttackAction)
	{
		EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AGKCharacter::HandleAttackStarted);
	}

	if (HealAction)
	{
		EIC->BindAction(HealAction, ETriggerEvent::Started, this, &AGKCharacter::HandleHealStarted);
	}

	if (LockOnAction)
	{
		EIC->BindAction(LockOnAction, ETriggerEvent::Started, this, &AGKCharacter::HandleLockOnStarted);
	}
}

void AGKCharacter::HandleMove(const FInputActionValue& Value)
{
	MoveInput = Value.Get<FVector2D>();
	ApplyMovementFromCachedInput();
}

void AGKCharacter::ApplyMovementFromCachedInput()
{
	if (!Controller || MoveInput.IsNearlyZero())
	{
		return;
	}

	if (CombatState == EGKCombatState::Death || CombatState == EGKCombatState::HitStun
		|| CombatState == EGKCombatState::Evade_Active || CombatState == EGKCombatState::Evade_Recovery
		|| CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::Heal
		|| CombatState == EGKCombatState::JumpAttack)
	{
		return;
	}

	const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), MoveInput.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), MoveInput.X);
}

void AGKCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AGKCharacter::HandleSprintStarted(const FInputActionValue& /*Value*/)
{
	if (CombatState == EGKCombatState::Death || CombatState == EGKCombatState::HitStun)
	{
		return;
	}

	bSprintPressed = true;
	if (CanTransitionToSprint())
	{
		EnterSprintState();
	}
}

void AGKCharacter::HandleSprintTriggered(const FInputActionValue& /*Value*/)
{
	if (CombatState == EGKCombatState::Sprint)
	{
		ApplyMovementFromCachedInput();
		return;
	}

	if (bSprintPressed && CanTransitionToSprint())
	{
		EnterSprintState();
	}
}

void AGKCharacter::HandleSprintCompleted(const FInputActionValue& /*Value*/)
{
	bSprintPressed = false;

	if (CombatState == EGKCombatState::Death || CombatState == EGKCombatState::HitStun)
	{
		return;
	}

	if (CombatState == EGKCombatState::Sprint)
	{
		ExitSprintState();
	}
}

void AGKCharacter::HandleJumpStarted(const FInputActionValue& /*Value*/)
{
	if (CanTransitionToJump())
	{
		TryStartJump();
	}
}

void AGKCharacter::HandleEvadeStarted(const FInputActionValue& /*Value*/)
{
	if (CanTransitionToEvade())
	{
		TryStartEvade();
	}
}

void AGKCharacter::HandleAttackStarted(const FInputActionValue& Value)
{
	if (CombatState == EGKCombatState::Jump)
	{
		if (CanTransitionToJumpAttack())
		{
			TryStartJumpAttack();
		}
		return;
	}

	if (CombatState == EGKCombatState::Attack)
	{
		const FGKComboAttackRow* Row = GetComboRow(CurrentComboIndex);
		if (!Row || Row->ComboInputWindowEnd <= Row->ComboInputWindowStart)
		{
			return;
		}

		if (ActiveMotionElapsed >= Row->ComboInputWindowStart && ActiveMotionElapsed <= Row->ComboInputWindowEnd)
		{
			ComboInputBuffered = true;
			GKCharacterLog::DebugPrint(this, FString::Printf(TEXT("Combo buffer ON (index %d)"), CurrentComboIndex));
		}
		return;
	}

	if (CanTransitionToAttack())
	{
		TryStartAttack();
	}
}

void AGKCharacter::HandleHealStarted(const FInputActionValue& Value)
{
	if (CanTransitionToHeal())
	{
		TryStartHeal();
	}
}

void AGKCharacter::HandleLockOnStarted(const FInputActionValue& Value)
{
	if (CombatState != EGKCombatState::Death)
	{
		ToggleLockOn();
	}
}

bool AGKCharacter::CanAcceptCombatInput() const
{
	return CombatState != EGKCombatState::Death && CombatState != EGKCombatState::HitStun
		&& CombatState != EGKCombatState::Evade_Active && CombatState != EGKCombatState::Evade_Recovery
		&& CombatState != EGKCombatState::Jump && CombatState != EGKCombatState::JumpAttack;
}

bool AGKCharacter::IsAirborneCombatState() const
{
	return CombatState == EGKCombatState::Jump || CombatState == EGKCombatState::JumpAttack;
}

bool AGKCharacter::CanTransitionToEvade() const
{
	if (!CanAcceptCombatInput())
	{
		return false;
	}

	if (CombatState == EGKCombatState::Evade_Active || CombatState == EGKCombatState::Evade_Recovery)
	{
		return false;
	}

	if (CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::Heal
		|| CombatState == EGKCombatState::Idle || CombatState == EGKCombatState::Run
		|| CombatState == EGKCombatState::Sprint)
	{
		return HasEnoughStamina(GetCombatConfig()->Stamina_Evade);
	}

	return false;
}

bool AGKCharacter::CanTransitionToSprint() const
{
	if (!CanAcceptCombatInput() || CombatState == EGKCombatState::Sprint)
	{
		return false;
	}

	if (CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::Heal
		|| CombatState == EGKCombatState::Evade_Active || CombatState == EGKCombatState::Evade_Recovery)
	{
		return false;
	}

	return CurrentStamina > 0.f;
}

bool AGKCharacter::CanTransitionToJump() const
{
	if (CombatState == EGKCombatState::Death || CombatState == EGKCombatState::HitStun)
	{
		return false;
	}

	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement || !Movement->IsMovingOnGround())
	{
		return false;
	}

	return CombatState == EGKCombatState::Idle || CombatState == EGKCombatState::Run
		|| CombatState == EGKCombatState::Sprint;
}

bool AGKCharacter::CanTransitionToAttack() const
{
	if (IsAirborneCombatState())
	{
		return false;
	}

	if (CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::Death
		|| CombatState == EGKCombatState::HitStun || CombatState == EGKCombatState::Evade_Active
		|| CombatState == EGKCombatState::Evade_Recovery || CombatState == EGKCombatState::Heal)
	{
		return false;
	}

	const FGKComboAttackRow* Row = GetComboRow(0);
	return Row && HasEnoughStamina(Row->StaminaCost);
}

bool AGKCharacter::CanTransitionToJumpAttack() const
{
	if (CombatState != EGKCombatState::Jump)
	{
		return false;
	}

	return HasEnoughStamina(GetCombatConfig()->Stamina_JumpAttack);
}

bool AGKCharacter::CanTransitionToHeal() const
{
	if (!CanAcceptCombatInput() || HealItemRemaining <= 0)
	{
		return false;
	}

	return CombatState == EGKCombatState::Idle || CombatState == EGKCombatState::Run
		|| CombatState == EGKCombatState::Sprint;
}

bool AGKCharacter::HasEnoughStamina(float Cost) const
{
	return CurrentStamina >= Cost;
}

void AGKCharacter::SetCombatState(EGKCombatState NewState)
{
	if (CombatState == NewState)
	{
		return;
	}

	if (NewState != EGKCombatState::Attack && CombatState == EGKCombatState::Attack)
	{
		ResetComboState();
	}

	if (NewState == EGKCombatState::Jump && CombatState != EGKCombatState::Jump)
	{
		ResetComboState();
	}

	CombatState = NewState;
	GKCharacterLog::DebugPrint(this, FString::Printf(TEXT("State -> %s"), *UEnum::GetValueAsString(NewState)), FColor::Green);
}

void AGKCharacter::ResetComboState()
{
	CurrentComboIndex = 0;
	ComboInputBuffered = false;
	bAttackHitApplied = false;
	ActiveMotionElapsed = 0.f;
}

void AGKCharacter::ApplyRunSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = GetCombatConfig()->RunSpeed;
}

void AGKCharacter::ApplySprintSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = GetCombatConfig()->SprintSpeed;
}

void AGKCharacter::UpdateLocomotionStateFromInput()
{
	if (CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::Evade_Active
		|| CombatState == EGKCombatState::Evade_Recovery || CombatState == EGKCombatState::Heal
		|| CombatState == EGKCombatState::HitStun || CombatState == EGKCombatState::Death
		|| CombatState == EGKCombatState::Jump || CombatState == EGKCombatState::JumpAttack)
	{
		return;
	}

	if (CombatState == EGKCombatState::Sprint)
	{
		if (CurrentStamina <= 0.f)
		{
			ExitSprintState();
		}
		return;
	}

	if (MoveInput.IsNearlyZero())
	{
		if (CombatState == EGKCombatState::Run)
		{
			SetCombatState(EGKCombatState::Idle);
		}
	}
	else if (CombatState == EGKCombatState::Idle)
	{
		SetCombatState(EGKCombatState::Run);
	}
}

void AGKCharacter::UpdateSprintStaminaDrain(float DeltaSeconds)
{
	if (CombatState != EGKCombatState::Sprint)
	{
		return;
	}

	const UGKCombatConfig* Config = GetCombatConfig();
	CurrentStamina = FMath::Max(0.f, CurrentStamina - Config->Stamina_SprintPerSec * DeltaSeconds);
	StaminaRegenBlockedUntil = GetWorld()->GetTimeSeconds() + Config->StaminaRegenDelay;

	if (CurrentStamina <= 0.f)
	{
		ExitSprintState();
	}
}

void AGKCharacter::UpdateStaminaRegen(float DeltaSeconds)
{
	if (CombatState == EGKCombatState::Death || CombatState == EGKCombatState::Sprint)
	{
		return;
	}

	const UGKCombatConfig* Config = GetCombatConfig();
	if (GetWorld()->GetTimeSeconds() < StaminaRegenBlockedUntil)
	{
		return;
	}

	CurrentStamina = FMath::Min(Config->MaxStamina, CurrentStamina + Config->StaminaRegenPerSec * DeltaSeconds);
}

void AGKCharacter::TryStartAttack()
{
	if (CombatState == EGKCombatState::Sprint)
	{
		ExitSprintState();
	}

	BeginComboAttack(0);
}

void AGKCharacter::TryStartJump()
{
	if (CombatState == EGKCombatState::Sprint)
	{
		ExitSprintState();
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement || !Movement->IsMovingOnGround())
	{
		return;
	}

	Jump();
	SetCombatState(EGKCombatState::Jump);
}

void AGKCharacter::TryStartJumpAttack()
{
	BeginJumpAttack();
}

void AGKCharacter::TryStartEvade()
{
	if (CombatState == EGKCombatState::Sprint)
	{
		ExitSprintState();
	}

	BeginEvadeMotion();
}

void AGKCharacter::TryStartHeal()
{
	if (CombatState == EGKCombatState::Sprint)
	{
		ExitSprintState();
	}

	BeginHealMotion();
}

void AGKCharacter::EnterSprintState()
{
	if (!CanTransitionToSprint())
	{
		return;
	}

	SetCombatState(EGKCombatState::Sprint);
	ApplySprintSpeed();
}

void AGKCharacter::ExitSprintState()
{
	if (CombatState != EGKCombatState::Sprint)
	{
		return;
	}

	ApplyRunSpeed();
	SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
}

void AGKCharacter::ClearMotionTimers()
{
	GetWorldTimerManager().ClearTimer(MotionTimerHandle);
	GetWorldTimerManager().ClearTimer(HitWindowStartTimerHandle);
	GetWorldTimerManager().ClearTimer(HitWindowEndTimerHandle);
	GetWorldTimerManager().ClearTimer(HealDrinkTimerHandle);
	GetWorldTimerManager().ClearTimer(EvadePhaseTimerHandle);
	GetWorldTimerManager().ClearTimer(HitStunTimerHandle);
}

void AGKCharacter::BeginComboAttack(int32 ComboIndex)
{
	const FGKComboAttackRow* Row = GetComboRow(ComboIndex);
	if (!Row || !HasEnoughStamina(Row->StaminaCost))
	{
		SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
		ResetComboState();
		return;
	}

	ClearMotionTimers();
	ResetComboState();

	CurrentComboIndex = ComboIndex;
	const UGKCombatConfig* Config = GetCombatConfig();
	CurrentStamina -= Row->StaminaCost;
	StaminaRegenBlockedUntil = GetWorld()->GetTimeSeconds() + Config->StaminaRegenDelay;

	SetCombatState(EGKCombatState::Attack);
	FaceLockOnTargetIfNeeded();
	BroadcastWeaponSwing(ComboIndex);

	if (Row->Montage)
	{
		PlayAnimMontage(Row->Montage);
	}

	GetWorldTimerManager().SetTimer(
		HitWindowStartTimerHandle,
		this,
		&AGKCharacter::OnAttackHitWindowStart,
		Row->HitWindowStart,
		false);

	GetWorldTimerManager().SetTimer(
		HitWindowEndTimerHandle,
		this,
		&AGKCharacter::OnAttackHitWindowEnd,
		Row->HitWindowEnd,
		false);

	GetWorldTimerManager().SetTimer(
		MotionTimerHandle,
		this,
		&AGKCharacter::FinishAttackMotion,
		Row->MotionDuration,
		false);
}

void AGKCharacter::OnAttackHitWindowStart()
{
	bAttackHitApplied = false;
	ProcessAttackHit();
}

void AGKCharacter::OnAttackHitWindowEnd()
{
	bAttackHitApplied = true;
}

void AGKCharacter::ProcessAttackHit()
{
	if (bAttackHitApplied)
	{
		return;
	}

	const FGKComboAttackRow* Row = GetComboRow(CurrentComboIndex);
	if (!Row)
	{
		return;
	}

	const float TraceDistance = GetCapsuleComponent()->GetScaledCapsuleRadius() * 4.f;
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * TraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GKComboHit), false, this);
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
	{
		return;
	}

	AGKEnemyCharacter* Enemy = Cast<AGKEnemyCharacter>(Hit.GetActor());
	if (!Enemy || !Enemy->IsAlive())
	{
		return;
	}

	bAttackHitApplied = true;
	Enemy->ApplyComboDamage(Row->Damage, CurrentComboIndex, this);
}

void AGKCharacter::FinishAttackMotion()
{
	const FGKComboAttackRow* CurrentRow = GetComboRow(CurrentComboIndex);
	if (!CurrentRow)
	{
		SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
		ResetComboState();
		return;
	}

	if (ComboInputBuffered && CurrentComboIndex < 2)
	{
		const int32 NextIndex = CurrentComboIndex + 1;
		const FGKComboAttackRow* NextRow = GetComboRow(NextIndex);
		if (NextRow && HasEnoughStamina(NextRow->StaminaCost))
		{
			BeginComboAttack(NextIndex);
			return;
		}
	}

	SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
	ResetComboState();
}

void AGKCharacter::BeginJumpAttack()
{
	const UGKCombatConfig* Config = GetCombatConfig();
	if (!HasEnoughStamina(Config->Stamina_JumpAttack))
	{
		return;
	}

	ClearMotionTimers();
	ResetComboState();

	CurrentStamina -= Config->Stamina_JumpAttack;
	StaminaRegenBlockedUntil = GetWorld()->GetTimeSeconds() + Config->StaminaRegenDelay;

	ActiveMotionElapsed = 0.f;
	bJumpAttackHitApplied = false;

	SetCombatState(EGKCombatState::JumpAttack);
	FaceLockOnTargetIfNeeded();
	BroadcastWeaponSwing(JumpAttackAudioComboIndex);

	if (Config->JumpAttackMontage)
	{
		PlayAnimMontage(Config->JumpAttackMontage);
	}

	GetWorldTimerManager().SetTimer(
		HitWindowStartTimerHandle,
		this,
		&AGKCharacter::OnJumpAttackHitWindowStart,
		Config->JumpAttack_HitWindowStart,
		false);

	GetWorldTimerManager().SetTimer(
		HitWindowEndTimerHandle,
		this,
		&AGKCharacter::OnJumpAttackHitWindowEnd,
		Config->JumpAttack_HitWindowEnd,
		false);

	GetWorldTimerManager().SetTimer(
		MotionTimerHandle,
		this,
		&AGKCharacter::FinishJumpAttack,
		Config->JumpAttack_MotionDuration,
		false);
}

void AGKCharacter::OnJumpAttackHitWindowStart()
{
	bJumpAttackHitApplied = false;
	ProcessJumpAttackHit();
}

void AGKCharacter::OnJumpAttackHitWindowEnd()
{
	bJumpAttackHitApplied = true;
}

void AGKCharacter::ProcessJumpAttackHit()
{
	if (bJumpAttackHitApplied || CombatState != EGKCombatState::JumpAttack)
	{
		return;
	}

	const UGKCombatConfig* Config = GetCombatConfig();
	const float TraceDistance = GetCapsuleComponent()->GetScaledCapsuleRadius() * 4.f;
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * TraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GKJumpAttackHit), false, this);
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
	{
		return;
	}

	AGKEnemyCharacter* Enemy = Cast<AGKEnemyCharacter>(Hit.GetActor());
	if (!Enemy || !Enemy->IsAlive())
	{
		return;
	}

	bJumpAttackHitApplied = true;
	Enemy->ApplyComboDamage(Config->JumpAttack_Damage, JumpAttackAudioComboIndex, this);
}

void AGKCharacter::FinishJumpAttack()
{
	if (CombatState != EGKCombatState::JumpAttack)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (Movement->IsFalling())
		{
			FVector Velocity = Movement->Velocity;
			Velocity.Z = FMath::Min(Velocity.Z, 0.f);
			Movement->Velocity = Velocity;
		}
	}

	SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
}

void AGKCharacter::BeginEvadeMotion()
{
	const UGKCombatConfig* Config = GetCombatConfig();
	if (!HasEnoughStamina(Config->Stamina_Evade))
	{
		return;
	}

	ClearMotionTimers();
	ResetComboState();

	CurrentStamina -= Config->Stamina_Evade;
	StaminaRegenBlockedUntil = GetWorld()->GetTimeSeconds() + Config->StaminaRegenDelay;

	bIsInvulnerable = true;
	SetCombatState(EGKCombatState::Evade_Active);
	BroadcastEvadeStart();

	if (Config->EvadeMontage)
	{
		PlayAnimMontage(Config->EvadeMontage);
	}

	const FVector EvadeDirection = GetEvadeDirection();
	const float EvadeDistance = Config->RunSpeed * Config->Evade_IFrameDuration;
	LaunchCharacter(EvadeDirection * EvadeDistance, true, false);

	GetWorldTimerManager().SetTimer(
		EvadePhaseTimerHandle,
		this,
		&AGKCharacter::EnterEvadeRecoveryPhase,
		Config->Evade_IFrameDuration,
		false);
}

void AGKCharacter::EnterEvadeRecoveryPhase()
{
	bIsInvulnerable = false;
	SetCombatState(EGKCombatState::Evade_Recovery);

	const UGKCombatConfig* Config = GetCombatConfig();
	const float RecoveryDuration = Config->Evade_TotalDuration - Config->Evade_IFrameDuration;
	GetWorldTimerManager().SetTimer(
		EvadePhaseTimerHandle,
		this,
		&AGKCharacter::FinishEvadeMotion,
		RecoveryDuration,
		false);
}

void AGKCharacter::FinishEvadeMotion()
{
	BroadcastEvadeEnd();
	SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
}

FVector AGKCharacter::GetEvadeDirection() const
{
	if (!MoveInput.IsNearlyZero() && Controller)
	{
		const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
		FVector Direction = Forward * MoveInput.Y + Right * MoveInput.X;

		if (bLockOnActive && LockOnTarget.IsValid())
		{
			const FVector ToTarget = LockOnTarget->GetActorLocation() - GetActorLocation();
			const FVector ToTargetFlat = FVector(ToTarget.X, ToTarget.Y, 0.f).GetSafeNormal();
			const FVector RightFlat = FVector::CrossProduct(FVector::UpVector, ToTargetFlat).GetSafeNormal();
			const float ForwardDot = FVector::DotProduct(Direction, ToTargetFlat);
			const float RightDot = FVector::DotProduct(Direction, RightFlat);

			if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
			{
				Direction = ForwardDot >= 0.f ? ToTargetFlat : -ToTargetFlat;
			}
			else
			{
				Direction = RightDot >= 0.f ? RightFlat : -RightFlat;
			}
		}

		return Direction.GetSafeNormal();
	}

	return -GetActorForwardVector();
}

void AGKCharacter::BeginHealMotion()
{
	const UGKPlayerStatsConfig* Stats = GetPlayerStatsConfig();
	if (HealItemRemaining <= 0)
	{
		return;
	}

	ClearMotionTimers();
	ResetComboState();

	HealItemRemaining -= 1;
	bHealDrinkApplied = false;
	bHealCompleted = false;

	SetCombatState(EGKCombatState::Heal);
	BroadcastHealItemStart();

	if (Stats->HealItemMontage)
	{
		PlayAnimMontage(Stats->HealItemMontage);
	}

	GetWorldTimerManager().SetTimer(
		HealDrinkTimerHandle,
		this,
		&AGKCharacter::ApplyHealDrinkEffect,
		Stats->HealItem_DrinkTime,
		false);

	GetWorldTimerManager().SetTimer(
		MotionTimerHandle,
		this,
		&AGKCharacter::FinishHealMotion,
		Stats->HealItem_MotionDuration,
		false);
}

void AGKCharacter::ApplyHealDrinkEffect()
{
	if (bHealDrinkApplied || CombatState != EGKCombatState::Heal)
	{
		return;
	}

	const UGKPlayerStatsConfig* Stats = GetPlayerStatsConfig();
	bHealDrinkApplied = true;
	CurrentHP = FMath::Min(Stats->MaxHP, CurrentHP + Stats->HealItem_HealAmount);
	BroadcastHealItemDrink();
}

void AGKCharacter::FinishHealMotion()
{
	if (CombatState != EGKCombatState::Heal)
	{
		return;
	}

	if (bHealDrinkApplied)
	{
		BroadcastHealItemComplete();
	}

	SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
}

void AGKCharacter::EnterHitStun(AActor* Attacker, const FVector& HitLocation)
{
	if (CombatState == EGKCombatState::Death || CombatState == EGKCombatState::Evade_Active || bIsInvulnerable)
	{
		return;
	}

	ClearMotionTimers();

	if (CombatState == EGKCombatState::Sprint)
	{
		ExitSprintState();
	}

	ResetComboState();
	SetCombatState(EGKCombatState::HitStun);
	BroadcastHitDamage(HitLocation, Attacker);

	const UGKCombatConfig* Config = GetCombatConfig();
	const float HitStunDuration = Config->Evade_TotalDuration - Config->Evade_IFrameDuration;
	GetWorldTimerManager().SetTimer(
		HitStunTimerHandle,
		this,
		&AGKCharacter::FinishHitStun,
		HitStunDuration,
		false);
}

void AGKCharacter::FinishHitStun()
{
	if (CombatState == EGKCombatState::HitStun)
	{
		SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
	}
}

void AGKCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (CombatState == EGKCombatState::Jump)
	{
		SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
	}
}

float AGKCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (CombatState == EGKCombatState::Death || DamageAmount <= 0.f)
	{
		return 0.f;
	}

	CurrentHP = FMath::Max(0.f, CurrentHP - DamageAmount);

	if (CurrentHP <= 0.f)
	{
		EnterDeath();
		return DamageAmount;
	}

	EnterHitStun(DamageCauser, GetActorLocation());
	return DamageAmount;
}

void AGKCharacter::EnterDeath()
{
	ClearMotionTimers();
	ClearLockOn();
	SetCombatState(EGKCombatState::Death);
	GKCharacterLog::DebugPrint(this, TEXT("YOU DIED"), FColor::Red);
}

void AGKCharacter::FaceLockOnTargetIfNeeded()
{
	if (!bLockOnActive || !LockOnTarget.IsValid())
	{
		return;
	}

	const FVector ToTarget = LockOnTarget->GetActorLocation() - GetActorLocation();
	SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));
}

void AGKCharacter::BroadcastWeaponSwing(int32 ComboIndex)
{
	GKCharacterLog::DebugPrint(this, FString::Printf(TEXT("[AudioHook] OnWeaponSwing(%d)"), ComboIndex));
	OnWeaponSwing(ComboIndex);
}

void AGKCharacter::BroadcastEvadeStart()
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnEvadeStart()"));
	OnEvadeStart();
}

void AGKCharacter::BroadcastEvadeEnd()
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnEvadeEnd()"));
	OnEvadeEnd();
}

void AGKCharacter::BroadcastHealItemStart()
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnHealItemStart()"));
	OnHealItemStart();
}

void AGKCharacter::BroadcastHealItemDrink()
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnHealItemDrink()"));
	OnHealItemDrink();
}

void AGKCharacter::BroadcastHealItemComplete()
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnHealItemComplete()"));
	OnHealItemComplete();
}

void AGKCharacter::BroadcastHitDamage(const FVector& HitLocation, AActor* Attacker)
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnHitDamage()"));
	OnHitDamage(HitLocation, Attacker);
}

void AGKCharacter::ToggleLockOn()
{
	if (bLockOnActive)
	{
		ClearLockOn();
	}
	else
	{
		AcquireLockOnTarget();
	}
}

void AGKCharacter::AcquireLockOnTarget()
{
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGKEnemyCharacter::StaticClass(), FoundEnemies);

	AGKEnemyCharacter* BestTarget = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (AActor* Actor : FoundEnemies)
	{
		AGKEnemyCharacter* Enemy = Cast<AGKEnemyCharacter>(Actor);
		if (!IsEnemyValidForLockOn(Enemy))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Enemy->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestTarget = Enemy;
		}
	}

	if (BestTarget)
	{
		LockOnTarget = BestTarget;
		bLockOnActive = true;
		GKCharacterLog::DebugPrint(this, FString::Printf(TEXT("LockOn ON -> %s"), *BestTarget->GetName()), FColor::Magenta);
	}
	else
	{
		GKCharacterLog::DebugPrint(this, TEXT("LockOn: no valid target"), FColor::Silver);
	}
}

void AGKCharacter::ClearLockOn()
{
	bLockOnActive = false;
	LockOnTarget = nullptr;
	GKCharacterLog::DebugPrint(this, TEXT("LockOn OFF"), FColor::Magenta);
}

bool AGKCharacter::IsEnemyValidForLockOn(const AGKEnemyCharacter* Enemy) const
{
	if (!Enemy || !Enemy->IsAlive())
	{
		return false;
	}

	const UGKCombatConfig* Config = GetCombatConfig();
	if (FVector::Dist(GetActorLocation(), Enemy->GetActorLocation()) > Config->LockOnMaxDistance)
	{
		return false;
	}

	if (!FollowCamera)
	{
		return true;
	}

	const FVector CameraForward = FollowCamera->GetForwardVector();
	const FVector ToEnemy = (Enemy->GetActorLocation() - FollowCamera->GetComponentLocation()).GetSafeNormal();
	const float MinDot = FMath::Cos(FMath::DegreesToRadians(Config->LockOnFOVDegrees));
	return FVector::DotProduct(CameraForward, ToEnemy) >= MinDot;
}

void AGKCharacter::UpdateLockOn(float DeltaSeconds)
{
	if (bLockOnActive && (!LockOnTarget.IsValid() || !IsEnemyValidForLockOn(LockOnTarget.Get())))
	{
		ClearLockOn();
	}
}

void AGKCharacter::ApplyCharacterTuning()
{
	const UGKCombatConfig* Config = GetCombatConfig();

	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = !bLockOnActive;
	Movement->RotationRate = FRotator(0.f, RotationRate, 0.f);
	Movement->MinAnalogWalkSpeed = MinAnalogWalkSpeed;
	Movement->BrakingDecelerationWalking = BrakingDecelerationWalking;
	Movement->BrakingDecelerationFalling = BrakingDecelerationFalling;
	Movement->JumpZVelocity = Config->Jump_ZVelocity;
	Movement->AirControl = Config->Jump_AirControl;

	Movement->MaxWalkSpeed = (CombatState == EGKCombatState::Sprint) ? Config->SprintSpeed : Config->RunSpeed;
	CameraBoom->TargetArmLength = CameraArmLength;
}
