// Copyright Ashen Ossuary. All Rights Reserved.
// GKWeaponMeshComponent.h
// [에이전트 E → A] 무기 StaticMesh 소켓 어태치 전담 컴포넌트.
// AGKCharacter의 hand_r_socket / hand_l_socket에 동적 어태치.
// 실제 피격 처리(AGKCharacter::ExecuteMeleeLineTrace)와 무관 — 시각만 담당.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GKWeaponMeshComponent.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EGKWeaponHand : uint8
{
    RightHand   UMETA(DisplayName = "Right Hand"),
    LeftHand    UMETA(DisplayName = "Left Hand"),
};

UCLASS(ClassGroup = (GK), meta = (BlueprintSpawnableComponent),
    DisplayName = "GK Weapon Mesh Component")
class GK_API UGKWeaponMeshComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGKWeaponMeshComponent();

    /**
     * 무기 StaticMesh를 지정된 손 소켓에 어태치.
     * BeginPlay에서 자동 호출되며, 런타임 교체에도 재호출 가능.
     */
    UFUNCTION(BlueprintCallable, Category = "GK|Weapon")
    void AttachWeaponToSocket(EGKWeaponHand Hand = EGKWeaponHand::RightHand);

    /** 무기 메시 가시성 전환 (회피·히트스톤 연출용 훅). */
    UFUNCTION(BlueprintCallable, Category = "GK|Weapon")
    void SetWeaponVisible(bool bVisible, EGKWeaponHand Hand = EGKWeaponHand::RightHand);

    /** 런타임 무기 교체 (퀵슬롯 등). */
    UFUNCTION(BlueprintCallable, Category = "GK|Weapon")
    void SwapWeaponMesh(UStaticMesh* NewMesh, EGKWeaponHand Hand = EGKWeaponHand::RightHand);

protected:
    virtual void BeginPlay() override;

    // ── 에디터 설정 ───────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GK|Weapon|RightHand")
    TSoftObjectPtr<UStaticMesh> RightHandMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GK|Weapon|LeftHand")
    TSoftObjectPtr<UStaticMesh> LeftHandMesh;

    /** hand_r_socket / hand_l_socket 이름. 캐릭터 스켈레톤에 맞게 에디터에서 조정. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GK|Weapon|Sockets")
    FName RightHandSocketName = TEXT("hand_r_socket");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GK|Weapon|Sockets")
    FName LeftHandSocketName  = TEXT("hand_l_socket");

    /** 소켓 기준 상대 오프셋 (메시 피벗 보정). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GK|Weapon|Sockets")
    FTransform RightHandRelativeTransform = FTransform::Identity;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GK|Weapon|Sockets")
    FTransform LeftHandRelativeTransform  = FTransform::Identity;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GK|Weapon|Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> WeaponMaterialOverrides;

private:
    TObjectPtr<UStaticMeshComponent> RightWeaponSMC;
    TObjectPtr<UStaticMeshComponent> LeftWeaponSMC;

    UStaticMeshComponent* GetOrCreateWeaponSMC(EGKWeaponHand Hand);
    USkeletalMeshComponent* GetOwnerMainMesh() const;
    void ApplyMaterials(UStaticMeshComponent* SMC) const;
};
