// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GKCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAkComponent;
class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)
enum class EGKCombatState : uint8
{
	Idle      UMETA(DisplayName = "Idle"),
	Attacking UMETA(DisplayName = "Attacking"),
	Evading   UMETA(DisplayName = "Evading"),
	HitStun   UMETA(DisplayName = "HitStun")
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
	void OnWeaponSwing(int32 InComboIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnEvadeStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnEvadeEnd();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnHitDamage(FVector HitLocation, AActor* Attacker);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "Audio|Wwise")
	TObjectPtr<UAkComponent> CharacterAkComponent;

	// --- Character Tuning (EditDefaultsOnly — §3-4, 에디터·BP 기본값에서 조정) ---
	UPROPERTY(EditDefaultsOnly, Category = "Character|Capsule")
	float CapsuleRadius = 42.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Capsule")
	float CapsuleHalfHeight = 96.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float RotationRate = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float JumpZVelocity = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float AirControl = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float MaxWalkSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float MinAnalogWalkSpeed = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float BrakingDecelerationWalking = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	float BrakingDecelerationFalling = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraArmLength = 400.f;

	// --- Combat State (로직은 Skill 1 기획 명세 수령 후 구현) ---
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EGKCombatState CombatState = EGKCombatState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 ComboIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float MaxStamina = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float Stamina = 0.f;

private:
	// --- Enhanced Input ---
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void ApplyCharacterTuning();
	void SetupWwiseDistanceProbe();
};
