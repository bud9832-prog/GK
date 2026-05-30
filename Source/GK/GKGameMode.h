// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GKGameMode.generated.h"

UCLASS()
class GK_API AGKGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGKGameMode();

protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

	// --- Stage Hooks (Skill 2 구현 대기 — 선언만, 로직 없음) ---
	UPROPERTY(BlueprintReadOnly, Category = "Stage")
	int32 CurrentStage = 1;

	UFUNCTION(BlueprintImplementableEvent, Category = "Stage")
	void OnStageCleared(int32 StageNum);

	UFUNCTION(BlueprintImplementableEvent, Category = "Stage")
	void OnSkillUnlocked(FName SkillName);
};
