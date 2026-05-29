// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "GKCombatConfig.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FGKComboAttackRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	int32 ComboIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float Damage = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float StaminaCost = 22.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float MotionDuration = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float HitWindowStart = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float HitWindowEnd = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float ComboInputWindowStart = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float ComboInputWindowEnd = 0.70f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> Montage = nullptr;
};

UCLASS(BlueprintType)
class GK_API UGKCombatConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Stamina (global)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float StaminaRegenPerSec = 35.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float StaminaRegenDelay = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float Stamina_Evade = 22.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float Stamina_SprintPerSec = 12.f;

	// Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RunSpeed = 450.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintHoldThreshold = 0.18f;

	// Combo — Data Table reference
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UDataTable> ComboAttackTable = nullptr;

	// Evade
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade")
	TObjectPtr<UAnimMontage> EvadeMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade")
	float Evade_TotalDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Evade")
	float Evade_IFrameDuration = 0.45f;

	// Lock-on
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	float LockOnMaxDistance = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	float LockOnFOVDegrees = 60.f;

	/** §7-3 default rows — used when ComboAttackTable is not assigned in editor. */
	static UDataTable* CreateDefaultComboAttackTable(UObject* Outer);

	const FGKComboAttackRow* GetComboRowByIndex(int32 ComboIndex) const;
};
