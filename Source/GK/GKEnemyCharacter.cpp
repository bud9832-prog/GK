// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AGKEnemyCharacter::AGKEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

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
