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

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyComboDamage(float Damage, int32 ComboIndex, AActor* InstigatorActor);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAlive() const { return CurrentHP > 0.f && LastHitReaction != EGKHitReaction::Death; }

protected:
	virtual void BeginPlay() override;
};
