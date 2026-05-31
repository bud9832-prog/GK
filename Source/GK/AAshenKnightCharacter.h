// Copyright Ashen Ossuary. All Rights Reserved.
// AAshenKnightCharacter.h
//
// [설계 원칙]
//  - 모든 전투·입력·스테미나·오디오 훅 로직은 AGKCharacter에 존재. 여기서 중복 금지.
//  - 이 클래스의 책임: 비주얼 컴포넌트 조립 + 오디오 훅 구현부 연결 only.
//  - C++ 전담 에이전트(A/B)가 전투 로직 수정 시 이 파일에 영향 없어야 함.
//  - BP 자식 클래스 BP_AshenKnightCharacter에서 DataAsset 및 메시 레퍼런스 할당.
//
// [컴포넌트 구성]
//  AGKCharacter
//    └─ UGKModularArmorComponent   (갑옷 8파트 + 망토)
//    └─ UGKWeaponMeshComponent     (양손 무기 스태틱 메시)

#pragma once

#include "CoreMinimal.h"
#include "GKCharacter.h"
#include "AAshenKnightCharacter.generated.h"

class UGKModularArmorComponent;
class UGKWeaponMeshComponent;

UCLASS(DisplayName = "Ashen Knight Character")
class GK_API AAshenKnightCharacter : public AGKCharacter
{
    GENERATED_BODY()

public:
    AAshenKnightCharacter();

    // ── 오디오 훅 구현 (BlueprintImplementableEvent → C++ 디폴트 처리 경유점) ──
    // 사운드 최종 결정은 KiHoon. 여기서는 Wwise 연동 인터페이스만 뚫어둠.
    // BP_AshenKnightCharacter에서 Super 호출 후 Wwise Event 발송 구현.

    virtual void OnFootstep_Implementation(EPhysicalSurface SurfaceType) {};
    virtual void OnWeaponSwing_Implementation(int32 ComboIndex) {};
    virtual void OnEvadeStart_Implementation() {};
    virtual void OnEvadeEnd_Implementation() {};
    virtual void OnHitDamage_Implementation(FVector HitLocation, AActor* Attacker) {};

    // ── 컴포넌트 접근자 ───────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "GK|Visual")
    UGKModularArmorComponent* GetArmorComponent() const { return ModularArmor; }

    UFUNCTION(BlueprintPure, Category = "GK|Visual")
    UGKWeaponMeshComponent* GetWeaponComponent() const { return WeaponMesh; }

protected:
    virtual void BeginPlay() override;

    // ── 비주얼 컴포넌트 ──────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GK|Visual|Armor",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UGKModularArmorComponent> ModularArmor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GK|Visual|Weapon",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UGKWeaponMeshComponent> WeaponMesh;

    // ── 이미지 기반 캡슐 튜닝 (오버라이드) ──────────────────
    // 이미지 분석: 전신 갑옷 착용 → 어깨 너비 고려 CapsuleRadius 44→48
    // CapsuleHalfHeight는 AGKCharacter 기본값(96) 유지
    static constexpr float AshenKnightCapsuleRadius = 48.f;
};
