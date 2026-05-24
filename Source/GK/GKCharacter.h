// Copyright Ashen Ossuary. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GKCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class GK_API AGKCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGKCharacter();

	// --- Audio Hooks (BlueprintImplementableEvent — Wwise mapping by KiHoon) ---

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Movement")
	void OnFootstep(EPhysicalSurface SurfaceType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnWeaponSwing(int32 ComboIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnEvadeStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnEvadeEnd();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
	void OnHitDamage(FVector HitLocation, AActor* Attacker);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
};
