// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GKCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
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
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- Combat State (로직은 Skill 1 기획 명세 수령 후 구현) ---
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EGKCombatState CombatState = EGKCombatState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 ComboIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float MaxStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float Stamina = 100.0f;

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
};
