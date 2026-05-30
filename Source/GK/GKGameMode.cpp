// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKGameMode.h"
#include "GKCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AGKGameMode::AGKGameMode()
{
	bStartPlayersAsSpectators = false;

	static ConstructorHelpers::FClassFinder<APawn> GKCharacterBP(
		TEXT("/Game/ThirdPerson/Blueprints/BP_GKCharacter.BP_GKCharacter_C"));
	if (GKCharacterBP.Class)
	{
		DefaultPawnClass = GKCharacterBP.Class;
	}
	else
	{
		DefaultPawnClass = AGKCharacter::StaticClass();
	}
}

void AGKGameMode::RestartPlayer(AController* NewPlayer)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(NewPlayer))
	{
		PlayerController->ChangeState(NAME_Playing);
	}

	const UWorld* World = GetWorld();
	const float TimeSeconds = World ? World->GetTimeSeconds() : 0.f;
	AActor* StartSpot = NewPlayer ? FindPlayerStart(NewPlayer) : nullptr;

	UE_LOG(LogTemp, Log,
		TEXT("[GK|Possession] RestartPlayer BEGIN Controller=%s DefaultPawnClass=%s StartSpot=%s Time=%.3f"),
		*GetNameSafe(NewPlayer),
		DefaultPawnClass ? *DefaultPawnClass->GetName() : TEXT("None"),
		*GetNameSafe(StartSpot),
		TimeSeconds);

	Super::RestartPlayer(NewPlayer);

	if (NewPlayer && !NewPlayer->GetPawn() && DefaultPawnClass)
	{
		UWorld* MutableWorld = GetWorld();
		if (!MutableWorld)
		{
			return;
		}

		const FVector SpawnLocation = StartSpot ? StartSpot->GetActorLocation() : FVector(0.f, 0.f, 120.f);
		const FRotator SpawnRotation = StartSpot ? StartSpot->GetActorRotation() : FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (APawn* FallbackPawn = MutableWorld->SpawnActor<APawn>(DefaultPawnClass, SpawnLocation, SpawnRotation, SpawnParams))
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(NewPlayer))
			{
				if (APlayerState* PlayerState = PlayerController->PlayerState)
				{
					PlayerState->SetIsSpectator(false);
				}
				PlayerController->ChangeState(NAME_Playing);
				PlayerController->SetViewTarget(FallbackPawn);
				PlayerController->Possess(FallbackPawn);
			}
			else
			{
				NewPlayer->Possess(FallbackPawn);
			}

			UE_LOG(LogTemp, Log,
				TEXT("[GK|Possession] AfterFallbackPossess Controller=%s Pawn=%s ControlledPawn=%s"),
				*GetNameSafe(NewPlayer),
				*GetNameSafe(FallbackPawn),
				*GetNameSafe(NewPlayer->GetPawn()));
			UE_LOG(LogTemp, Warning,
				TEXT("[GK|Possession] FallbackSpawn Controller=%s Pawn=%s Class=%s Time=%.3f"),
				*GetNameSafe(NewPlayer),
				*GetNameSafe(FallbackPawn),
				*GetNameSafe(FallbackPawn->GetClass()),
				TimeSeconds);
		}
		else
		{
			UE_LOG(LogTemp, Error,
				TEXT("[GK|Possession] FallbackSpawn FAILED Controller=%s DefaultPawnClass=%s Time=%.3f"),
				*GetNameSafe(NewPlayer),
				*GetNameSafe(DefaultPawnClass),
				TimeSeconds);
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("[GK|Possession] RestartPlayer END Controller=%s Pawn=%s Time=%.3f"),
		*GetNameSafe(NewPlayer),
		*GetNameSafe(NewPlayer ? NewPlayer->GetPawn() : nullptr),
		TimeSeconds);
}

void AGKGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (NewPlayer && NewPlayer->IsInState(NAME_Spectating))
	{
		NewPlayer->ChangeState(NAME_Playing);
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (NewPlayer && !NewPlayer->GetPawn())
	{
		NewPlayer->ChangeState(NAME_Playing);
		RestartPlayer(NewPlayer);
	}

	const UWorld* World = GetWorld();
	const float TimeSeconds = World ? World->GetTimeSeconds() : 0.f;
	const APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;

	if (Pawn)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[GK|Possession] Complete Controller=%s Pawn=%s Class=%s DefaultPawnClass=%s Time=%.3f"),
			*GetNameSafe(NewPlayer),
			*GetNameSafe(Pawn),
			*GetNameSafe(Pawn->GetClass()),
			DefaultPawnClass ? *DefaultPawnClass->GetName() : TEXT("None"),
			TimeSeconds);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[GK|Possession] MissingPawn Controller=%s DefaultPawnClass=%s Time=%.3f"),
			*GetNameSafe(NewPlayer),
			DefaultPawnClass ? *DefaultPawnClass->GetName() : TEXT("None"),
			TimeSeconds);
	}
}

APawn* AGKGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	const UWorld* World = GetWorld();
	const float TimeSeconds = World ? World->GetTimeSeconds() : 0.f;

	APawn* SpawnedPawn = Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	if (SpawnedPawn)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[GK|Possession] SpawnDefaultPawn Controller=%s Pawn=%s Class=%s StartSpot=%s Time=%.3f"),
			*GetNameSafe(NewPlayer),
			*GetNameSafe(SpawnedPawn),
			*GetNameSafe(SpawnedPawn->GetClass()),
			*GetNameSafe(StartSpot),
			TimeSeconds);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[GK|Possession] SpawnDefaultPawn FAILED Controller=%s DefaultPawnClass=%s StartSpot=%s Time=%.3f"),
			*GetNameSafe(NewPlayer),
			DefaultPawnClass ? *DefaultPawnClass->GetName() : TEXT("None"),
			*GetNameSafe(StartSpot),
			TimeSeconds);
	}

	return SpawnedPawn;
}
