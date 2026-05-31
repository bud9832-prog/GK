// Copyright Ashen Ossuary. All Rights Reserved.
// AAshenKnightCharacter.cpp

#include "AAshenKnightCharacter.h"
#include "GKModularArmorComponent.h"
#include "GKWeaponMeshComponent.h"
#include "Components/CapsuleComponent.h"

AAshenKnightCharacter::AAshenKnightCharacter()
{
    // ── 갑옷 컴포넌트 ──────────────────────────────────────
    ModularArmor = CreateDefaultSubobject<UGKModularArmorComponent>(TEXT("ModularArmor"));

    // ── 무기 컴포넌트 ──────────────────────────────────────
    WeaponMesh = CreateDefaultSubobject<UGKWeaponMeshComponent>(TEXT("WeaponMesh"));

    // ── 캡슐 재설정 (갑옷 착용 캐릭터 폭 보정) ────────────
    // AGKCharacter::CapsuleRadius는 EditDefaultsOnly이므로 BP에서도 재조정 가능.
    // 생성자에서는 GetCapsuleComponent()로 직접 접근.
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->InitCapsuleSize(AshenKnightCapsuleRadius, 96.f);
    }
}

void AAshenKnightCharacter::BeginPlay()
{
    // AGKCharacter::BeginPlay() → EnsureRuntimeConfigs, PreloadCriticalMontages,
    // Enhanced Input 등록, Wwise DistanceProbe 설정 순서로 실행.
    Super::BeginPlay();

    // 비주얼 컴포넌트는 자체 BeginPlay에서 처리됨 (SpawnArmorComponents, AttachWeaponToSocket).
    // 여기서 추가 호출 불필요.

    UE_LOG(LogTemp, Log, TEXT("[AshenKnight] BeginPlay 완료 — 아머 슬롯 수: %s"),
        *GetNameSafe(ModularArmor));
}
