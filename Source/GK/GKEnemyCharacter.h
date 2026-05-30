// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GKEnemyCharacter.generated.h"

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

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bAttackHitWindowActive = false;

	FTimerHandle DownRecoveryTimerHandle;

	void EnterDownState(float Duration, AActor* InstigatorActor);

	UFUNCTION()
	void OnDownRecoveryExpired();

	virtual void BeginPlay() override;
};
