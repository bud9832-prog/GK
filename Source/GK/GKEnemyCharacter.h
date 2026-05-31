// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GKAnimMontageTypes.h"
#include "GKEnemyCharacter.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EGKHitReaction : uint8
{
	None  UMETA(DisplayName = "None"),
	Sway  UMETA(DisplayName = "Sway"),
	Down  UMETA(DisplayName = "Down"),
	Death UMETA(DisplayName = "Death"),
};

UCLASS()
class GK_API AGKEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGKEnemyCharacter();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float MaxHP = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float CurrentHP = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AttackPower = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	FName LockOnTargetBoneName = TEXT("spine_03");

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EGKHitReaction LastHitReaction = EGKHitReaction::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Down")
	float DownDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Table")
	TObjectPtr<UDataTable> AnimMontageTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Table")
	EGKEnemyType EnemyType = EGKEnemyType::FirstEnemy;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Down")
	bool bIsDown = false;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyComboDamage(float Damage, int32 ComboIndex, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyHeavyAttackDamage(float Damage, float DownStateDuration, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyParrySuccess(float RipostWindowDuration, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetAttackHitWindowActive(bool bActive);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttackHitWindowActive() const { return bAttackHitWindowActive; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInDownState() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAlive() const { return CurrentHP > 0.f && LastHitReaction != EGKHitReaction::Death; }

	UFUNCTION(BlueprintCallable, Category = "Anim|Table")
	UAnimMontage* GetMontageForAction(EGKAnimAction Action, int32 Variant = 0) const;

	const FGKAnimMontageRow* GetRowForAction(EGKAnimAction Action, int32 Variant = 0) const;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bAttackHitWindowActive = false;

	FTimerHandle DownRecoveryTimerHandle;
	TObjectPtr<UDataTable> RuntimeAnimMontageTable;

	void EnterDownState(float Duration, AActor* InstigatorActor);
	UDataTable* GetAnimMontageTable();

	UFUNCTION()
	void OnDownRecoveryExpired();

	virtual void BeginPlay() override;
};
