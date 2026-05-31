// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKCharacter.h"
#include "GKEnemyCharacter.h"
#include "GKAnimMontageTypes.h"
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
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
static TAutoConsoleVariable<int32> CVarParryDebugForceSuccess(
	TEXT("gk.ParryDebugForceSuccess"),
	0,
	TEXT("Dev only: 1 = force parry success on nearest in-range enemy during Parry_Active"),
	ECVF_Cheat);
#endif

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT || WITH_EDITOR
static TAutoConsoleVariable<int32> CVarAnimTableStrictMode(
	TEXT("gk.AnimTableStrictMode"),
	0,
	TEXT("Dev/Editor: 1 = reject deprecated montage fallback; DT row must resolve"),
	ECVF_Cheat);
#endif

namespace GKAnimTableLog
{
	static TMap<FName, int32> GFallbackCounts;

	static bool IsStrictMode()
	{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT || WITH_EDITOR
		return CVarAnimTableStrictMode.GetValueOnGameThread() != 0;
#else
		return false;
#endif
	}

	static FName GetActionLogKey(EGKAnimAction Action, int32 Variant)
	{
		return FGKAnimMontageTableHelpers::BuildPCAnimRowName(Action, Variant);
	}

	static FString GetActionDisplayName(EGKAnimAction Action, int32 Variant)
	{
		const FName RowName = GetActionLogKey(Action, Variant);
		return RowName.IsNone() ? UEnum::GetValueAsString(Action) : RowName.ToString();
	}

	static void RecordFallback(EGKAnimAction Action, int32 Variant)
	{
		const FName Key = GetActionLogKey(Action, Variant);
		if (!Key.IsNone())
		{
			GFallbackCounts.FindOrAdd(Key)++;
		}
	}

	static int32 GetTotalFallbackCount()
	{
		int32 Total = 0;
		for (const TPair<FName, int32>& Pair : GFallbackCounts)
		{
			Total += Pair.Value;
		}
		return Total;
	}

	static void LogSessionSummary()
	{
		const int32 Total = GetTotalFallbackCount();
		UE_LOG(LogTemp, Warning, TEXT("[AnimTable] SessionFallbackSummary Total=%d"), Total);
		for (const TPair<FName, int32>& Pair : GFallbackCounts)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AnimTable] SessionFallbackSummary RowName=%s Count=%d"),
				*Pair.Key.ToString(), Pair.Value);
		}
	}

	static void ResetSession()
	{
		GFallbackCounts.Empty();
	}

	static void LogStrictReject(EGKAnimAction Action, int32 Variant, const FString& Reason)
	{
		const FString ActionName = GetActionDisplayName(Action, Variant);
		const FName RowName = GetActionLogKey(Action, Variant);
		UE_LOG(LogTemp, Warning,
			TEXT("[AnimTable] StrictReject Action=%s RowName=%s Reason=%s"),
			*ActionName,
			*RowName.ToString(),
			*Reason);
		ensureMsgf(false,
			TEXT("[AnimTable] StrictReject Action=%s RowName=%s Reason=%s"),
			*ActionName,
			*RowName.ToString(),
			*Reason);
	}

	static void LogFallbackUsed(
		EGKAnimAction Action,
		int32 Variant,
		const FString& Reason,
		const FString& DeprecatedSlotName,
		UAnimMontage* Montage)
	{
		RecordFallback(Action, Variant);
		const FString ActionName = GetActionDisplayName(Action, Variant);
		const FName RowName = GetActionLogKey(Action, Variant);
		UE_LOG(LogTemp, Warning,
			TEXT("[AnimTable] FallbackUsed Action=%s RowName=%s Reason=%s DeprecatedSlot=%s Montage=%s"),
			*ActionName,
			*RowName.ToString(),
			*Reason,
			*DeprecatedSlotName,
			*GetNameSafe(Montage));
	}

	static void LogRowHit(EGKAnimAction Action, int32 Variant, UAnimMontage* Montage)
	{
		const FString ActionName = GetActionDisplayName(Action, Variant);
		const FName RowName = GetActionLogKey(Action, Variant);
		UE_LOG(LogTemp, Log,
			TEXT("[AnimTable] RowHit Action=%s RowName=%s Montage=%s"),
			*ActionName,
			*RowName.ToString(),
			*GetNameSafe(Montage));
	}
}

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

UDataTable* AGKCharacter::GetAnimMontageTable()
{
	if (AnimMontageTable)
	{
		return AnimMontageTable;
	}

	if (!RuntimeAnimMontageTable)
	{
		RuntimeAnimMontageTable = FGKAnimMontageTableHelpers::CreateDefaultAnimMontageTable(this);
	}

	return RuntimeAnimMontageTable;
}

const FGKAnimMontageRow* AGKCharacter::GetRowForAction(EGKAnimAction Action, int32 Variant) const
{
	const UDataTable* Table = AnimMontageTable
		? AnimMontageTable.Get()
		: RuntimeAnimMontageTable.Get();
	return FGKAnimMontageTableHelpers::FindPCRow(Table, Action, Variant);
}

UAnimMontage* AGKCharacter::GetMontageForAction(EGKAnimAction Action, int32 Variant) const
{
	const FGKAnimMontageRow* Row = GetRowForAction(Action, Variant);
	if (Row)
	{
		if (UAnimMontage* Montage = FGKAnimMontageTableHelpers::ResolveMontage(Row))
		{
			return Montage;
		}

		if (GKAnimTableLog::IsStrictMode())
		{
			const FString Reason = Row->Montage.IsNull() ? TEXT("Soft 비어있음") : TEXT("로드 실패");
			GKAnimTableLog::LogStrictReject(Action, Variant, Reason);
			return nullptr;
		}
	}
	else if (GKAnimTableLog::IsStrictMode())
	{
		GKAnimTableLog::LogStrictReject(Action, Variant, TEXT("Row 없음"));
		return nullptr;
	}

	FString DeprecatedSlotName;
	if (UAnimMontage* Montage = GetDeprecatedMontageFallback(Action, Variant, DeprecatedSlotName))
	{
		const FString Reason = Row ? (Row->Montage.IsNull() ? TEXT("Soft 비어있음") : TEXT("로드 실패"))
		                           : TEXT("Row 없음");
		GKAnimTableLog::LogFallbackUsed(Action, Variant, Reason, DeprecatedSlotName, Montage);
		return Montage;
	}

	return nullptr;
}

UAnimMontage* AGKCharacter::GetDeprecatedMontageFallback(
	EGKAnimAction Action, int32 Variant, FString& OutDeprecatedSlotName) const
{
	OutDeprecatedSlotName.Reset();

	const UGKCombatConfig* Config = GetCombatConfig();
	const UGKPlayerStatsConfig* Stats = GetPlayerStatsConfig();

	switch (Action)
	{
	case EGKAnimAction::Attack_Combo:
		OutDeprecatedSlotName = FString::Printf(TEXT("FGKComboAttackRow.Montage(Combo_%02d)"), Variant + 1);
		if (const FGKComboAttackRow* Row = GetComboRow(Variant))
		{
			return Row->Montage;
		}
		break;
	case EGKAnimAction::Evade:
		OutDeprecatedSlotName = TEXT("UGKCombatConfig.EvadeMontage");
		return Config ? Config->EvadeMontage : nullptr;
	case EGKAnimAction::Heal:
		OutDeprecatedSlotName = TEXT("UGKPlayerStatsConfig.HealItemMontage");
		return Stats ? Stats->HealItemMontage : nullptr;
	case EGKAnimAction::HeavyAttack:
		OutDeprecatedSlotName = TEXT("UGKCombatConfig.HeavyAttackMontage");
		return Config ? Config->HeavyAttackMontage : nullptr;
	case EGKAnimAction::Parry_Active:
		OutDeprecatedSlotName = TEXT("UGKCombatConfig.ParryMontage");
		return Config ? Config->ParryMontage : nullptr;
	case EGKAnimAction::Parry_Recovery:
		OutDeprecatedSlotName = TEXT("UGKCombatConfig.ParryMontage");
		return Config ? Config->ParryMontage : nullptr;
	default:
		OutDeprecatedSlotName = TEXT("None");
		break;
	}

	return nullptr;
}

bool AGKCharacter::TryPlayActionMontage(EGKAnimAction Action, int32 Variant)
{
	const FGKAnimMontageRow* Row = GetRowForAction(Action, Variant);
	float PlayRate = Row ? Row->PlayRate : 1.f;

	if (Row)
	{
		if (UAnimMontage* Montage = FGKAnimMontageTableHelpers::ResolveMontage(Row))
		{
			GKAnimTableLog::LogRowHit(Action, Variant, Montage);
			PlayAnimMontage(Montage, PlayRate);
			return true;
		}

		if (GKAnimTableLog::IsStrictMode())
		{
			const FString Reason = Row->Montage.IsNull() ? TEXT("Soft 비어있음") : TEXT("로드 실패");
			GKAnimTableLog::LogStrictReject(Action, Variant, Reason);
			return false;
		}
	}
	else if (GKAnimTableLog::IsStrictMode())
	{
		GKAnimTableLog::LogStrictReject(Action, Variant, TEXT("Row 없음"));
		return false;
	}

	FString DeprecatedSlotName;
	UAnimMontage* Montage = GetDeprecatedMontageFallback(Action, Variant, DeprecatedSlotName);
	if (!Montage)
	{
		return false;
	}

	const FString Reason = Row ? (Row->Montage.IsNull() ? TEXT("Soft 비어있음") : TEXT("로드 실패"))
	                           : TEXT("Row 없음");
	GKAnimTableLog::LogFallbackUsed(Action, Variant, Reason, DeprecatedSlotName, Montage);
	PlayAnimMontage(Montage, PlayRate);
	return true;
}

void AGKCharacter::PreloadCriticalMontages()
{
	auto PreloadOne = [this](EGKAnimAction Action, int32 Variant)
	{
		const FName RowName = FGKAnimMontageTableHelpers::BuildPCAnimRowName(Action, Variant);
		if (const FGKAnimMontageRow* Row = GetRowForAction(Action, Variant))
		{
			if (UAnimMontage* Montage = FGKAnimMontageTableHelpers::ResolveMontage(Row))
			{
				UE_LOG(LogTemp, Log, TEXT("[AnimTable] Preload RowName=%s Montage=%s"),
					*RowName.ToString(), *GetNameSafe(Montage));
			}
		}
	};

	PreloadOne(EGKAnimAction::Evade, 0);
	PreloadOne(EGKAnimAction::Heal, 0);
	PreloadOne(EGKAnimAction::HeavyAttack, 0);
	PreloadOne(EGKAnimAction::Parry_Active, 0);
	PreloadOne(EGKAnimAction::Parry_Recovery, 0);

	for (int32 ComboIndex = 0; ComboIndex < 3; ++ComboIndex)
	{
		PreloadOne(EGKAnimAction::Attack_Combo, ComboIndex);
	}
}

void AGKCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocallyControlled())
	{
		GKAnimTableLog::LogSessionSummary();
		GKAnimTableLog::ResetSession();
	}

	Super::EndPlay(EndPlayReason);
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

	if (!AnimMontageTable)
	{
		GetAnimMontageTable();
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

	PreloadCriticalMontages();

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

	if (CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::JumpAttack
		|| CombatState == EGKCombatState::HeavyAttack)
	{
		ActiveMotionElapsed += DeltaSeconds;
	}

	if (CombatState == EGKCombatState::Parry_Active)
	{
		TickParryWindow();
	}

	UpdateLockOn(DeltaSeconds);
	UpdateSprintStaminaDrain(DeltaSeconds);
	UpdateStaminaRegen(DeltaSeconds);
	UpdateLocomotionStateFromInput();
}

void AGKCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	const UWorld* World = GetWorld();
	const float TimeSeconds = World ? World->GetTimeSeconds() : 0.f;
	UE_LOG(LogTemp, Log,
		TEXT("[GK|Possession] PossessedBy Controller=%s Pawn=%s Class=%s Time=%.3f"),
		*GetNameSafe(NewController),
		*GetNameSafe(this),
		*GetClass()->GetName(),
		TimeSeconds);

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

	if (HeavyAttackAction)
	{
		EIC->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AGKCharacter::HandleHeavyAttackStarted);
	}

	if (ParryAction)
	{
		EIC->BindAction(ParryAction, ETriggerEvent::Started, this, &AGKCharacter::HandleParryStarted);
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

	if (IsLocomotionBlockedState())
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
	if (IsFullyBlockedCombatState())
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

	if (IsFullyBlockedCombatState())
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

void AGKCharacter::HandleHeavyAttackStarted(const FInputActionValue& /*Value*/)
{
	if (CanTransitionToHeavyAttack())
	{
		TryStartHeavyAttack();
	}
}

void AGKCharacter::HandleParryStarted(const FInputActionValue& /*Value*/)
{
	if (CanTransitionToParry())
	{
		TryStartParry();
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
	if (IsFullyBlockedCombatState())
	{
		return false;
	}

	return CombatState != EGKCombatState::Evade_Active && CombatState != EGKCombatState::Evade_Recovery
		&& CombatState != EGKCombatState::Jump && CombatState != EGKCombatState::JumpAttack
		&& CombatState != EGKCombatState::Parry_Active && CombatState != EGKCombatState::Parry_Recovery;
}

bool AGKCharacter::IsGroundLocomotionState() const
{
	return CombatState == EGKCombatState::Idle || CombatState == EGKCombatState::Run
		|| CombatState == EGKCombatState::Sprint;
}

bool AGKCharacter::IsAirborneCombatState() const
{
	return CombatState == EGKCombatState::Jump || CombatState == EGKCombatState::JumpAttack;
}

bool AGKCharacter::IsParryMotionState() const
{
	return CombatState == EGKCombatState::Parry_Active || CombatState == EGKCombatState::Parry_Recovery;
}

bool AGKCharacter::CanTransitionToEvade() const
{
	if (IsFullyBlockedCombatState() || CombatState == EGKCombatState::Evade_Active
		|| CombatState == EGKCombatState::Evade_Recovery || CombatState == EGKCombatState::Jump
		|| CombatState == EGKCombatState::JumpAttack || CombatState == EGKCombatState::Parry_Active)
	{
		return false;
	}

	if (CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::HeavyAttack
		|| CombatState == EGKCombatState::Heal || CombatState == EGKCombatState::Parry_Recovery
		|| IsGroundLocomotionState())
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
		|| CombatState == EGKCombatState::Evade_Active || CombatState == EGKCombatState::Evade_Recovery
		|| CombatState == EGKCombatState::HeavyAttack || IsParryMotionState())
	{
		return false;
	}

	return CurrentStamina > 0.f;
}

bool AGKCharacter::CanTransitionToJump() const
{
	if (IsFullyBlockedCombatState())
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

	if (CombatState == EGKCombatState::Attack || IsFullyBlockedCombatState()
		|| CombatState == EGKCombatState::Evade_Active || CombatState == EGKCombatState::Evade_Recovery
		|| CombatState == EGKCombatState::Heal || CombatState == EGKCombatState::HeavyAttack
		|| IsParryMotionState())
	{
		return false;
	}

	const FGKComboAttackRow* Row = GetComboRow(0);
	return Row && HasEnoughStamina(Row->StaminaCost);
}

bool AGKCharacter::CanTransitionToHeavyAttack() const
{
	if (!bCanUseHeavyAttack || IsAirborneCombatState())
	{
		return false;
	}

	if (IsFullyBlockedCombatState() || CombatState == EGKCombatState::Attack
		|| CombatState == EGKCombatState::HeavyAttack || CombatState == EGKCombatState::Evade_Active
		|| CombatState == EGKCombatState::Evade_Recovery || CombatState == EGKCombatState::Heal
		|| IsParryMotionState())
	{
		return false;
	}

	return IsGroundLocomotionState() && HasEnoughStamina(GetCombatConfig()->Stamina_HeavyAttack);
}

bool AGKCharacter::CanTransitionToParry() const
{
	if (!bCanUseParry || IsAirborneCombatState())
	{
		return false;
	}

	if (IsFullyBlockedCombatState() || CombatState == EGKCombatState::Attack
		|| CombatState == EGKCombatState::HeavyAttack || CombatState == EGKCombatState::Evade_Active
		|| CombatState == EGKCombatState::Evade_Recovery || CombatState == EGKCombatState::Heal
		|| IsParryMotionState())
	{
		return false;
	}

	return IsGroundLocomotionState() && HasEnoughStamina(GetCombatConfig()->Stamina_Parry);
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

bool AGKCharacter::IsFullyBlockedCombatState() const
{
	return CombatState == EGKCombatState::Death || CombatState == EGKCombatState::HitStun;
}

bool AGKCharacter::IsLocomotionBlockedState() const
{
	return CombatState == EGKCombatState::Attack || CombatState == EGKCombatState::Evade_Active
		|| CombatState == EGKCombatState::Evade_Recovery || CombatState == EGKCombatState::Heal
		|| CombatState == EGKCombatState::HitStun || CombatState == EGKCombatState::Death
		|| CombatState == EGKCombatState::Jump || CombatState == EGKCombatState::JumpAttack
		|| CombatState == EGKCombatState::HeavyAttack || IsParryMotionState();
}

bool AGKCharacter::PrepareWeaponAction(float StaminaCost)
{
	if (!HasEnoughStamina(StaminaCost))
	{
		return false;
	}

	ClearMotionTimers();
	ResetComboState();

	CurrentStamina -= StaminaCost;
	StaminaRegenBlockedUntil = GetWorld()->GetTimeSeconds() + GetCombatConfig()->StaminaRegenDelay;
	ActiveMotionElapsed = 0.f;
	return true;
}

AGKEnemyCharacter* AGKCharacter::ExecuteMeleeLineTrace(FName StatTag) const
{
	const float TraceDistance = GetCapsuleComponent()->GetScaledCapsuleRadius() * 4.f;
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * TraceDistance;

	FCollisionQueryParams Params(StatTag, false, this);
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
	{
		return nullptr;
	}

	AGKEnemyCharacter* Enemy = Cast<AGKEnemyCharacter>(Hit.GetActor());
	if (!Enemy || !Enemy->IsAlive())
	{
		return nullptr;
	}

	return Enemy;
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

	if ((NewState == EGKCombatState::HeavyAttack || NewState == EGKCombatState::Parry_Active)
		&& CombatState != NewState)
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
	if (IsLocomotionBlockedState())
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

void AGKCharacter::TryStartHeavyAttack()
{
	if (CombatState == EGKCombatState::Sprint)
	{
		ExitSprintState();
	}

	BeginHeavyAttack();
}

void AGKCharacter::TryStartParry()
{
	if (CombatState == EGKCombatState::Sprint)
	{
		ExitSprintState();
	}

	BeginParry();
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
	GetWorldTimerManager().ClearTimer(ParryActiveTimerHandle);
	GetWorldTimerManager().ClearTimer(ParryRecoveryTimerHandle);
}

void AGKCharacter::BeginComboAttack(int32 ComboIndex)
{
	const FGKComboAttackRow* Row = GetComboRow(ComboIndex);
	if (!Row || !PrepareWeaponAction(Row->StaminaCost))
	{
		SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
		ResetComboState();
		return;
	}

	CurrentComboIndex = ComboIndex;

	SetCombatState(EGKCombatState::Attack);
	FaceLockOnTargetIfNeeded();
	BroadcastWeaponSwing(ComboIndex);
	TryPlayActionMontage(EGKAnimAction::Attack_Combo, ComboIndex);

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

	AGKEnemyCharacter* Enemy = ExecuteMeleeLineTrace(TEXT("GKComboHit"));
	if (!Enemy)
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
	if (!PrepareWeaponAction(Config->Stamina_JumpAttack))
	{
		return;
	}

	bJumpAttackHitApplied = false;

	SetCombatState(EGKCombatState::JumpAttack);
	FaceLockOnTargetIfNeeded();
	BroadcastWeaponSwing(JumpAttackAudioComboIndex);

	TryPlayActionMontage(EGKAnimAction::JumpAttack);

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
	AGKEnemyCharacter* Enemy = ExecuteMeleeLineTrace(TEXT("GKJumpAttackHit"));
	if (!Enemy)
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

void AGKCharacter::SetUnlockHeavyAttack(bool bUnlocked)
{
	bCanUseHeavyAttack = bUnlocked;
}

void AGKCharacter::SetUnlockParry(bool bUnlocked)
{
	bCanUseParry = bUnlocked;
}

void AGKCharacter::BeginHeavyAttack()
{
	const UGKCombatConfig* Config = GetCombatConfig();
	if (!PrepareWeaponAction(Config->Stamina_HeavyAttack))
	{
		return;
	}

	bHeavyAttackHitApplied = false;

	SetCombatState(EGKCombatState::HeavyAttack);
	FaceLockOnTargetIfNeeded();
	BroadcastWeaponSwing(HeavyAttackAudioComboIndex);
	TryPlayActionMontage(EGKAnimAction::HeavyAttack);

	GetWorldTimerManager().SetTimer(
		HitWindowStartTimerHandle,
		this,
		&AGKCharacter::OnHeavyAttackHitWindowStart,
		Config->HeavyAttack_HitWindowStart,
		false);

	GetWorldTimerManager().SetTimer(
		HitWindowEndTimerHandle,
		this,
		&AGKCharacter::OnHeavyAttackHitWindowEnd,
		Config->HeavyAttack_HitWindowEnd,
		false);

	GetWorldTimerManager().SetTimer(
		MotionTimerHandle,
		this,
		&AGKCharacter::FinishHeavyAttack,
		Config->HeavyAttack_MotionDuration,
		false);
}

void AGKCharacter::OnHeavyAttackHitWindowStart()
{
	bHeavyAttackHitApplied = false;
	ProcessHeavyAttackHit();
}

void AGKCharacter::OnHeavyAttackHitWindowEnd()
{
	bHeavyAttackHitApplied = true;
}

void AGKCharacter::ProcessHeavyAttackHit()
{
	if (bHeavyAttackHitApplied || CombatState != EGKCombatState::HeavyAttack)
	{
		return;
	}

	const UGKCombatConfig* Config = GetCombatConfig();
	AGKEnemyCharacter* Enemy = ExecuteMeleeLineTrace(TEXT("GKHeavyAttackHit"));
	if (!Enemy)
	{
		return;
	}

	bHeavyAttackHitApplied = true;
	Enemy->ApplyHeavyAttackDamage(Config->HeavyAttack_Damage, 0.f, this);
}

void AGKCharacter::FinishHeavyAttack()
{
	if (CombatState != EGKCombatState::HeavyAttack)
	{
		return;
	}

	SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
}

void AGKCharacter::BeginParry()
{
	const UGKCombatConfig* Config = GetCombatConfig();
	if (!PrepareWeaponAction(Config->Stamina_Parry))
	{
		return;
	}

	bParrySucceeded = false;

	SetCombatState(EGKCombatState::Parry_Active);
	FaceLockOnTargetIfNeeded();
	BroadcastParryAttempt();
	TryPlayActionMontage(EGKAnimAction::Parry_Active);

	GetWorldTimerManager().SetTimer(
		ParryActiveTimerHandle,
		this,
		&AGKCharacter::EndParryActivePhase,
		Config->Parry_ActiveDuration,
		false);
}

void AGKCharacter::CompleteParrySuccess(AGKEnemyCharacter* Enemy)
{
	if (!Enemy || bParrySucceeded || CombatState != EGKCombatState::Parry_Active)
	{
		return;
	}

	const UGKCombatConfig* Config = GetCombatConfig();
	bParrySucceeded = true;
	GetWorldTimerManager().ClearTimer(ParryActiveTimerHandle);
	Enemy->ApplyParrySuccess(Config->Parry_RipostWindow, this);
	BroadcastParrySuccess();
	// 의도 동작(SKILL_01B A-6/S11): 패링 성공 시 Parry_Recovery 없이 즉시 Idle/Run 복귀. Recovery는 실패 경로 전용.
	SetCombatState(MoveInput.IsNearlyZero() ? EGKCombatState::Idle : EGKCombatState::Run);
}

void AGKCharacter::TickParryWindow()
{
	if (CombatState != EGKCombatState::Parry_Active || bParrySucceeded)
	{
		return;
	}

	const UGKCombatConfig* Config = GetCombatConfig();

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	if (CVarParryDebugForceSuccess.GetValueOnGameThread() != 0)
	{
		TArray<AActor*> DebugEnemies;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGKEnemyCharacter::StaticClass(), DebugEnemies);
		for (AActor* Actor : DebugEnemies)
		{
			AGKEnemyCharacter* Enemy = Cast<AGKEnemyCharacter>(Actor);
			if (!Enemy || !Enemy->IsAlive())
			{
				continue;
			}

			if (FVector::Dist(GetActorLocation(), Enemy->GetActorLocation()) <= Config->LockOnMaxDistance)
			{
				CompleteParrySuccess(Enemy);
				return;
			}
		}
	}
#endif

	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGKEnemyCharacter::StaticClass(), FoundEnemies);

	for (AActor* Actor : FoundEnemies)
	{
		AGKEnemyCharacter* Enemy = Cast<AGKEnemyCharacter>(Actor);
		if (!Enemy || !Enemy->IsAlive() || !Enemy->IsAttackHitWindowActive())
		{
			continue;
		}

		if (FVector::Dist(GetActorLocation(), Enemy->GetActorLocation()) > Config->LockOnMaxDistance)
		{
			continue;
		}

		CompleteParrySuccess(Enemy);
		return;
	}
}

void AGKCharacter::EndParryActivePhase()
{
	if (CombatState != EGKCombatState::Parry_Active || bParrySucceeded)
	{
		return;
	}

	EnterParryRecoveryPhase();
}

void AGKCharacter::EnterParryRecoveryPhase()
{
	if (CombatState != EGKCombatState::Parry_Active)
	{
		return;
	}

	BroadcastParryFail();

	const UGKCombatConfig* Config = GetCombatConfig();
	SetCombatState(EGKCombatState::Parry_Recovery);
	TryPlayActionMontage(EGKAnimAction::Parry_Recovery);

	GetWorldTimerManager().SetTimer(
		ParryRecoveryTimerHandle,
		this,
		&AGKCharacter::FinishParryMotion,
		Config->Parry_RecoveryDuration,
		false);
}

void AGKCharacter::FinishParryMotion()
{
	if (CombatState != EGKCombatState::Parry_Recovery)
	{
		return;
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
	TryPlayActionMontage(EGKAnimAction::Evade);

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
	TryPlayActionMontage(EGKAnimAction::Heal);

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
	GetWorldTimerManager().SetTimer(
		HitStunTimerHandle,
		this,
		&AGKCharacter::FinishHitStun,
		Config->HitStun_Duration,
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

void AGKCharacter::BroadcastParryAttempt()
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnParryAttempt()"));
	OnParryAttempt();
}

void AGKCharacter::BroadcastParrySuccess()
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnParrySuccess()"));
	OnParrySuccess();
}

void AGKCharacter::BroadcastParryFail()
{
	GKCharacterLog::DebugPrint(this, TEXT("[AudioHook] OnParryFail()"));
	OnParryFail();
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
