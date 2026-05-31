// Copyright Ashen Ossuary. All Rights Reserved.
// GKWeaponMeshComponent.cpp

#include "GKWeaponMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

UGKWeaponMeshComponent::UGKWeaponMeshComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoActivate = true;
}

void UGKWeaponMeshComponent::BeginPlay()
{
    Super::BeginPlay();
    AttachWeaponToSocket(EGKWeaponHand::RightHand);

    // 왼손 메시가 설정된 경우에만 어태치
    if (!LeftHandMesh.IsNull())
    {
        AttachWeaponToSocket(EGKWeaponHand::LeftHand);
    }
}

// ─────────────────────────────────────────────────────────────
// 내부 유틸
// ─────────────────────────────────────────────────────────────

USkeletalMeshComponent* UGKWeaponMeshComponent::GetOwnerMainMesh() const
{
    const ACharacter* Character = Cast<ACharacter>(GetOwner());
    return Character ? Character->GetMesh() : nullptr;
}

UStaticMeshComponent* UGKWeaponMeshComponent::GetOrCreateWeaponSMC(EGKWeaponHand Hand)
{
    TObjectPtr<UStaticMeshComponent>& TargetRef = (Hand == EGKWeaponHand::RightHand)
        ? RightWeaponSMC : LeftWeaponSMC;

    if (TargetRef && IsValid(TargetRef))
    {
        return TargetRef.Get();
    }

    AActor* Owner  = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    const FName CompName = (Hand == EGKWeaponHand::RightHand)
        ? TEXT("WeaponSMC_R") : TEXT("WeaponSMC_L");

    UStaticMeshComponent* NewSMC = NewObject<UStaticMeshComponent>(Owner, CompName);
    if (!NewSMC)
    {
        return nullptr;
    }

    NewSMC->RegisterComponent();
    NewSMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NewSMC->SetCollisionResponseToAllChannels(ECR_Ignore);
    TargetRef = NewSMC;

    return NewSMC;
}

void UGKWeaponMeshComponent::ApplyMaterials(UStaticMeshComponent* SMC) const
{
    if (!SMC)
    {
        return;
    }
    for (int32 i = 0; i < WeaponMaterialOverrides.Num(); ++i)
    {
        if (!WeaponMaterialOverrides[i].IsNull())
        {
            UMaterialInterface* MI = WeaponMaterialOverrides[i].LoadSynchronous();
            SMC->SetMaterial(i, MI);
        }
    }
}

// ─────────────────────────────────────────────────────────────
// 공개: 소켓 어태치
// ─────────────────────────────────────────────────────────────

void UGKWeaponMeshComponent::AttachWeaponToSocket(EGKWeaponHand Hand)
{
    USkeletalMeshComponent* BaseMesh = GetOwnerMainMesh();
    if (!BaseMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GKWeapon] BaseMesh 없음 — AttachWeapon 중단"));
        return;
    }

    UStaticMeshComponent* WeaponSMC = GetOrCreateWeaponSMC(Hand);
    if (!WeaponSMC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GKWeapon] WeaponSMC 생성 실패 (Hand=%d)"), (int32)Hand);
        return;
    }

    const FName SocketName             = (Hand == EGKWeaponHand::RightHand) ? RightHandSocketName : LeftHandSocketName;
    const FTransform RelativeTransform = (Hand == EGKWeaponHand::RightHand) ? RightHandRelativeTransform : LeftHandRelativeTransform;
    const TSoftObjectPtr<UStaticMesh>& SoftMesh = (Hand == EGKWeaponHand::RightHand) ? RightHandMesh : LeftHandMesh;

    // 소켓 존재 여부 검증
    if (!BaseMesh->DoesSocketExist(SocketName))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GKWeapon] 소켓 없음: '%s' — 스켈레톤에 해당 소켓을 추가하세요"),
            *SocketName.ToString());
        return;
    }

    WeaponSMC->AttachToComponent(BaseMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    WeaponSMC->SetRelativeTransform(RelativeTransform);

    if (!SoftMesh.IsNull())
    {
        WeaponSMC->SetStaticMesh(SoftMesh.LoadSynchronous());
    }

    ApplyMaterials(WeaponSMC);

    UE_LOG(LogTemp, Log, TEXT("[GKWeapon] 소켓 어태치 완료: %s → %s"),
        *GetNameSafe(WeaponSMC->GetStaticMesh()), *SocketName.ToString());
}

void UGKWeaponMeshComponent::SetWeaponVisible(bool bVisible, EGKWeaponHand Hand)
{
    UStaticMeshComponent* SMC = (Hand == EGKWeaponHand::RightHand)
        ? RightWeaponSMC.Get() : LeftWeaponSMC.Get();
    if (SMC)
    {
        SMC->SetVisibility(bVisible);
    }
}

void UGKWeaponMeshComponent::SwapWeaponMesh(UStaticMesh* NewMesh, EGKWeaponHand Hand)
{
    UStaticMeshComponent* SMC = GetOrCreateWeaponSMC(Hand);
    if (!SMC)
    {
        return;
    }

    SMC->SetStaticMesh(NewMesh);
    SMC->SetVisibility(NewMesh != nullptr);
    ApplyMaterials(SMC);

    UE_LOG(LogTemp, Log, TEXT("[GKWeapon] 메시 교체: %s (Hand=%d)"),
        *GetNameSafe(NewMesh), (int32)Hand);
}
