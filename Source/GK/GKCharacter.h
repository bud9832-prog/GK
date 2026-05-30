// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GKCombatConfig.h"
#include "GKPlayerStatsConfig.h"
#include "GKCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAkComponent;
class UInputMappingContext;
class UInputAction;
class AGKEnemyCharacter;

UENUM(BlueprintType)
enum class EGKCombatState : uint8
{
	Idle            UMETA(DisplayName = "Idle"),
	Run             UMETA(DisplayName = "Run"),
	Sprint          UMETA(DisplayName = "Sprint"),
	Attack          UMETA(DisplayName = "Attack"),
	Evade_Active    UMETA(DisplayName = "Evade Active"),
	Evade_Recovery  UMETA(DisplayName = "Evade Recovery"),
	Heal            UMETA(DisplayName = "Heal"),
	Jump            UMETA(DisplayName = "Jump"),
	JumpAttack      UMETA(DisplayName = "Jump Attack"),
	HeavyAttack     UMETA(DisplayName = "Heavy Attack"),
	Parry_Active    UMETA(DisplayName = "Parry Active"),
	Parry_Recovery  UMETA(DisplayName = "Parry Recovery"),
	HitStun         UMETA(DisplayName = "Hit Stun"),
	Death           UMETA(DisplayName = "Death"),
};

UCLASS()
class GK_API AGKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGKCharacter();

	// --- Audio Hooks (BlueprintImplementableEvent — Wwise mapping by KiHoon) ---

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Movement")
	void OnFootstep(EPhysicalSurface SurfaceType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnWeaponSwing(int32 ComboIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnEvadeStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnEvadeEnd();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnHitDamage(FVector HitLocation, AActor* Attacker);

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Skill1|Heal")
	void OnHealItemStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Skill1|Heal")
	void OnHealItemDrink();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Skill1|Heal")
	void OnHealItemComplete();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Skill1B|Parry")
	void OnParryAttempt();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Skill1B|Parry")
	void OnParrySuccess();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Skill1B|Parry")
	void OnParryFail();

	UFUNCTION(BlueprintCallable, Category = "Combat|Unlock")
	void SetUnlockHeavyAttack(bool bUnlocked);

	UFUNCTION(BlueprintCallable, Category = "Combat|Unlock")
	void SetUnlockParry(bool bUnlocked);

	UFUNCTION(BlueprintPure, Category = "Combat|Unlock")
	bool CanUseHeavyAttack() const { return bCanUseHeavyAttack; }

	UFUNCTION(BlueprintPure, Category = "Combat|Unlock")
	bool CanUseParry() const { return bCanUseParry; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	EGKCombatState GetCombatState() const { return CombatState; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetCurrentComboIndex() const { return CurrentComboIndex; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetHealItemRemaining() const { return HealItemRemaining; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsLockOnActive() const { return bLockOnActive; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	AGKEnemyCharacter* GetLockOnTarget() const { return LockOnTarget.Get(); }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "Audio|Wwise")
	TObjectPtr<UAkComponent> CharacterAkComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Config")
	TObjectPtr<UGKCombatConfig> CombatConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Config")
	TObjectPtr<UGKPlayerStatsConfig> PlayerStatsConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Capsule")
	float CapsuleRadius = 42.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Capsule")
	float CapsuleHalfHeight = 96.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float RotationRate = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float MinAnalogWalkSpeed = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float BrakingDecelerationWalking = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float BrakingDecelerationFalling = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraArmLength = 400.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EGKCombatState CombatState = EGKCombatState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 CurrentComboIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool ComboInputBuffered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float CurrentHP = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float CurrentStamina = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 HealItemRemaining = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Unlock")
	bool bCanUseHeavyAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Unlock")
	bool bCanUseParry = false;

private:
	static constexpr int32 JumpAttackAudioComboIndex = 3;
	static constexpr int32 HeavyAttackAudioComboIndex = 4;

	// --- Enhanced Input ---
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> EvadeAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ParryAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> HealAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LockOnAction;

	TWeakObjectPtr<AGKEnemyCharacter> LockOnTarget;
	bool bLockOnActive = false;

	FVector2D MoveInput = FVector2D::ZeroVector;
	bool bSprintPressed = false;

	bool bHealDrinkApplied = false;
	bool bHealCompleted = false;
	bool bAttackHitApplied = false;
	bool bJumpAttackHitApplied = false;
	bool bHeavyAttackHitApplied = false;
	bool bParrySucceeded = false;
	bool bIsInvulnerable = false;

	float StaminaRegenBlockedUntil = 0.f;
	float ActiveMotionElapsed = 0.f;

	FTimerHandle MotionTimerHandle;
	FTimerHandle HitWindowStartTimerHandle;
	FTimerHandle HitWindowEndTimerHandle;
	FTimerHandle HealDrinkTimerHandle;
	FTimerHandle EvadePhaseTimerHandle;
	FTimerHandle HitStunTimerHandle;
	FTimerHandle ParryActiveTimerHandle;
	FTimerHandle ParryRecoveryTimerHandle;

	TObjectPtr<UDataTable> RuntimeComboTable;

	const UGKCombatConfig* GetCombatConfig() const;
	const UGKPlayerStatsConfig* GetPlayerStatsConfig() const;
	const FGKComboAttackRow* GetComboRow(int32 ComboIndex) const;
	UDataTable* GetComboTable();

	void EnsureRuntimeConfigs();
	void ApplyCharacterTuning();
	void SetupWwiseDistanceProbe();

	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void ApplyMovementFromCachedInput();
	void HandleSprintStarted(const FInputActionValue& Value);
	void HandleSprintTriggered(const FInputActionValue& Value);
	void HandleSprintCompleted(const FInputActionValue& Value);
	void HandleJumpStarted(const FInputActionValue& Value);
	void HandleEvadeStarted(const FInputActionValue& Value);
	void HandleAttackStarted(const FInputActionValue& Value);
	void HandleHeavyAttackStarted(const FInputActionValue& Value);
	void HandleParryStarted(const FInputActionValue& Value);
	void HandleHealStarted(const FInputActionValue& Value);
	void HandleLockOnStarted(const FInputActionValue& Value);

	void UpdateLocomotionStateFromInput();
	void UpdateSprintStaminaDrain(float DeltaSeconds);
	void UpdateStaminaRegen(float DeltaSeconds);
	void UpdateLockOn(float DeltaSeconds);

	bool IsFullyBlockedCombatState() const;
	bool IsLocomotionBlockedState() const;
	bool PrepareWeaponAction(float StaminaCost);
	AGKEnemyCharacter* ExecuteMeleeLineTrace(FName StatTag) const;

	bool CanAcceptCombatInput() const;
	bool CanTransitionToEvade() const;
	bool CanTransitionToSprint() const;
	bool CanTransitionToJump() const;
	bool CanTransitionToAttack() const;
	bool CanTransitionToJumpAttack() const;
	bool CanTransitionToHeavyAttack() const;
	bool CanTransitionToParry() const;
	bool CanTransitionToHeal() const;
	bool IsGroundLocomotionState() const;
	bool IsAirborneCombatState() const;
	bool IsParryMotionState() const;
	bool HasEnoughStamina(float Cost) const;

	void SetCombatState(EGKCombatState NewState);
	void ResetComboState();
	void ApplyRunSpeed();
	void ApplySprintSpeed();

	void TryStartAttack();
	void TryStartJump();
	void TryStartJumpAttack();
	void TryStartHeavyAttack();
	void TryStartParry();
	void TryStartEvade();
	void TryStartHeal();
	void EnterSprintState();
	void ExitSprintState();

	void BeginComboAttack(int32 ComboIndex);
	void FinishAttackMotion();
	void OnAttackHitWindowStart();
	void OnAttackHitWindowEnd();
	void ProcessAttackHit();

	void BeginJumpAttack();
	void OnJumpAttackHitWindowStart();
	void OnJumpAttackHitWindowEnd();
	void ProcessJumpAttackHit();
	void FinishJumpAttack();

	void BeginHeavyAttack();
	void OnHeavyAttackHitWindowStart();
	void OnHeavyAttackHitWindowEnd();
	void ProcessHeavyAttackHit();
	void FinishHeavyAttack();

	void BeginParry();
	/** Tick-based polling while Parry_Active — checks enemy attack hit windows each frame. */
	void TickParryWindow();
	void CompleteParrySuccess(AGKEnemyCharacter* Enemy);
	void EndParryActivePhase();
	void EnterParryRecoveryPhase();
	void FinishParryMotion();

	void BeginEvadeMotion();
	void EnterEvadeRecoveryPhase();
	void FinishEvadeMotion();

	void BeginHealMotion();
	void ApplyHealDrinkEffect();
	void FinishHealMotion();

	void EnterHitStun(AActor* Attacker, const FVector& HitLocation);
	void FinishHitStun();
	void EnterDeath();

	void FaceLockOnTargetIfNeeded();
	FVector GetEvadeDirection() const;
	void ClearMotionTimers();

	void BroadcastWeaponSwing(int32 ComboIndex);
	void BroadcastEvadeStart();
	void BroadcastEvadeEnd();
	void BroadcastHealItemStart();
	void BroadcastHealItemDrink();
	void BroadcastHealItemComplete();
	void BroadcastHitDamage(const FVector& HitLocation, AActor* Attacker);
	void BroadcastParryAttempt();
	void BroadcastParrySuccess();
	void BroadcastParryFail();

	void ToggleLockOn();
	void AcquireLockOnTarget();
	void ClearLockOn();
	bool IsEnemyValidForLockOn(const AGKEnemyCharacter* Enemy) const;
};
