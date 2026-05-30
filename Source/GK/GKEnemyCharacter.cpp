// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AGKEnemyCharacter::AGKEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 90.f);

	if (UStaticMeshComponent* PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh")))
	{
		PlaceholderMesh->SetupAttachment(GetRootComponent());
		PlaceholderMesh->SetRelativeScale3D(FVector(1.8f, 0.6f, 0.6f));
		PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (CubeMesh.Succeeded())
		{
			PlaceholderMesh->SetStaticMesh(CubeMesh.Object);
		}
	}
}

void AGKEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	LastHitReaction = EGKHitReaction::None;
}

void AGKEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (LastHitReaction == EGKHitReaction::Down && GetWorld()->GetTimeSeconds() >= DownStateExpiresAt)
	{
		LastHitReaction = EGKHitReaction::None;
	}
}

void AGKEnemyCharacter::ApplyComboDamage(float Damage, int32 ComboIndex, AActor* InstigatorActor)
{
	if (!IsAlive())
	{
		return;
	}

	CurrentHP = FMath::Max(0.f, CurrentHP - Damage);

	if (CurrentHP <= 0.f)
	{
		LastHitReaction = EGKHitReaction::Death;
		UE_LOG(LogTemp, Log, TEXT("[Enemy] Death — ComboIndex=%d Instigator=%s"), ComboIndex,
			InstigatorActor ? *InstigatorActor->GetName() : TEXT("None"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
				FString::Printf(TEXT("Enemy Death (Combo %d)"), ComboIndex + 1));
		}
		return;
	}

	LastHitReaction = EGKHitReaction::Sway;
	UE_LOG(LogTemp, Log, TEXT("[Enemy] Sway — Damage=%.1f HP=%.1f ComboIndex=%d"), Damage, CurrentHP, ComboIndex);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow,
			FString::Printf(TEXT("Enemy Hit: %.0f dmg (HP %.0f)"), Damage, CurrentHP));
	}
}

void AGKEnemyCharacter::ApplyHeavyAttackDamage(float Damage, float DownDuration, AActor* InstigatorActor)
{
	if (!IsAlive())
	{
		return;
	}

	CurrentHP = FMath::Max(0.f, CurrentHP - Damage);

	if (CurrentHP <= 0.f)
	{
		LastHitReaction = EGKHitReaction::Death;
		UE_LOG(LogTemp, Log, TEXT("[Enemy] Death — HeavyAttack Instigator=%s"),
			InstigatorActor ? *InstigatorActor->GetName() : TEXT("None"));
		return;
	}

	EnterDownState(DownDuration, InstigatorActor);
	UE_LOG(LogTemp, Log, TEXT("[Enemy] Down — HeavyAttack Damage=%.1f HP=%.1f"), Damage, CurrentHP);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange,
			FString::Printf(TEXT("Enemy Down (Heavy %.0f dmg, HP %.0f)"), Damage, CurrentHP));
	}
}

void AGKEnemyCharacter::ApplyParrySuccess(float RipostWindowDuration, AActor* InstigatorActor)
{
	if (!IsAlive())
	{
		return;
	}

	bAttackHitWindowActive = false;
	EnterDownState(RipostWindowDuration, InstigatorActor);
	UE_LOG(LogTemp, Log, TEXT("[Enemy] Down — ParrySuccess RipostWindow=%.2fs"), RipostWindowDuration);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("Enemy Down (Parry Success)"));
	}
}

void AGKEnemyCharacter::SetAttackHitWindowActive(bool bActive)
{
	bAttackHitWindowActive = bActive;
}

bool AGKEnemyCharacter::IsInDownState() const
{
	return LastHitReaction == EGKHitReaction::Down && GetWorld()->GetTimeSeconds() < DownStateExpiresAt;
}

void AGKEnemyCharacter::EnterDownState(float Duration, AActor* InstigatorActor)
{
	LastHitReaction = EGKHitReaction::Down;
	DownStateExpiresAt = GetWorld()->GetTimeSeconds() + Duration;
}
