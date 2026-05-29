// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GKPlayerStatsConfig.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class GK_API UGKPlayerStatsConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HP")
	float MaxHP = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealItem")
	TObjectPtr<UAnimMontage> HealItemMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealItem")
	float HealItem_MotionDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealItem")
	float HealItem_DrinkTime = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealItem")
	float HealItem_HealAmount = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealItem")
	int32 HealItem_StartCount = 3;
};
