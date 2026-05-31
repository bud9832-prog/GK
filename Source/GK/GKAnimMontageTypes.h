// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GKAnimMontageTypes.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EGKAnimOwner : uint8
{
	PC    UMETA(DisplayName = "PC"),
	Enemy UMETA(DisplayName = "Enemy"),
};

UENUM(BlueprintType)
enum class EGKEnemyType : uint8
{
	None        UMETA(DisplayName = "None"),
	FirstEnemy  UMETA(DisplayName = "First Enemy"),
};

UENUM(BlueprintType)
enum class EGKAnimAction : uint8
{
	None            UMETA(DisplayName = "None"),
	Idle            UMETA(DisplayName = "Idle"),
	Run             UMETA(DisplayName = "Run"),
	Walk            UMETA(DisplayName = "Walk"),
	HitStun         UMETA(DisplayName = "Hit Stun"),
	Death           UMETA(DisplayName = "Death"),
	Sprint          UMETA(DisplayName = "Sprint"),
	Jump            UMETA(DisplayName = "Jump"),
	JumpAttack      UMETA(DisplayName = "Jump Attack"),
	Attack_Combo    UMETA(DisplayName = "Attack Combo"),
	Evade           UMETA(DisplayName = "Evade"),
	Heal            UMETA(DisplayName = "Heal"),
	HeavyAttack     UMETA(DisplayName = "Heavy Attack"),
	Parry_Active    UMETA(DisplayName = "Parry Active"),
	Parry_Recovery  UMETA(DisplayName = "Parry Recovery"),
	Enemy_Attack    UMETA(DisplayName = "Enemy Attack"),
	Enemy_Sway      UMETA(DisplayName = "Enemy Sway"),
	Enemy_Down      UMETA(DisplayName = "Enemy Down"),
};

USTRUCT(BlueprintType)
struct FGKAnimMontageRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Owner")
	EGKAnimOwner OwnerType = EGKAnimOwner::PC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Owner",
		meta = (EditCondition = "OwnerType == EGKAnimOwner::Enemy"))
	EGKEnemyType EnemyType = EGKEnemyType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	EGKAnimAction ActionTag = EGKAnimAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	int32 ActionVariant = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	float BlendInTime = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	float BlendOutTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meta")
	FString Notes;
};

struct GK_API FGKAnimMontageTableHelpers
{
	static bool AnimActionRequiresVariant(EGKAnimAction Action);
	static FName GetPCActionKeyName(EGKAnimAction Action);
	static FName GetEnemyTypeKeyName(EGKEnemyType EnemyType);
	static FName GetEnemyActionKeyName(EGKAnimAction Action);
	static FName BuildPCAnimRowName(EGKAnimAction Action, int32 Variant = 0);
	static FName BuildEnemyAnimRowName(EGKEnemyType EnemyType, EGKAnimAction Action, int32 Variant = 0);
	static const FGKAnimMontageRow* FindPCRow(const UDataTable* Table, EGKAnimAction Action, int32 Variant = 0);
	static const FGKAnimMontageRow* FindEnemyRow(
		const UDataTable* Table, EGKEnemyType EnemyType, EGKAnimAction Action, int32 Variant = 0);
	static UAnimMontage* ResolveMontage(const FGKAnimMontageRow* Row);
	static UDataTable* CreateDefaultAnimMontageTable(UObject* Outer);
};
